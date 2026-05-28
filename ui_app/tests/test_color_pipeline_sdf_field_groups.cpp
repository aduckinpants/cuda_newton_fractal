#include "../src/color_pipeline_sdf_field_groups.h"

#include <initializer_list>
#include <iostream>
#include <string>

namespace {

bool Check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << "\n";
        return false;
    }
    return true;
}

KernelParams MakeSdfStack(std::initializer_list<ColorSignal> signals) {
    KernelParams params{};
    params.color_source_stack_count = static_cast<int>(signals.size());
    int index = 0;
    for (ColorSignal signal : signals) {
        params.color_source_stack[index].signal = signal;
        params.color_source_stack[index].params.blend_weight = 1.0f;
        ++index;
    }
    return params;
}

} // namespace

int main() {
    {
        KernelParams params = MakeSdfStack({ColorSignal::sdf_normal_angle, ColorSignal::sdf_curvature});
        SdfFieldGroupPlan plan{};
        std::string error;
        if (!Check(BuildColorPipelineSdfFieldGroupPlan(params, 2, 0, plan, &error), "inherited SDF rows should plan successfully")) return 1;
        if (!Check(error.empty(), "successful inherited plan should not leave an error")) return 1;
        if (!Check(plan.group_count == 1, "two inherited SDF rows should share one field group")) return 1;
        if (!Check(!plan.uses_distinct_fields, "all-inherit rows should preserve the single-field fast path")) return 1;
        if (!Check(plan.groups[0].requested_downsample == 2 && plan.groups[0].effective_downsample == 2, "inherited group should use shared requested/effective downsample")) return 1;
        if (!Check(plan.groups[0].row_count == 2, "inherited group should contain both rows")) return 1;
        if (!Check(plan.row_groups[0].group_index == 0 && plan.row_groups[1].group_index == 0, "both rows should map to the inherited group")) return 1;
        if (!Check(plan.row_groups[0].inherited && plan.row_groups[1].inherited, "default row field policy should be inherit_shared")) return 1;
    }

    {
        KernelParams params = MakeSdfStack({ColorSignal::sdf_normal_angle, ColorSignal::sdf_curvature});
        params.color_source_stack[0].params.sdf_field_downsample = 1;
        params.color_source_stack[1].params.sdf_field_downsample = 4;
        SdfFieldGroupPlan plan{};
        std::string error;
        if (!Check(BuildColorPipelineSdfFieldGroupPlan(params, 2, 0, plan, &error), "explicit row field downsample rows should plan successfully")) return 1;
        if (!Check(plan.group_count == 2, "1x and 4x explicit rows should produce two field groups")) return 1;
        if (!Check(plan.uses_distinct_fields, "distinct row-local field resolutions should disable the single-field fast path")) return 1;
        if (!Check(plan.row_groups[0].requested_downsample == 1 && plan.row_groups[0].effective_downsample == 1, "first explicit row should keep 1x requested/effective downsample")) return 1;
        if (!Check(plan.row_groups[1].requested_downsample == 4 && plan.row_groups[1].effective_downsample == 4, "second explicit row should keep 4x requested/effective downsample")) return 1;
        if (!Check(!plan.row_groups[0].inherited && !plan.row_groups[1].inherited, "explicit row field downsample rows should not be reported as inherited")) return 1;
    }

    {
        KernelParams params = MakeSdfStack({ColorSignal::sdf_signed_distance, ColorSignal::sdf_boundary_band});
        params.color_source_stack[0].params.sdf_field_downsample = 2;
        SdfFieldGroupPlan plan{};
        std::string error;
        if (!Check(BuildColorPipelineSdfFieldGroupPlan(params, 1, 4, plan, &error), "adaptive preview should plan row-local field groups")) return 1;
        if (!Check(plan.group_count == 1, "adaptive budget should merge rows that share one effective downsample")) return 1;
        if (!Check(plan.groups[0].effective_downsample == 4, "adaptive budget should raise effective field downsample for each group")) return 1;
        if (!Check(plan.row_groups[0].requested_downsample == 2 && plan.row_groups[0].effective_downsample == 4, "explicit row should report requested and adaptive effective downsample")) return 1;
        if (!Check(plan.row_groups[1].requested_downsample == 1 && plan.row_groups[1].effective_downsample == 4, "inherited row should report shared requested and adaptive effective downsample")) return 1;
    }

    {
        KernelParams params = MakeSdfStack({ColorSignal::sdf_signed_distance, ColorSignal::smooth_escape});
        SdfFieldGroupPlan plan{};
        std::string error;
        if (!Check(!BuildColorPipelineSdfFieldGroupPlan(params, 2, 0, plan, &error), "mixed SDF/non-SDF source stacks must fail closed")) return 1;
        if (!Check(error.find("cannot mix") != std::string::npos, "mixed-stack failure should explain the incompatible source stack")) return 1;
    }

    {
        KernelParams params = MakeSdfStack({
            ColorSignal::sdf_signed_distance,
            ColorSignal::sdf_inside_outside,
            ColorSignal::sdf_boundary_band,
            ColorSignal::sdf_normal_angle,
            ColorSignal::sdf_curvature,
        });
        const int explicitDownsamples[] = {1, 2, 4, 8, 16};
        for (int index = 0; index < params.color_source_stack_count; ++index) {
            params.color_source_stack[index].params.sdf_field_downsample = explicitDownsamples[index];
        }
        SdfFieldGroupPlan plan{};
        std::string error;
        if (!Check(!BuildColorPipelineSdfFieldGroupPlan(params, 1, 0, plan, &error), "more than four distinct live SDF field groups should fail closed")) return 1;
        if (!Check(error.find("exceeds") != std::string::npos, "group-cap failure should report the cap")) return 1;
    }

    return 0;
}
