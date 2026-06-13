#pragma once

#include "fractal_types.h"
#include "explaino_root_field.h"
#include "sdf_pack_field_producer.h"

#include <cstdint>
#include <string>

struct ExplainoRootSdfResolvedScene {
    int root_count{0};
    ExplainoRootFieldLayoutKind layout_kind{ExplainoRootFieldLayoutKind::none};
    Float2 base_roots[kExplainoRootFieldMaxRoots]{};
    Float2 effective_roots[kExplainoRootFieldMaxRoots]{};
    int bridge_count{0};
    std::uint64_t base_root_hash{0};
    std::uint64_t effective_root_hash{0};
};

struct ExplainoRootSdfFieldReport {
    int root_count{0};
    int bridge_count{0};
    const char* root_layout_kind{"none"};
    int requested_generated_root_count{0};
    const char* h_source{"none"};
    std::uint64_t base_root_hash{0};
    std::uint64_t effective_root_hash{0};
};

bool ResolveExplainoRootSdfScene(
    const ViewState& view,
    const KernelParams& params,
    ExplainoRootSdfResolvedScene* outScene,
    std::string* outError = nullptr);

bool ComputeExplainoRootSdfFieldForViewport(
    const ViewState& view,
    const KernelParams& params,
    const SdfPackFieldRegion& region,
    int renderWidth,
    int renderHeight,
    int effectiveDownsample,
    SdfPackFieldBackend backend,
    SdfFieldResult& outField,
    SdfPackFieldReport* outPackReport = nullptr,
    ExplainoRootSdfFieldReport* outRootReport = nullptr,
    std::string* outError = nullptr);
