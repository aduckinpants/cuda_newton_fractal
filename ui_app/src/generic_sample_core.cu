// generic_sample_core.cu — Standalone CUDA compilation unit for generic function sampling.
// Contains: generic_sample_kernel<<<>>>(), SampleGenericFunction().
// Linkable independently from fractal_renderer.cu (no DX11/ImGui/fractal_types deps).

#include "generic_function_types.h"
#include "generic_function_eval.cuh"

#include <cuda_runtime.h>
#include <algorithm>

// --- Device kernel ---
// Handles iterate and compose at root level with convergence/divergence tracking.
// Falls back to flat evaluation for other root ops.

__global__ void generic_sample_kernel(
    const double2* coords,
    int numPoints,
    GenericSampleResult* results,
    GenericFunctionDesc func,
    double epsilon,
    double escape_radius)
{
    int idx = (int)(blockIdx.x * blockDim.x + threadIdx.x);
    if (idx >= numPoints) return;

    double2 c = coords[idx];
    GFComplex z = {c.x, c.y};
    const GFNode& root = func.nodes[func.root_node];

    if (root.op == GFNodeOp::gf_iterate) {
        // Iterative evaluation with fixed-point convergence / escape divergence.
        int maxIter = (root.param_index >= 0 && root.param_index < func.param_count)
            ? (int)func.params[root.param_index] : func.max_iterate;

        int subtree = root.child_left;
        double eps2 = epsilon * epsilon;
        double esc2 = escape_radius * escape_radius;

        int iter = 0;
        bool conv = false, div = false;

        for (; iter < maxIter; ++iter) {
            GFComplex z_new = gf_device_eval_flat(
                func.nodes, func.node_count, func.params, subtree, z);

            // Fixed-point convergence: |z_new - z| < epsilon
            double dx = z_new.x - z.x;
            double dy = z_new.y - z.y;
            double delta2 = dx * dx + dy * dy;

            z = z_new;

            if (delta2 < eps2) { conv = true; iter++; break; }
            double a2 = gf_abs2(z);
            if (a2 > esc2)     { div  = true; iter++; break; }
        }

        // Derivative of the one-step function at final z.
        double mag = sqrt(gf_abs2(z));
        double h = 1e-8 * fmax(mag, 1.0);
        GFComplex fz  = gf_device_eval_flat(
            func.nodes, func.node_count, func.params, subtree, z);
        GFComplex fzh = gf_device_eval_flat(
            func.nodes, func.node_count, func.params, subtree, {z.x + h, z.y});

        GenericSampleResult r;
        r.value_x = z.x;
        r.value_y = z.y;
        r.abs2 = gf_abs2(z);
        r.derivative_x = (fzh.x - fz.x) / h;
        r.derivative_y = (fzh.y - fz.y) / h;
        r.iterations = iter;
        r.converged = conv;
        r.diverged = div;
        results[idx] = r;

    } else if (root.op == GFNodeOp::gf_compose) {
        // compose(f, g): evaluate g(z), then f(g(z)).
        GFComplex gz = gf_device_eval_flat(
            func.nodes, func.node_count, func.params, root.child_right, z);
        GFComplex fgz = gf_device_eval_flat(
            func.nodes, func.node_count, func.params, root.child_left, gz);

        // Derivative of the composed function via forward-difference.
        double mag = sqrt(gf_abs2(z));
        double h = 1e-8 * fmax(mag, 1.0);
        GFComplex gz2 = gf_device_eval_flat(
            func.nodes, func.node_count, func.params, root.child_right, {z.x + h, z.y});
        GFComplex fgz2 = gf_device_eval_flat(
            func.nodes, func.node_count, func.params, root.child_left, gz2);

        double a2 = gf_abs2(fgz);
        GenericSampleResult r;
        r.value_x = fgz.x;
        r.value_y = fgz.y;
        r.abs2 = a2;
        r.derivative_x = (fgz2.x - fgz.x) / h;
        r.derivative_y = (fgz2.y - fgz.y) / h;
        r.iterations = 0;
        r.converged = a2 < epsilon * epsilon;
        r.diverged = a2 > escape_radius * escape_radius;
        results[idx] = r;

    } else {
        // Direct flat evaluation (no iterate/compose at root).
        results[idx] = gf_device_sample(func, z, epsilon, escape_radius);
    }
}

// --- Host API ---

bool SampleGenericFunction(
    const GFPoint* coords,
    int numPoints,
    const GenericFunctionDesc& func,
    double epsilon,
    double escape_radius,
    GenericSampleResult* outResults,
    const char** outError)
{
    if (outError) *outError = nullptr;

    // Zero points is a valid no-op.
    if (numPoints <= 0) return true;

    if (!coords) {
        if (outError) *outError = "coords is null";
        return false;
    }
    if (!outResults) {
        if (outError) *outError = "outResults is null";
        return false;
    }

    // Validate descriptor (fail fast, no implicit fallback).
    GFValidationResult val = ValidateGenericFunctionDesc(func);
    if (!val.valid) {
        if (outError) *outError = val.error;
        return false;
    }

    // Select CUDA device.
    int deviceCount = 0;
    cudaError_t deviceCountErr = cudaGetDeviceCount(&deviceCount);
    if (deviceCountErr != cudaSuccess) {
        if (outError) *outError = "CUDA backend unavailable: device enumeration failed";
        return false;
    }
    if (deviceCount <= 0) {
        if (outError) *outError = "CUDA backend unavailable: no CUDA devices detected";
        return false;
    }
    if (cudaSetDevice(0) != cudaSuccess) {
        if (outError) *outError = "CUDA backend unavailable: failed to select device 0";
        return false;
    }

    // Ensure enough stack for deep expression trees.
    if (cudaDeviceSetLimit(cudaLimitStackSize, 16384) != cudaSuccess) {
        if (outError) *outError = "CUDA backend unavailable: failed to configure device stack size";
        return false;
    }

    // Allocate device memory.
    static_assert(sizeof(GFPoint) == sizeof(double2), "GFPoint/double2 layout mismatch");
    double2* d_coords = nullptr;
    GenericSampleResult* d_results = nullptr;
    size_t coordBytes = (size_t)numPoints * sizeof(double2);
    size_t resultBytes = (size_t)numPoints * sizeof(GenericSampleResult);

    if (cudaMalloc(&d_coords, coordBytes) != cudaSuccess) {
        if (outError) *outError = "cudaMalloc failed for coords";
        return false;
    }
    if (cudaMalloc(&d_results, resultBytes) != cudaSuccess) {
        cudaFree(d_coords);
        if (outError) *outError = "cudaMalloc failed for results";
        return false;
    }

    // Upload coordinates.
    if (cudaMemcpy(d_coords, coords, coordBytes, cudaMemcpyHostToDevice) != cudaSuccess) {
        cudaFree(d_coords);
        cudaFree(d_results);
        if (outError) *outError = "cudaMemcpy failed for coords";
        return false;
    }

    // Launch kernel.
    int threadsPerBlock = 256;
    int blocks = (numPoints + threadsPerBlock - 1) / threadsPerBlock;

    generic_sample_kernel<<<blocks, threadsPerBlock>>>(
        d_coords, numPoints, d_results, func, epsilon, escape_radius);

    cudaError_t launchErr = cudaGetLastError();
    if (launchErr != cudaSuccess) {
        cudaFree(d_coords);
        cudaFree(d_results);
        if (outError) *outError = "CUDA generic sample kernel launch failed";
        return false;
    }

    cudaError_t syncErr = cudaDeviceSynchronize();
    if (syncErr != cudaSuccess) {
        cudaFree(d_coords);
        cudaFree(d_results);
        if (outError) *outError = "CUDA generic sample kernel execution failed";
        return false;
    }

    // Copy results back.
    if (cudaMemcpy(outResults, d_results, resultBytes, cudaMemcpyDeviceToHost) != cudaSuccess) {
        cudaFree(d_coords);
        cudaFree(d_results);
        if (outError) *outError = "cudaMemcpy failed for results";
        return false;
    }

    cudaFree(d_coords);
    cudaFree(d_results);
    return true;
}
