#pragma once

#include "fractal_types.h"

#include <cstdint>
#include <string>

constexpr int kExplainoRootFieldMaxRoots = 16;

enum class ExplainoRootFieldLayoutKind : std::uint8_t {
    none = 0,
    legacy_quartic_v1 = 1,
    custom = 2,
    family_local = 3,
    regular_ngon_v1 = 4,
};

enum class ExplainoRootFieldSourceKind : std::uint8_t {
    none = 0,
    generated = 1,
    custom = 2,
};

struct ExplainoRootFieldDescriptor {
    std::uint32_t version{1};
    int active_count{0};
    int max_count{kExplainoRootFieldMaxRoots};
    ExplainoRootFieldLayoutKind layout_kind{ExplainoRootFieldLayoutKind::none};
    ExplainoRootFieldSourceKind source_kind{ExplainoRootFieldSourceKind::none};
    Float2 base_roots[kExplainoRootFieldMaxRoots]{};
    Float2 effective_roots[kExplainoRootFieldMaxRoots]{};
    std::uint64_t base_root_hash{0};
    std::uint64_t effective_root_hash{0};
};

const char* ExplainoRootFieldLayoutKindId(ExplainoRootFieldLayoutKind kind);
const char* ExplainoRootFieldSourceKindId(ExplainoRootFieldSourceKind kind);
bool TryParseExplainoRootFieldLayoutKindId(const std::string& id, ExplainoRootFieldLayoutKind* outKind);
bool TryParseExplainoRootFieldSourceKindId(const std::string& id, ExplainoRootFieldSourceKind* outKind);

bool ResolveExplainoRootFieldDescriptor(
    const ViewState& view,
    const KernelParams& params,
    ExplainoRootFieldDescriptor* outDescriptor,
    std::string* outError = nullptr);
