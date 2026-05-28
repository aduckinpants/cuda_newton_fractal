#pragma once

#include "fractal_types.h"

#include <algorithm>
#include <array>
#include <string>

constexpr int kColorPipelineSdfFieldDownsampleInherit = 0;
constexpr int kColorPipelineSdfMaxFieldGroupCount = 4;

struct SdfFieldGroupRowBinding {
    int source_row_index{-1};
    int group_index{-1};
    int requested_downsample{1};
    int effective_downsample{1};
    bool inherited{true};
};

struct SdfFieldGroup {
    int group_index{-1};
    int requested_downsample{1};
    int effective_downsample{1};
    bool has_inherited_row{false};
    bool has_explicit_row{false};
    int row_count{0};
    std::array<int, kColorPipelineMaxSourceStackCount> source_row_indices{};
};

struct SdfFieldGroupPlan {
    bool valid{false};
    bool uses_distinct_fields{false};
    int source_row_count{0};
    int group_count{0};
    std::array<SdfFieldGroup, kColorPipelineSdfMaxFieldGroupCount> groups{};
    std::array<SdfFieldGroupRowBinding, kColorPipelineMaxSourceStackCount> row_groups{};
};

inline int NormalizeColorPipelineSdfRowFieldDownsamplePolicy(int value) {
    if (value <= 0) return kColorPipelineSdfFieldDownsampleInherit;
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    if (value <= 4) return 4;
    if (value <= 8) return 8;
    return 16;
}

inline int NormalizeColorPipelineSdfResolvedDownsample(int value) {
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    if (value <= 4) return 4;
    if (value <= 8) return 8;
    return 16;
}

inline bool IsColorPipelineSdfFieldGroupSignal(ColorSignal signal) {
    return signal == ColorSignal::sdf_signed_distance ||
        signal == ColorSignal::sdf_inside_outside ||
        signal == ColorSignal::sdf_boundary_band ||
        signal == ColorSignal::sdf_normal_angle ||
        signal == ColorSignal::sdf_curvature ||
        signal == ColorSignal::lens_field_v2_distance;
}

inline bool TryResolveColorPipelineSdfRowDownsample(
    const ColorPipelineSourceStackEntry& entry,
    int shared_requested_downsample,
    int adaptive_effective_downsample,
    SdfFieldGroupRowBinding& outBinding,
    std::string* outError) {
    const int policy = NormalizeColorPipelineSdfRowFieldDownsamplePolicy(entry.params.sdf_field_downsample);
    const bool inherited = policy == kColorPipelineSdfFieldDownsampleInherit;
    const int requested = inherited
        ? NormalizeColorPipelineSdfResolvedDownsample(shared_requested_downsample)
        : policy;
    const int adaptive = adaptive_effective_downsample > 0
        ? NormalizeColorPipelineSdfResolvedDownsample(adaptive_effective_downsample)
        : requested;
    const int effective = (std::max)(requested, adaptive);
    if (requested <= 0 || effective <= 0) {
        if (outError) *outError = "SDF row field downsample resolved to an invalid value";
        return false;
    }
    outBinding.requested_downsample = requested;
    outBinding.effective_downsample = effective;
    outBinding.inherited = inherited;
    return true;
}

inline bool AppendSdfFieldGroupPlanRow(
    SdfFieldGroupPlan& plan,
    const ColorPipelineSourceStackEntry& entry,
    int sourceRowIndex,
    int sharedRequestedDownsample,
    int adaptiveEffectiveDownsample,
    std::string* outError) {
    if (plan.source_row_count < 0 || plan.source_row_count >= kColorPipelineMaxSourceStackCount) {
        if (outError) *outError = "SDF source row count exceeds supported maximum";
        return false;
    }

    SdfFieldGroupRowBinding binding{};
    binding.source_row_index = sourceRowIndex;
    if (!TryResolveColorPipelineSdfRowDownsample(
            entry,
            sharedRequestedDownsample,
            adaptiveEffectiveDownsample,
            binding,
            outError)) {
        return false;
    }

    int groupIndex = -1;
    for (int index = 0; index < plan.group_count; ++index) {
        if (plan.groups[static_cast<std::size_t>(index)].effective_downsample == binding.effective_downsample) {
            groupIndex = index;
            break;
        }
    }
    if (groupIndex < 0) {
        if (plan.group_count >= kColorPipelineSdfMaxFieldGroupCount) {
            if (outError) *outError = "SDF field group count exceeds the supported live cap";
            return false;
        }
        groupIndex = plan.group_count++;
        SdfFieldGroup& group = plan.groups[static_cast<std::size_t>(groupIndex)];
        group = {};
        group.group_index = groupIndex;
        group.requested_downsample = binding.requested_downsample;
        group.effective_downsample = binding.effective_downsample;
    }

    binding.group_index = groupIndex;
    SdfFieldGroup& group = plan.groups[static_cast<std::size_t>(groupIndex)];
    if (group.row_count >= kColorPipelineMaxSourceStackCount) {
        if (outError) *outError = "SDF field group row count exceeds supported maximum";
        return false;
    }
    group.source_row_indices[static_cast<std::size_t>(group.row_count++)] = sourceRowIndex;
    group.requested_downsample = (std::min)(group.requested_downsample, binding.requested_downsample);
    group.has_inherited_row = group.has_inherited_row || binding.inherited;
    group.has_explicit_row = group.has_explicit_row || !binding.inherited;
    plan.row_groups[static_cast<std::size_t>(plan.source_row_count++)] = binding;
    return true;
}

inline bool BuildColorPipelineSdfFieldGroupPlan(
    const KernelParams& params,
    int shared_requested_downsample,
    int adaptive_effective_downsample,
    SdfFieldGroupPlan& outPlan,
    std::string* outError = nullptr) {
    outPlan = {};
    const int sharedRequested = NormalizeColorPipelineSdfResolvedDownsample(shared_requested_downsample);
    const int adaptiveEffective = adaptive_effective_downsample > 0
        ? NormalizeColorPipelineSdfResolvedDownsample(adaptive_effective_downsample)
        : 0;
    const int sourceStackCount = (std::max)(0, (std::min)(params.color_source_stack_count, kColorPipelineMaxSourceStackCount));
    if (sourceStackCount <= 0) {
        if (!IsColorPipelineSdfFieldGroupSignal(params.color_pipeline.signal)) {
            if (outError) *outError = "SDF field groups require an SDF Color Pipeline source";
            return false;
        }
        ColorPipelineSourceStackEntry flatEntry{};
        flatEntry.signal = params.color_pipeline.signal;
        if (!AppendSdfFieldGroupPlanRow(outPlan, flatEntry, -1, sharedRequested, adaptiveEffective, outError)) {
            outPlan = {};
            return false;
        }
        outPlan.valid = true;
        outPlan.uses_distinct_fields = false;
        return true;
    }

    bool hasSdf = false;
    bool hasNonSdf = false;
    for (int index = 0; index < sourceStackCount; ++index) {
        const bool isSdf = IsColorPipelineSdfFieldGroupSignal(params.color_source_stack[index].signal);
        hasSdf = hasSdf || isSdf;
        hasNonSdf = hasNonSdf || !isSdf;
    }
    if (hasSdf && hasNonSdf) {
        if (outError) *outError = "SDF Color Pipeline source stacks cannot mix SDF and non-SDF Source rows";
        return false;
    }
    if (!hasSdf) {
        if (outError) *outError = "SDF field groups require at least one SDF Source row";
        return false;
    }

    for (int index = 0; index < sourceStackCount; ++index) {
        if (!AppendSdfFieldGroupPlanRow(
                outPlan,
                params.color_source_stack[index],
                index,
                sharedRequested,
                adaptiveEffective,
                outError)) {
            outPlan = {};
            return false;
        }
    }
    outPlan.valid = true;
    outPlan.uses_distinct_fields = outPlan.group_count > 1;
    return true;
}
