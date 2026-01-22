// Minimal CUDA Newton fractal renderer (PPM output)
// Previously referenced artifacts-only location; now local under project\cuda_newton_fractal

#include <cuda_runtime.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

static void cudaCheck(cudaError_t err, const char* what) {
    if (err == cudaSuccess) return;
    std::fprintf(stderr, "CUDA error (%s): %s\n", what, cudaGetErrorString(err));
    std::exit(2);
}

struct float2_ {
    float x;
    float y;
};

__device__ __forceinline__ float2_ c_add(float2_ a, float2_ b) { return {a.x + b.x, a.y + b.y}; }
__device__ __forceinline__ float2_ c_sub(float2_ a, float2_ b) { return {a.x - b.x, a.y - b.y}; }
__device__ __forceinline__ float2_ c_mul(float2_ a, float2_ b) {
    return {a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x};
}
__device__ __forceinline__ float2_ c_scale(float2_ a, float s) { return {a.x * s, a.y * s}; }
__device__ __forceinline__ float c_abs2(float2_ a) { return a.x * a.x + a.y * a.y; }

__device__ __forceinline__ float2_ c_div(float2_ a, float2_ b) {
    // a / b
    float denom = b.x * b.x + b.y * b.y;
    // denom should not be 0 for typical Newton steps; handle anyway.
    if (denom == 0.0f) return {0.0f, 0.0f};
    return {(a.x * b.x + a.y * b.y) / denom, (a.y * b.x - a.x * b.y) / denom};
}

// f(z) = z^3 - 1
__device__ __forceinline__ float2_ f(float2_ z) {
    float2_ z2 = c_mul(z, z);
    float2_ z3 = c_mul(z2, z);
    return {z3.x - 1.0f, z3.y};
}

// f'(z) = 3 z^2
__device__ __forceinline__ float2_ df(float2_ z) {
    float2_ z2 = c_mul(z, z);
    return {3.0f * z2.x, 3.0f * z2.y};
}

__device__ __forceinline__ float dist2(float2_ a, float2_ b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

__global__ void newtonKernel(
    uchar3* out,
    int width,
    int height,
    float xMin,
    float xMax,
    float yMin,
    float yMax,
    int maxIter,
    float eps
) {
    int px = blockIdx.x * blockDim.x + threadIdx.x;
    int py = blockIdx.y * blockDim.y + threadIdx.y;
    if (px >= width || py >= height) return;

    float u = (width <= 1) ? 0.0f : (float)px / (float)(width - 1);
    float v = (height <= 1) ? 0.0f : (float)py / (float)(height - 1);

    float2_ z;
    z.x = xMin + u * (xMax - xMin);
    z.y = yMin + v * (yMax - yMin);

    int iter = 0;
    for (; iter < maxIter; ++iter) {
        float2_ fv = f(z);
        if (c_abs2(fv) <= eps * eps) break;
        float2_ dfv = df(z);
        float2_ step = c_div(fv, dfv);
        z = c_sub(z, step);
        // mild bailout to avoid NaNs exploding
        if (!isfinite(z.x) || !isfinite(z.y) || c_abs2(z) > 1e6f) break;
    }

    // Three roots of z^3 = 1:
    // r0 = 1 + 0i
    // r1 = -1/2 + i*sqrt(3)/2
    // r2 = -1/2 - i*sqrt(3)/2
    const float s3_2 = 0.8660254037844386f;
    float2_ r0{1.0f, 0.0f};
    float2_ r1{-0.5f, s3_2};
    float2_ r2{-0.5f, -s3_2};

    float d0 = dist2(z, r0);
    float d1 = dist2(z, r1);
    float d2 = dist2(z, r2);

    int root = 0;
    float dmin = d0;
    if (d1 < dmin) { dmin = d1; root = 1; }
    if (d2 < dmin) { dmin = d2; root = 2; }

    // Shade by iteration count: faster convergence -> brighter
    float t = (maxIter <= 0) ? 0.0f : 1.0f - (float)iter / (float)maxIter;
    t = fminf(1.0f, fmaxf(0.0f, t));
    float base = 0.25f + 0.75f * t;

    unsigned char r, g, b;
    if (iter >= maxIter) {
        // Non-converged: dark gray
        r = g = b = 20;
    } else {
        // root colors
        float cr = 0.0f, cg = 0.0f, cb = 0.0f;
        if (root == 0) { cr = 1.0f; cg = 0.25f; cb = 0.25f; }
        if (root == 1) { cr = 0.25f; cg = 1.0f; cb = 0.25f; }
        if (root == 2) { cr = 0.25f; cg = 0.25f; cb = 1.0f; }

        r = (unsigned char)fminf(255.0f, fmaxf(0.0f, 255.0f * (cr * base)));
        g = (unsigned char)fminf(255.0f, fmaxf(0.0f, 255.0f * (cg * base)));
        b = (unsigned char)fminf(255.0f, fmaxf(0.0f, 255.0f * (cb * base)));
    }

    out[py * width + px] = make_uchar3(r, g, b);
}

static int parseInt(const char* s, int defaultValue) {
    if (!s || !*s) return defaultValue;
    char* end = nullptr;
    long v = std::strtol(s, &end, 10);
    if (!end || *end != '\0') return defaultValue;
    if (v < 1) v = 1;
    if (v > 100000) v = 100000;
    return (int)v;
}

static float parseFloat(const char* s, float defaultValue) {
    if (!s || !*s) return defaultValue;
    char* end = nullptr;
    float v = std::strtof(s, &end);
    if (!end || *end != '\0') return defaultValue;
    return v;
}

static void writePPM(const std::string& path, int w, int h, const std::vector<unsigned char>& rgb) {
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
        std::perror("fopen");
        std::fprintf(stderr, "Failed to open output: %s\n", path.c_str());
        std::exit(3);
    }
    std::fprintf(f, "P6\n%d %d\n255\n", w, h);
    std::fwrite(rgb.data(), 1, rgb.size(), f);
    std::fclose(f);
}

int main(int argc, char** argv) {
    int width = 1024;
    int height = 1024;
    int maxIter = 50;
    float eps = 1e-4f;

    float xMin = -2.0f, xMax = 2.0f;
    float yMin = -2.0f, yMax = 2.0f;

    std::string outPath = "newton.ppm";

    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--width") && i + 1 < argc) { width = parseInt(argv[++i], width); continue; }
        if (!std::strcmp(argv[i], "--height") && i + 1 < argc) { height = parseInt(argv[++i], height); continue; }
        if (!std::strcmp(argv[i], "--max-iter") && i + 1 < argc) { maxIter = parseInt(argv[++i], maxIter); continue; }
        if (!std::strcmp(argv[i], "--eps") && i + 1 < argc) { eps = parseFloat(argv[++i], eps); continue; }
        if (!std::strcmp(argv[i], "--xmin") && i + 1 < argc) { xMin = parseFloat(argv[++i], xMin); continue; }
        if (!std::strcmp(argv[i], "--xmax") && i + 1 < argc) { xMax = parseFloat(argv[++i], xMax); continue; }
        if (!std::strcmp(argv[i], "--ymin") && i + 1 < argc) { yMin = parseFloat(argv[++i], yMin); continue; }
        if (!std::strcmp(argv[i], "--ymax") && i + 1 < argc) { yMax = parseFloat(argv[++i], yMax); continue; }
        if (!std::strcmp(argv[i], "--out") && i + 1 < argc) { outPath = argv[++i]; continue; }

        if (!std::strcmp(argv[i], "--help") || !std::strcmp(argv[i], "-h")) {
            std::printf(
                "Newton fractal (CUDA)\n\n"
                "Usage:\n"
                "  newton_fractal.exe [--width N] [--height N] [--max-iter N] [--eps E] [--xmin X] [--xmax X] [--ymin Y] [--ymax Y] [--out path.ppm]\n\n"
                "Defaults: 1024x1024, max-iter=50, eps=1e-4, view=[-2,2]x[-2,2], out=newton.ppm\n"
            );
            return 0;
        }

        std::fprintf(stderr, "Unknown arg: %s\n", argv[i]);
        return 1;
    }

    int device = 0;
    cudaDeviceProp prop{};
    cudaCheck(cudaGetDevice(&device), "cudaGetDevice");
    cudaCheck(cudaGetDeviceProperties(&prop, device), "cudaGetDeviceProperties");

    std::printf("Using CUDA device %d: %s\n", device, prop.name);

    size_t pixelCount = (size_t)width * (size_t)height;
    size_t outBytes = pixelCount * sizeof(uchar3);

    uchar3* dOut = nullptr;
    cudaCheck(cudaMalloc((void**)&dOut, outBytes), "cudaMalloc dOut");

    dim3 block(16, 16);
    dim3 grid((width + block.x - 1) / block.x, (height + block.y - 1) / block.y);

    newtonKernel<<<grid, block>>>(dOut, width, height, xMin, xMax, yMin, yMax, maxIter, eps);
    cudaCheck(cudaGetLastError(), "kernel launch");
    cudaCheck(cudaDeviceSynchronize(), "cudaDeviceSynchronize");

    std::vector<unsigned char> hostRgb;
    hostRgb.resize(pixelCount * 3);

    std::vector<uchar3> tmp;
    tmp.resize(pixelCount);

    cudaCheck(cudaMemcpy(tmp.data(), dOut, outBytes, cudaMemcpyDeviceToHost), "cudaMemcpy D2H");

    for (size_t i = 0; i < pixelCount; ++i) {
        hostRgb[i * 3 + 0] = tmp[i].x;
        hostRgb[i * 3 + 1] = tmp[i].y;
        hostRgb[i * 3 + 2] = tmp[i].z;
    }

    writePPM(outPath, width, height, hostRgb);

    cudaCheck(cudaFree(dOut), "cudaFree dOut");

    std::printf("Wrote %s (%dx%d)\n", outPath.c_str(), width, height);
    return 0;
}
