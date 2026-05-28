#include "../src/lens_sdf.h"
#include "../src/lens_sdf_cuda.h"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

bool IsFiniteField(const SdfFieldResult& field) {
    for (float value : field.signed_distance_px) {
        if (!std::isfinite(value)) {
            return false;
        }
    }
    return true;
}

bool CheckSemanticParity(const char* label, const std::vector<uint8_t>& mask, int width, int height, int downsample, float tolerance) {
    SdfFieldResult cpu;
    SdfFieldResult gpu;
    LensSdfBackendReport report;
    LensSdfFieldGenerationReport fieldReport;
    if (!ComputeLensSdfFieldForMask(mask.data(), width, height, downsample, cpu)) {
        std::cerr << label << ": CPU reference failed\n";
        return false;
    }
    if (!ComputeLensSdfFieldForMaskWithBackend(mask.data(), width, height, downsample, LensSdfBackend::cuda_jfa, gpu, &report, &fieldReport)) {
        std::cerr << label << ": CUDA backend failed\n";
        return false;
    }
    if (report.requested != LensSdfBackend::cuda_jfa || report.used != LensSdfBackend::cuda_jfa || report.fallback_used) {
        std::cerr << label << ": CUDA backend report is not authoritative\n";
        return false;
    }
    if (gpu.width != cpu.width || gpu.height != cpu.height ||
        std::fabs(gpu.pixel_scale - cpu.pixel_scale) > 0.0001f ||
        gpu.sign_convention != SdfSignConvention::negative_inside_positive_outside ||
        gpu.source_kind != SdfFieldSourceKind::mask_derived ||
        gpu.signed_distance_px.size() != cpu.signed_distance_px.size() ||
        !IsFiniteField(gpu)) {
        std::cerr << label << ": CUDA field metadata is wrong\n";
        return false;
    }
    if (fieldReport.input_width != width ||
        fieldReport.input_height != height ||
        fieldReport.downsample != NormalizeLensDownsamplePow2(downsample) ||
        fieldReport.field_width != gpu.width ||
        fieldReport.field_height != gpu.height ||
        fieldReport.mask_downsample_ms < 0.0f ||
        fieldReport.backend_ms < 0.0f) {
        std::cerr << label << ": CUDA field-generation report is wrong\n";
        return false;
    }

    for (size_t i = 0; i < cpu.signed_distance_px.size(); ++i) {
        const float c = cpu.signed_distance_px[i];
        const float g = gpu.signed_distance_px[i];
        if ((c < 0.0f && !(g < 0.0f)) || (c > 0.0f && !(g > 0.0f)) || (c == 0.0f && std::fabs(g) > 0.0001f)) {
            std::cerr << label << ": sign mismatch at " << i << " cpu=" << c << " gpu=" << g << "\n";
            return false;
        }
        if (std::fabs(c) < 1.0e8f && std::fabs(g) < 1.0e8f && std::fabs(c - g) > tolerance) {
            std::cerr << label << ": bounded-distance mismatch at " << i << " cpu=" << c << " gpu=" << g << "\n";
            return false;
        }
    }
    return true;
}

std::vector<uint8_t> SinglePixelMask(int width, int height, int x, int y) {
    std::vector<uint8_t> mask(static_cast<size_t>(width) * static_cast<size_t>(height), 0);
    mask[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = 255;
    return mask;
}

std::vector<uint8_t> StripeMask(int width, int height) {
    std::vector<uint8_t> mask(static_cast<size_t>(width) * static_cast<size_t>(height), 0);
    for (int y = 0; y < height; ++y) {
        for (int x = width / 3; x < (2 * width) / 3; ++x) {
            mask[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = 255;
        }
    }
    return mask;
}

std::vector<uint8_t> BoxMask(int width, int height) {
    std::vector<uint8_t> mask(static_cast<size_t>(width) * static_cast<size_t>(height), 0);
    for (int y = height / 4; y < (3 * height) / 4; ++y) {
        for (int x = width / 4; x < (3 * width) / 4; ++x) {
            mask[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)] = 255;
        }
    }
    return mask;
}

} // namespace

int main() {
    if (!CheckSemanticParity("single-pixel", SinglePixelMask(9, 9, 4, 4), 9, 9, 1, 1.5f)) {
        return 1;
    }
    if (!CheckSemanticParity("stripe", StripeMask(16, 10), 16, 10, 1, 1.5f)) {
        return 1;
    }
    if (!CheckSemanticParity("box", BoxMask(18, 14), 18, 14, 1, 1.5f)) {
        return 1;
    }
    if (!CheckSemanticParity("odd-downsample", BoxMask(15, 11), 15, 11, 2, 1.5f)) {
        return 1;
    }
    if (!CheckSemanticParity("all-outside", std::vector<uint8_t>(25, 0), 5, 5, 1, 1.5f)) {
        return 1;
    }
    if (!CheckSemanticParity("all-inside", std::vector<uint8_t>(25, 255), 5, 5, 1, 1.5f)) {
        return 1;
    }

    {
        SdfFieldResult field;
        LensSdfBackendReport report;
        if (ComputeLensSdfFieldForMaskWithBackend(nullptr, 4, 4, 1, LensSdfBackend::cuda_jfa, field, &report) ||
            field.width != 0 ||
            !field.signed_distance_px.empty()) {
            std::cerr << "CUDA backend should reject invalid input and clear output\n";
            return 1;
        }
    }

    {
        const std::vector<uint8_t> mask = BoxMask(12, 8);
        SdfFieldResult cpuDirect;
        SdfFieldResult cpuBackend;
        LensSdfBackendReport report;
        LensSdfFieldGenerationReport fieldReport;
        if (!ComputeLensSdfFieldForMask(mask.data(), 12, 8, 2, cpuDirect) ||
            !ComputeLensSdfFieldForMaskWithBackend(mask.data(), 12, 8, 2, LensSdfBackend::cpu_chamfer, cpuBackend, &report, &fieldReport)) {
            std::cerr << "CPU backend path should succeed\n";
            return 1;
        }
        if (report.used != LensSdfBackend::cpu_chamfer || report.fallback_used ||
            cpuDirect.width != cpuBackend.width ||
            cpuDirect.height != cpuBackend.height ||
            cpuDirect.signed_distance_px != cpuBackend.signed_distance_px) {
            std::cerr << "CPU backend should preserve existing ComputeLensSdfFieldForMask behavior\n";
            return 1;
        }
        if (fieldReport.downsample != 2 ||
            fieldReport.field_width != cpuBackend.width ||
            fieldReport.field_height != cpuBackend.height ||
            fieldReport.mask_downsample_ms < 0.0f ||
            fieldReport.backend_ms < 0.0f) {
            std::cerr << "CPU backend field-generation report should describe the measured stages\n";
            return 1;
        }
    }

    {
        const std::vector<uint8_t> mask = StripeMask(10, 10);
        SdfFieldResult field;
        LensSdfBackendReport report;
        if (!ComputeLensSdfFieldForMaskWithBackend(mask.data(), 10, 10, 1, LensSdfBackend::auto_backend, field, &report)) {
            std::cerr << "auto backend should produce a valid field\n";
            return 1;
        }
        if (report.used != LensSdfBackend::cuda_jfa || report.fallback_used) {
            std::cerr << "auto backend should prefer CUDA while this focused CUDA rail is available\n";
            return 1;
        }
    }

    std::cout << "test_lens_sdf_cuda: all passed\n";
    return 0;
}
