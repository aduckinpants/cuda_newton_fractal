#pragma once

#include "fractal_family_rules.h"

#include <cmath>
#include <string>

inline bool FailFractalRuntimeValidation(const char* message, std::string* outError) {
    if (outError) *outError = message;
    return false;
}

inline bool FailFractalRuntimeValidation(const char* message, const char** outError) {
    if (outError) *outError = message;
    return false;
}

inline PolyKind ProjectionAndFlowRequiredPolyKind(ProjectionAndFlowRootFamily rootFamily) {
    switch (rootFamily) {
    case ProjectionAndFlowRootFamily::cubic_unit_roots:
        return PolyKind::z3_minus_1;
    case ProjectionAndFlowRootFamily::quartic_unit_roots:
        return PolyKind::z4_minus_1;
    }
    return PolyKind::z3_minus_1;
}

inline bool ProjectionAndFlowPolyPresetMatchesRootFamily(const KernelParams& params) {
    if (params.projection_and_flow_root_family == ProjectionAndFlowRootFamily::cubic_unit_roots) {
        return params.poly_coeffs[0] == -1.0f &&
               params.poly_coeffs[1] == 0.0f &&
               params.poly_coeffs[2] == 0.0f &&
               params.poly_coeffs[3] == 1.0f &&
               params.poly_coeffs[4] == 0.0f;
    }
    return params.poly_coeffs[0] == -1.0f &&
           params.poly_coeffs[1] == 0.0f &&
           params.poly_coeffs[2] == 0.0f &&
           params.poly_coeffs[3] == 0.0f &&
           params.poly_coeffs[4] == 1.0f;
}

template <typename ErrorSink>
inline bool ValidateFractalRuntimeStateImpl(const ViewState& view,
    const KernelParams& params,
    ErrorSink outError) {
    if (params.max_iter <= 0) {
        return FailFractalRuntimeValidation("max_iter must be > 0", outError);
    }
    if (!std::isfinite(params.epsilon) || params.epsilon <= 0.0f) {
        return FailFractalRuntimeValidation("epsilon must be finite and > 0", outError);
    }
    if (!IsColoringModeAllowedForFractal(view.fractal_type, params.coloring_mode)) {
        return FailFractalRuntimeValidation("selected coloring_mode is not valid for fractal_type", outError);
    }
    if (view.fractal_type == FractalType::projection_and_flow) {
        if (params.poly_kind != ProjectionAndFlowRequiredPolyKind(params.projection_and_flow_root_family)) {
            return FailFractalRuntimeValidation("projection_and_flow root family and poly_kind must agree", outError);
        }
        if (!ProjectionAndFlowPolyPresetMatchesRootFamily(params)) {
            return FailFractalRuntimeValidation("projection_and_flow root family must own the shipped polynomial preset", outError);
        }
        if (!std::isfinite(params.projection_and_flow_target_radius) ||
            params.projection_and_flow_target_radius <= 0.0f ||
            params.projection_and_flow_target_radius > 4.0f) {
            return FailFractalRuntimeValidation("projection_and_flow_target_radius must be finite and in (0,4]", outError);
        }
        if (!std::isfinite(params.projection_and_flow_pressure_threshold) ||
            params.projection_and_flow_pressure_threshold < 0.0f ||
            params.projection_and_flow_pressure_threshold > 8.0f) {
            return FailFractalRuntimeValidation("projection_and_flow_pressure_threshold must be finite and in [0,8]", outError);
        }
    }
    if ((view.fractal_type == FractalType::lambda_map || view.fractal_type == FractalType::explaino_lambda) &&
        (!std::isfinite(params.lambda_real) || !std::isfinite(params.lambda_imag) ||
         std::fabs(params.lambda_real) > 4.0f || std::fabs(params.lambda_imag) > 4.0f)) {
        return FailFractalRuntimeValidation("lambda_real/lambda_imag must be finite and in [-4,4]", outError);
    }
    if (view.fractal_type == FractalType::nova || view.fractal_type == FractalType::explaino_nova) {
        if (!std::isfinite(params.nova_alpha) || params.nova_alpha <= 0.0f || params.nova_alpha > 5.0f) {
            return FailFractalRuntimeValidation("nova_alpha must be finite and in (0,5]", outError);
        }
    }
    if (FindPhoenixStepCarrierSelectorDescriptor(view.fractal_type) != nullptr) {
        if (!std::isfinite(params.phoenix_p_real) || !std::isfinite(params.phoenix_p_imag)) {
            return FailFractalRuntimeValidation("phoenix_p must be finite", outError);
        }
        if (std::fabs(params.phoenix_p_real) > 1.0f || std::fabs(params.phoenix_p_imag) > 1.0f) {
            return FailFractalRuntimeValidation("phoenix_p_real/imag must be in [-1,1]", outError);
        }
    }
    if (FindExplainoClusterRadiusSelectorDescriptor(view.fractal_type) != nullptr) {
        if (!std::isfinite(params.explaino_cluster_radius) ||
            params.explaino_cluster_radius < 0.0f ||
            params.explaino_cluster_radius > 2.0f) {
            return FailFractalRuntimeValidation("explaino_cluster_radius must be finite and in [0,2]", outError);
        }
    }
    if (view.fractal_type == FractalType::multibrot) {
        if (!std::isfinite(params.multibrot_power_float) ||
            params.multibrot_power_float < 2.0f ||
            params.multibrot_power_float > 12.0f) {
            return FailFractalRuntimeValidation("multibrot_power_float must be finite and in [2,12]", outError);
        }
    }
    if (view.fractal_type == FractalType::multicorn) {
        if (params.multibrot_power < 2 || params.multibrot_power > 12) {
            return FailFractalRuntimeValidation("multibrot_power must be in [2,12]", outError);
        }
    }
    if (IsExplainoFamily(view.fractal_type)) {
        if (!std::isfinite(params.explaino_seed) || !std::isfinite(params.explaino_seed_b)) {
            return FailFractalRuntimeValidation("explaino_seed and explaino_seed_b must be finite", outError);
        }
        if (!std::isfinite(params.explaino_mix) || params.explaino_mix < 0.0f || params.explaino_mix > 1.0f) {
            return FailFractalRuntimeValidation("explaino_mix must be finite and in [0,1]", outError);
        }
        if (!std::isfinite(params.explaino_warp_strength) ||
            params.explaino_warp_strength < 0.0f ||
            params.explaino_warp_strength > 1.0f) {
            return FailFractalRuntimeValidation("explaino_warp_strength must be finite and in [0,1]", outError);
        }
    }

    return true;
}

inline bool ValidateFractalRuntimeState(const ViewState& view,
    const KernelParams& params,
    std::string* outError) {
    return ValidateFractalRuntimeStateImpl(view, params, outError);
}

inline bool ValidateFractalRuntimeState(const ViewState& view,
    const KernelParams& params,
    const char** outError) {
    return ValidateFractalRuntimeStateImpl(view, params, outError);
}
