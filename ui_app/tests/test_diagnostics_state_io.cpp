#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "../src/color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "../src/diagnostics_state_io.h"
#include "../src/diagnostics_capture.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <vector>

static bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

static bool ControllerPoliciesMatch(const SidecarAutoDemoControllerPolicy& lhs,
  const SidecarAutoDemoControllerPolicy& rhs,
  double eps = 1.0e-9) {
  return lhs.enabled == rhs.enabled &&
       lhs.allow_runtime_mutation == rhs.allow_runtime_mutation &&
       lhs.run_paced_loop == rhs.run_paced_loop &&
       NearlyEqual(lhs.paced_loop_interval_seconds, rhs.paced_loop_interval_seconds, eps) &&
       NearlyEqual(lhs.stop_demonstrated_fraction, rhs.stop_demonstrated_fraction, eps) &&
       lhs.stop_uncertain_count == rhs.stop_uncertain_count;
}

static bool MutationRecordsMatch(const SidecarAutoDemoMutationRecord& lhs,
  const SidecarAutoDemoMutationRecord& rhs,
  double eps = 1.0e-9) {
  return lhs.label == rhs.label &&
       lhs.path == rhs.path &&
       lhs.type == rhs.type &&
       NearlyEqual(lhs.target_value, rhs.target_value, eps) &&
       NearlyEqual(lhs.utility, rhs.utility, eps);
}

static bool MutationHistoriesMatch(const SidecarAutoDemoMutationHistory& lhs,
  const SidecarAutoDemoMutationHistory& rhs,
  double eps = 1.0e-9) {
  if (lhs.size() != rhs.size()) return false;
  for (size_t index = 0; index < lhs.size(); ++index) {
    if (!MutationRecordsMatch(lhs[index], rhs[index], eps)) return false;
  }
  return true;
}

static bool DraftRowHasNumberParam(const ColorPipelineRowState& row, const char* path, double expected, double eps = 1.0e-9) {
  for (const ColorPipelineParamState& param : row.parameter_values) {
    if (param.path == path && NearlyEqual(param.number_value, expected, eps)) {
      return true;
    }
  }
  return false;
}

static bool SetDraftRowNumberParam(ColorPipelineRowState* row, const char* path, double value) {
  if (!row) return false;
  for (ColorPipelineParamState& param : row->parameter_values) {
    if (param.path == path) {
      param.number_value = value;
      return true;
    }
  }
  return false;
}

static bool SetDraftRowBoolParam(ColorPipelineRowState* row, const char* path, bool value) {
  if (!row) return false;
  for (ColorPipelineParamState& param : row->parameter_values) {
    if (param.path == path) {
      param.bool_value = value;
      return true;
    }
  }
  return false;
}

static std::size_t FindDraftLaneIndex(const ColorPipelineWindowState& state, const char* laneId) {
  for (std::size_t index = 0; index < state.lanes.size(); ++index) {
    if (state.lanes[index].lane_id == (laneId ? laneId : "")) {
      return index;
    }
  }
  return state.lanes.size();
}

static bool NormalizeDraftForLaneComparison(
  ColorPipelineWindowState* state,
  std::string* outError = nullptr) {
  if (!state) {
    if (outError) *outError = "Advanced color matrix comparison requires a draft state";
    return false;
  }

  const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
  std::vector<ColorPipelineLaneState> normalizedLanes;
  normalizedLanes.reserve(catalogs.size());
  for (const ColorPipelineLaneCatalog& catalog : catalogs) {
    const std::size_t existingIndex = FindDraftLaneIndex(*state, catalog.lane_id);
    if (existingIndex < state->lanes.size()) {
      normalizedLanes.push_back(state->lanes[existingIndex]);
      continue;
    }

    ColorPipelineLaneState lane;
    if (!BuildColorPipelineLaneWithSingleRow(catalog, catalog.default_function_id, 0, &lane, outError)) {
      return false;
    }
    normalizedLanes.push_back(std::move(lane));
  }

  state->lanes = std::move(normalizedLanes);
  state->initialized = true;
  std::uint64_t nextRowId = state->next_row_id;
  for (ColorPipelineLaneState& lane : state->lanes) {
    if (!EnsureColorPipelineLaneRowsInitialized(&lane, &nextRowId)) {
      if (outError) *outError = "Advanced color matrix comparison failed to normalize draft row ids";
      return false;
    }
  }
  state->next_row_id = nextRowId;
  return true;
}

static double ChooseDistinctDraftNumberValue(const FunctionParamDescriptor& param, double currentValue, int variantIndex = 0) {
  if (param.type == "int") {
    const int currentInt = static_cast<int>(std::lround(currentValue));
    const int minInt = param.has_min ? static_cast<int>(std::lround(param.min_value)) : (currentInt - 2);
    const int maxInt = param.has_max ? static_cast<int>(std::lround(param.max_value)) : (currentInt + 2);
    const int midInt = minInt + ((maxInt - minInt) / 2);
    const int candidates[] = {
      midInt,
      minInt,
      maxInt,
      currentInt + 1,
      currentInt - 1,
    };
    const int candidateCount = static_cast<int>(sizeof(candidates) / sizeof(candidates[0]));
    for (int offset = 0; offset < candidateCount; ++offset) {
      const int candidate = candidates[(variantIndex + offset) % candidateCount];
      if (candidate < minInt || candidate > maxInt) {
        continue;
      }
      if (candidate != currentInt) {
        return static_cast<double>(candidate);
      }
    }
    return static_cast<double>(currentInt);
  }

  double minValue = param.has_min ? param.min_value : (currentValue - 1.0);
  double maxValue = param.has_max ? param.max_value : (currentValue + 1.0);
  if (maxValue < minValue) {
    std::swap(minValue, maxValue);
  }
  const double span = maxValue - minValue;
  const double step = (param.has_step && param.step_value > 0.0) ? param.step_value : 0.125;
  const double candidates[] = {
    minValue + span * 0.37,
    maxValue,
    minValue,
    currentValue + step,
    currentValue - step,
  };
  const int candidateCount = static_cast<int>(sizeof(candidates) / sizeof(candidates[0]));
  for (int offset = 0; offset < candidateCount; ++offset) {
    const double candidate = candidates[(variantIndex + offset) % candidateCount];
    if (!std::isfinite(candidate)) {
      continue;
    }
    if (candidate < minValue - 1.0e-9 || candidate > maxValue + 1.0e-9) {
      continue;
    }
    if (!NearlyEqual(candidate, currentValue, 1.0e-6)) {
      return candidate;
    }
  }
  return currentValue;
}

static bool ConfigureDistinctDraftRowParams(
  const FunctionDescriptor& descriptor,
  ColorPipelineRowState* row,
  int variantIndex = 0,
  std::string* outError = nullptr) {
  if (!row) {
    if (outError) *outError = "Advanced color matrix test requires a draft row";
    return false;
  }

  for (const FunctionParamDescriptor& param : descriptor.parameters) {
    if (param.type == "float" || param.type == "int") {
      double currentValue = 0.0;
      if (!TryGetColorPipelineParamNumber(*row, param.path.c_str(), &currentValue, outError)) {
        return false;
      }
      if (!SetColorPipelineParamNumber(
              row,
              param.path.c_str(),
              ChooseDistinctDraftNumberValue(param, currentValue, variantIndex),
              outError)) {
        return false;
      }
      continue;
    }

    if (param.type == "enum") {
      std::string currentValue;
      if (!TryGetColorPipelineParamEnum(*row, param.path.c_str(), &currentValue, outError)) {
        return false;
      }
      std::string nextValue = currentValue;
      const int optionCount = static_cast<int>(param.options.size());
      for (int offset = 0; offset < optionCount; ++offset) {
        const UISchemaOption& option = param.options[(variantIndex + offset) % optionCount];
        if (option.id != currentValue) {
          nextValue = option.id;
          break;
        }
      }
      if (!color_pipeline_core::SetColorPipelineParamEnum(row, param.path.c_str(), nextValue, outError)) {
        return false;
      }
      continue;
    }

    if (param.type == "bool") {
      bool currentValue = false;
      bool found = false;
      for (const ColorPipelineParamState& stateParam : row->parameter_values) {
        if (stateParam.path == param.path) {
          currentValue = stateParam.bool_value;
          found = true;
          break;
        }
      }
      if (!found || !SetDraftRowBoolParam(row, param.path.c_str(), !currentValue)) {
        if (outError) {
          *outError = "Missing advanced color bool parameter path '" + param.path + "'";
        }
        return false;
      }
      continue;
    }

    if (outError) {
      *outError = "Unsupported advanced color parameter type '" + param.type + "' in diagnostics matrix test";
    }
    return false;
  }

  return true;
}

static bool ConfigureDistinctDraftRowParams(
  const FunctionDescriptor& descriptor,
  ColorPipelineRowState* row,
  std::string* outError) {
  return ConfigureDistinctDraftRowParams(descriptor, row, 0, outError);
}

static bool AppendDraftRowForFunction(
  ColorPipelineWindowState* state,
  const ColorPipelineLaneCatalog& catalog,
  std::size_t laneIndex,
  const FunctionDescriptor& descriptor,
  std::uint64_t stableRowId,
  std::string* outError = nullptr) {
  if (!state || laneIndex >= state->lanes.size()) {
    if (outError) *outError = "Advanced color matrix append requires a valid lane index";
    return false;
  }
  ColorPipelineRowState row;
  if (!BuildColorPipelineRowFromFunctionId(catalog, descriptor.id.c_str(), stableRowId, &row, outError)) {
    return false;
  }
  state->lanes[laneIndex].rows.push_back(std::move(row));
  if (state->next_row_id <= stableRowId) {
    state->next_row_id = stableRowId + 1;
  }
  return true;
}

static bool ResolveSupportedSourceAndGradingForPaletteFunction(
  const ColorPipelineLaneCatalog& sourceCatalog,
  const FunctionDescriptor& paletteDescriptor,
  const ColorPipelineLaneCatalog& gradingCatalog,
  const FunctionDescriptor** outSourceDescriptor,
  const FunctionDescriptor** outGradingDescriptor,
  std::string* outError = nullptr) {
  if (outSourceDescriptor) *outSourceDescriptor = nullptr;
  if (outGradingDescriptor) *outGradingDescriptor = nullptr;
  for (const FunctionDescriptor& sourceDescriptor : sourceCatalog.functions) {
    ColorPipelineSelection pipeline{};
    ColoringMode mode = ColoringMode::root_basin;
    if (!TryBuildColorPipelineSelectionFromLaneIds(sourceDescriptor.id.c_str(), paletteDescriptor.id.c_str(), &pipeline, &mode)) {
      continue;
    }
    const char* gradingFunctionId = AdvancedColorGradingFunctionId(pipeline.grading);
    if (!gradingFunctionId) {
      continue;
    }
    const FunctionDescriptor* gradingDescriptor = FindColorPipelineFunctionDescriptor(gradingCatalog, gradingFunctionId);
    if (!gradingDescriptor) {
      continue;
    }
    if (outSourceDescriptor) *outSourceDescriptor = &sourceDescriptor;
    if (outGradingDescriptor) *outGradingDescriptor = gradingDescriptor;
    return true;
  }
  if (outError) {
    *outError = "No supported Source / Grading owner was found for Palette function '" + paletteDescriptor.id + "'";
  }
  return false;
}

static bool ReadTextFile(const std::filesystem::path& path, std::string* outText) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file) return false;
  std::ostringstream text;
  text << file.rdbuf();
  *outText = text.str();
  return true;
}

static bool ExpectManualExplainoJoyCaptureState(
  const ViewState& view,
  const KernelParams& params,
  const RenderSettings& render,
  const SidecarOrientationVector& orientation,
  bool hasOrientation,
  const SidecarAutoDemoControllerPolicy& controllerPolicy,
  bool hasControllerPolicy,
  bool hasMutationHistory,
  const ColorPipelineWindowState& draft,
  const char* label,
  double expectedCenterHpX = -1.84013,
  double expectedLog2Zoom = 1.37504,
  double expectedSmoothScale = 1.01033,
  double expectedGradeExposure = 1.19035,
  double expectedPaletteSeedScale = 1.45029,
  double expectedPaletteColorfulness = 0.74803) {
  if (view.fractal_type != FractalType::explaino_joy ||
      !NearlyEqual(view.center_hp_x, expectedCenterHpX, 1.0e-12) ||
      !NearlyEqual(view.center_hp_y, 0.00927063, 1.0e-6) ||
      !NearlyEqual(view.log2_zoom, expectedLog2Zoom, 1.0e-12) ||
      !NearlyEqual(view.explaino_phase_strength, 0.14458f, 1.0e-5)) {
    std::cerr << label << ": expected manual explaino_joy view fields to survive diagnostics state load\n";
    return false;
  }
  if (render.resolution.x != 64 || render.resolution.y != 64 ||
      render.block_size != 256 || render.device_id != 0 || render.sample_tier != SampleTier::fast) {
    std::cerr << label << ": expected the low-resolution manual capture render settings to survive diagnostics state load\n";
    return false;
  }
  if (params.max_iter != 376 || params.coloring_mode != ColoringMode::smooth_escape ||
      params.color_pipeline.signal != ColorSignal::smooth_escape ||
      params.color_shape != ColorPipelineShape::mirror_repeat ||
      params.color_pipeline.palette != ColorPalette::explaino_cmap ||
      params.color_pipeline.grading != ColorGradingPreset::escape_default) {
    std::cerr << label << ": expected split advanced-color runtime tuple to survive diagnostics state load\n";
    return false;
  }
  if (!NearlyEqual(params.color_smooth_escape_scale, expectedSmoothScale, 1.0e-7) ||
      !NearlyEqual(params.color_smooth_escape_bias, -0.05512f, 1.0e-5) ||
      !NearlyEqual(params.color_explaino_palette_seed_scale, expectedPaletteSeedScale, 1.0e-5) ||
      !NearlyEqual(params.color_explaino_palette_colorfulness, expectedPaletteColorfulness, 1.0e-5)) {
    std::cerr << label << ": expected Source/Palette owner fields from the manual capture to survive diagnostics state load\n";
    return false;
  }
  if (params.color_shape_stack_count != 1 ||
      params.color_shape_stack[0].shape != ColorPipelineShape::mirror_repeat ||
      !NearlyEqual(params.color_shape_stack[0].params.repeat_frequency, 8.0f, 1.0e-6) ||
      !NearlyEqual(params.color_shape_stack[0].params.repeat_phase, 0.0f, 1.0e-6)) {
    std::cerr << label << ": expected the manual capture Shape stack to survive diagnostics state load\n";
    return false;
  }
  if (params.color_grading_stack_count != 1 ||
      params.color_grading_stack[0].grading != ColorGradingPreset::escape_default ||
      !NearlyEqual(params.color_grading_stack[0].params.exposure, expectedGradeExposure, 1.0e-7) ||
      !NearlyEqual(params.color_grading_stack[0].params.saturation, 1.3622f, 1.0e-5) ||
      !NearlyEqual(params.color_contrast_lift_exposure, expectedGradeExposure, 1.0e-7) ||
      !NearlyEqual(params.color_contrast_lift_saturation, 1.3622f, 1.0e-5)) {
    std::cerr << label << ": expected the manual capture Grading stack and legacy mirror to survive diagnostics state load\n";
    return false;
  }
  if (!hasOrientation ||
      orientation.import_signature != 9475387712945145731ull ||
      orientation.pack_projection_hash != 17921396840264098233ull ||
      !NearlyEqual(orientation.field_embedding_stats, 10.0081, 1.0e-5) ||
      !NearlyEqual(orientation.diff_magnitude, 0.276757, 1.0e-6)) {
    std::cerr << label << ": expected sidecar orientation hashes and metrics to survive diagnostics state load\n";
    return false;
  }
  SidecarAutoDemoControllerPolicy expectedPolicy{};
  if (!hasControllerPolicy || !ControllerPoliciesMatch(controllerPolicy, expectedPolicy)) {
    std::cerr << label << ": expected disabled sidecar auto-demo policy to remain explicitly persisted\n";
    return false;
  }
  if (hasMutationHistory) {
    std::cerr << label << ": manual capture fixture should not invent sidecar mutation history\n";
    return false;
  }
  if (draft.next_row_id != 8 || draft.lanes.size() != 4 ||
      draft.lanes[0].lane_id != "source" || draft.lanes[0].rows.size() != 1 ||
      draft.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
      !DraftRowHasNumberParam(draft.lanes[0].rows[0], "signal.scale", expectedSmoothScale, 1.0e-12) ||
      !DraftRowHasNumberParam(draft.lanes[0].rows[0], "signal.bias", -0.05512, 1.0e-5) ||
      draft.lanes[1].lane_id != "shape" || draft.lanes[1].rows.size() != 1 ||
      draft.lanes[1].rows[0].function_id != "mirror_repeat" ||
      !DraftRowHasNumberParam(draft.lanes[1].rows[0], "shape.frequency", 8.0) ||
      draft.lanes[2].lane_id != "palette" || draft.lanes[2].rows.size() != 1 ||
      draft.lanes[2].rows[0].function_id != "explaino_cmap" ||
      !DraftRowHasNumberParam(draft.lanes[2].rows[0], "palette.seed_scale", 1.45029, 1.0e-5) ||
      !DraftRowHasNumberParam(draft.lanes[2].rows[0], "palette.colorfulness", 0.74803, 1.0e-5) ||
      draft.lanes[3].lane_id != "grading" || draft.lanes[3].rows.size() != 1 ||
      draft.lanes[3].rows[0].function_id != "contrast_lift" ||
      !DraftRowHasNumberParam(draft.lanes[3].rows[0], "grade.exposure", expectedGradeExposure, 1.0e-12) ||
      !DraftRowHasNumberParam(draft.lanes[3].rows[0], "grade.saturation", 1.3622, 1.0e-5)) {
    std::cerr << label << ": expected the real manual advanced-color draft rows to survive diagnostics state load\n";
    return false;
  }
  return true;
}

static const char* ManualExplainoJoyCaptureStateJsonLowRes() {
  return R"({
  "state_version": 3,
  "fractal_type": "explaino_joy",
  "view": {
    "center_x": -1.84013,
    "center_y": 0.00927063,
    "zoom": 2.59374,
    "rotation_degrees": 0,
    "center_hp_x": -1.84013,
    "center_hp_y": 0.00927063,
    "log2_zoom": 1.37504,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true,
    "auto_max_iter": false,
    "auto_increment_seed": false,
    "explaino_seed_rate": 0.001,
    "explaino_phase_strength": 0.14458
  },
  "params": {
    "max_iter": 376,
    "epsilon": 1e-06,
    "exposure": 1,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "mirror_repeat",
    "color_palette": "explaino_cmap",
    "color_grading": "escape_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.01,
    "phoenix_p_imag": 0.01,
    "multibrot_power": 3,
    "multibrot_power_float": 3,
    "lambda_real": 2.96859,
    "lambda_imag": -0.274461,
    "explaino_seed": 0,
    "explaino_seed_b": 1,
    "explaino_mix": 0.5,
    "explaino_warp_strength": 0.04097,
    "explaino_root_spread": 0.18193,
    "explaino_root_count": 4,
    "explaino_cluster_radius": 0,
    "joy_coupling": 0.3,
    "fold_coupling": 0,
    "bell_coupling": 0,
    "ripple_amplitude": 0,
    "splice_offset": 0,
    "vortex_strength": 0,
    "tension_strength": 0,
    "transcendental_func": "f_sin",
    "momentum_beta": 0,
    "mcmullen_preset": "z3_z3",
    "poly_coeffs": [0.699349, 1.74308, 2.62498, 2.1946, 1],
    "color_saturation": 1.15,
    "color_contrast": 1.1,
    "color_tint_r": 1,
    "color_tint_g": 1,
    "color_tint_b": 1,
    "color_phase_signal_offset": 0,
    "color_phase_wrap_cycles": 1,
    "color_phase_palette_offset": 0,
    "color_shape_offset": 0,
    "color_shape_scale": 1,
    "color_shape_repeat_frequency": 8,
    "color_shape_repeat_phase": 0,
    "color_shape_posterize_steps": 6,
    "color_shape_posterize_mix": 1,
    "color_shape_bias": 0.5,
    "color_shape_gain": 0.5,
    "color_shape_window_center": 0.5,
    "color_shape_window_width": 1,
    "color_shape_window_softness": 0,
    "color_shape_stack": [
      {
        "shape": "mirror_repeat",
        "offset": 0,
        "scale": 1,
        "repeat_frequency": 8,
        "repeat_phase": 0,
        "posterize_steps": 6,
        "posterize_mix": 1,
        "bias": 0.5,
        "gain": 0.5,
        "window_center": 0.5,
        "window_width": 1,
        "window_softness": 0
      }
    ],
    "color_grading_stack": [
      {
        "grading": "escape_default",
        "exposure": 1.19035,
        "saturation": 1.3622,
        "contrast": 1
      }
    ],
    "color_iteration_band_count": 8,
    "color_iteration_band_softness": 0.35,
    "color_iteration_band_emphasis": 1,
    "color_iteration_band_palette_offset": 0,
    "color_smooth_escape_scale": 1.01033,
    "color_smooth_escape_bias": -0.05512,
    "color_escape_magnitude_scale": 1,
    "color_escape_magnitude_bias": 0,
    "color_orbit_stripe_frequency": 1,
    "color_orbit_stripe_phase": 0,
    "color_root_proximity_scale": 1,
    "color_root_proximity_bias": 0,
    "color_heatmap_cycle_scale": 1,
    "color_heatmap_saturation": 1,
    "color_explaino_palette_seed_scale": 1.45029,
    "color_explaino_palette_seed_phase": 1,
    "color_explaino_palette_colorfulness": 0.74803,
    "color_contrast_lift_exposure": 1.19035,
    "color_contrast_lift_saturation": 1.3622
  },
  "render": {
    "width": 64,
    "height": 64,
    "interaction_debounce_ms": 200,
    "preview_target_fps": 30,
    "preview_min_scale": 0.5,
    "block_size": 256,
    "device_id": 0,
    "sample_tier": "fast"
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 9,
    "last_device_id": 0,
    "resolved_backend": "float32",
    "resolved_strategy": "direct"
  },
  "sidecar_orientation": {
    "import_signature": "9475387712945145731",
    "pack_projection_hash": "17921396840264098233",
    "field_embedding_stats": 10.0081,
    "slime_energy_delta": 10.0081,
    "busy_beaver_metrics": 0.517241,
    "decode_stability": 0.687303,
    "diff_magnitude": 0.276757
  },
  "sidecar_auto_demo_policy": {
    "enabled": false,
    "allow_runtime_mutation": false,
    "run_paced_loop": false,
    "paced_loop_interval_seconds": 1,
    "stop_demonstrated_fraction": 1,
    "stop_uncertain_count": 0
  },
  "color_pipeline_draft": {
    "next_row_id": 8,
    "lanes": [
      {
        "lane_id": "source",
        "label": "Source",
        "rows": [
        {
          "ui_row_id": 6,
          "enabled": true,
          "function_id": "smooth_escape_ramp",
          "parameter_values": [
          {
            "path": "signal.scale",
            "type": "float",
            "number_value": 1.01033
          },
          {
            "path": "signal.bias",
            "type": "float",
            "number_value": -0.05512
          }
          ]
        }
        ]
      },
      {
        "lane_id": "shape",
        "label": "Shape",
        "rows": [
        {
          "ui_row_id": 2,
          "enabled": true,
          "function_id": "mirror_repeat",
          "parameter_values": [
          {
            "path": "shape.frequency",
            "type": "float",
            "number_value": 8
          },
          {
            "path": "shape.phase",
            "type": "float",
            "number_value": 0
          }
          ]
        }
        ]
      },
      {
        "lane_id": "palette",
        "label": "Palette",
        "rows": [
        {
          "ui_row_id": 3,
          "enabled": true,
          "function_id": "explaino_cmap",
          "parameter_values": [
          {
            "path": "palette.seed_scale",
            "type": "float",
            "number_value": 1.45029
          },
          {
            "path": "palette.seed_phase",
            "type": "float",
            "number_value": 1
          },
          {
            "path": "palette.colorfulness",
            "type": "float",
            "number_value": 0.74803
          }
          ]
        }
        ]
      },
      {
        "lane_id": "grading",
        "label": "Grading",
        "rows": [
        {
          "ui_row_id": 7,
          "enabled": true,
          "function_id": "contrast_lift",
          "parameter_values": [
          {
            "path": "grade.exposure",
            "type": "float",
            "number_value": 1.19035
          },
          {
            "path": "grade.saturation",
            "type": "float",
            "number_value": 1.3622
          }
          ]
        }
        ]
      }
    ]
  }
})";
}

static void WriteMinimalStateWithExtraParams(
  const std::filesystem::path& statePath,
  const std::string& extraParamsJson,
  const std::string& extraRenderJson = "") {
  std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
  file << "{\n"
      "  \"state_version\": 3,\n"
      "  \"fractal_type\": \"newton\",\n"
      "  \"view\": {\n"
      "    \"center_x\": 0,\n"
      "    \"center_y\": 0,\n"
      "    \"zoom\": 1,\n"
      "    \"rotation_degrees\": 0,\n"
      "    \"center_hp_x\": 0,\n"
      "    \"center_hp_y\": 0,\n"
      "    \"log2_zoom\": 0,\n"
      "    \"explaino_phase\": 0,\n"
      "    \"explaino_seed_drift\": 0,\n"
      "    \"explaino_seed_tween\": true\n"
      "  },\n"
      "  \"params\": {\n"
      "    \"max_iter\": 500,\n"
      "    \"epsilon\": 0.000001,\n"
      "    \"exposure\": 1.0,\n"
      "    \"poly_kind\": 0,\n"
      "    \"coloring_mode\": \"root_basin\",\n"
      "    \"nova_alpha\": 0.5,\n"
      "    \"phoenix_p_real\": 0.0,\n"
      "    \"phoenix_p_imag\": 0.0,\n"
      "    \"multibrot_power\": 3,\n"
      "    \"multibrot_power_float\": 3.0,\n"
      "    \"lambda_real\": 0.0,\n"
      "    \"lambda_imag\": 0.0,\n"
      "    \"explaino_seed\": 0,\n"
      "    \"explaino_warp_strength\": 0,\n"
      "    \"explaino_root_count\": 0,\n"
      "    \"poly_coeffs\": [-1, 0, 0, 1, 0]";
  if (!extraParamsJson.empty()) {
    file << ",\n" << extraParamsJson;
  }
  file << "\n  },\n"
      "  \"render\": {\n"
      "    \"width\": 1024,\n"
      "    \"height\": 768,\n"
      "    \"block_size\": 256,\n"
      "    \"device_id\": 0";
  if (!extraRenderJson.empty()) {
    file << ",\n" << extraRenderJson;
  }
  file << "\n"
      "  }\n"
      "}";
}

static bool ExpectLoadDiagnosticsStateFailure(const std::filesystem::path& statePath,
  const std::string& expectedErrorSubstring,
  std::string* outObservedError) {
  ViewState view{};
  KernelParams params{};
  RenderSettings render{};
  std::string error;
  if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
    if (outObservedError) *outObservedError = "load unexpectedly succeeded";
    return false;
  }
  if (error.find(expectedErrorSubstring) == std::string::npos) {
    if (outObservedError) *outObservedError = error;
    return false;
  }
  return true;
}

int main() {
    namespace fs = std::filesystem;

    const fs::path tempRoot = fs::temp_directory_path() / "cuda_newton_fractal_clone_state_io_tests";
    fs::create_directories(tempRoot);

    {
        const fs::path statePath = tempRoot / "loaded_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
        "state_version": 3,
  "fractal_type": "phoenix",
  "view": {
    "center_x": 0.125,
    "center_y": -0.375,
    "zoom": 2.5,
    "rotation_degrees": 12.0,
    "center_hp_x": 0.125,
    "center_hp_y": -0.375,
    "log2_zoom": 1.3219280948873624,
    "explaino_phase": 0.75,
    "explaino_seed_drift": 0.125,
    "explaino_seed_tween": false,
    "auto_max_iter": true,
    "explaino_phase_strength": -2.5
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.6,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.55,
    "phoenix_p_real": 0.5667,
    "phoenix_p_imag": -0.125,
    "multibrot_power": 5,
    "explaino_seed": 4,
    "explaino_warp_strength": 0.3,
    "explaino_root_spread": 1.75,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1440,
    "height": 900,
    "block_size": 512,
    "device_id": 1,
    "sample_tier": "standard",
    "interaction_debounce_ms": 420,
    "preview_target_fps": 24.0,
    "preview_min_scale": 0.4
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        SidecarOrientationVector orientation{};
        bool hasOrientation = true;
        SidecarAutoDemoControllerPolicy controllerPolicy{};
        bool hasControllerPolicy = true;
        std::string error;
        if (!LoadDiagnosticsStateFile(
                statePath.string(),
                &view,
                &params,
                &render,
                &orientation,
                &hasOrientation,
                &controllerPolicy,
                &hasControllerPolicy,
                &error)) {
            std::cerr << "LoadDiagnosticsStateFile failed: " << error << "\n";
            return 1;
        }
        if (hasOrientation) {
          std::cerr << "Expected legacy diagnostics state loads without sidecar_orientation to report no persisted orientation\n";
          return 1;
        }
        if (hasControllerPolicy) {
          std::cerr << "Expected legacy diagnostics state loads without sidecar_auto_demo_policy to report no persisted controller policy\n";
          return 1;
        }

        if (view.fractal_type != FractalType::phoenix) {
            std::cerr << "fractal_type mismatch\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, 0.125f, 1.0e-6) || !NearlyEqual(view.center.y, -0.375f, 1.0e-6)) {
            std::cerr << "view center mismatch\n";
            return 1;
        }
        if (!NearlyEqual(view.center_hp_x, 0.125, 1.0e-12) || !NearlyEqual(view.center_hp_y, -0.375, 1.0e-12)) {
            std::cerr << "view HP center mismatch\n";
            return 1;
        }
        if (!NearlyEqual(view.log2_zoom, 1.3219280948873624, 1.0e-12)) {
            std::cerr << "view log2_zoom mismatch\n";
            return 1;
        }
        if (!NearlyEqual(view.explaino_phase_strength, -2.5f, 1.0e-6)) {
          std::cerr << "view explaino_phase_strength mismatch\n";
          return 1;
        }
        if (view.explaino_seed_tween != false) {
            std::cerr << "view explaino_seed_tween mismatch\n";
            return 1;
        }
        if (!view.auto_max_iter) {
          std::cerr << "view auto_max_iter should load from saved state when present\n";
          return 1;
        }
        if (view.explaino_alive) {
          std::cerr << "view explaino_alive should default to false when missing from saved state\n";
          return 1;
        }
        if (view.auto_increment_seed) {
          std::cerr << "view auto_increment_seed should default to false when missing from saved state\n";
          return 1;
        }
        if (!NearlyEqual(view.explaino_seed_rate, 0.001f, 1.0e-6)) {
          std::cerr << "new Explaino seed rate default should survive older saved states\n";
          return 1;
        }
        if (params.max_iter != 1200 || !NearlyEqual(params.exposure, 1.6f, 1.0e-6)) {
            std::cerr << "params mismatch\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "params poly_kind mismatch\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
          std::cerr << "params coloring_mode mismatch\n";
          return 1;
        }
        if (!NearlyEqual(params.nova_alpha, 0.55f, 1.0e-6) || !NearlyEqual(params.phoenix_p_real, 0.5667f, 1.0e-6) || !NearlyEqual(params.phoenix_p_imag, -0.125f, 1.0e-6)) {
          std::cerr << "params family-specific values mismatch\n";
          return 1;
        }
        if (params.multibrot_power != 5) {
          std::cerr << "params multibrot_power mismatch\n";
          return 1;
        }
        if (!NearlyEqual(params.explaino_root_spread, 1.75f, 1.0e-6)) {
          std::cerr << "params explaino_root_spread mismatch\n";
          return 1;
        }
        if (!NearlyEqual(params.multibrot_power_float, 5.0f, 1.0e-6)) {
          std::cerr << "params multibrot_power_float should fall back to legacy int value\n";
          return 1;
        }
        if (render.resolution.x != 1440 || render.resolution.y != 900 || render.block_size != 512 || render.device_id != 1) {
            std::cerr << "render mismatch\n";
            return 1;
        }
        if (render.sample_tier != SampleTier::standard) {
          std::cerr << "render sample_tier should load from saved diagnostics state\n";
          return 1;
        }
        if (render.interaction_debounce_ms != 420 || !NearlyEqual(render.preview_target_fps, 24.0f, 1.0e-6) || !NearlyEqual(render.preview_min_scale, 0.4f, 1.0e-6)) {
          std::cerr << "render adaptive preview pacing mismatch\n";
          return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "legacy_state_without_explicit_explaino_runtime_authority.json";
        WriteMinimalStateWithExtraParams(statePath, "");

        ViewState view{};
        KernelParams params{};
        params.explaino_root_count = 4;
        params.explaino_roots[0] = {9.0f, 8.0f};
        params.explaino_roots[3] = {7.0f, 6.0f};
        params.poly_coeffs_b[0] = 5.0f;
        params.poly_coeffs_b[4] = 4.0f;
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected legacy state without explicit ExplainO runtime authority to load: " << error << "\n";
            return 1;
        }
        if (params.explaino_root_count != 0 ||
            !NearlyEqual(params.explaino_roots[0].x, 0.0f, 1.0e-6) ||
            !NearlyEqual(params.explaino_roots[0].y, 0.0f, 1.0e-6) ||
            !NearlyEqual(params.explaino_roots[3].x, 0.0f, 1.0e-6) ||
            !NearlyEqual(params.explaino_roots[3].y, 0.0f, 1.0e-6)) {
            std::cerr << "Expected legacy state load to clear stale ExplainO roots when explaino_roots is absent\n";
            return 1;
        }
        if (!NearlyEqual(params.poly_coeffs_b[0], 0.0f, 1.0e-6) ||
            !NearlyEqual(params.poly_coeffs_b[4], 0.0f, 1.0e-6)) {
            std::cerr << "Expected legacy state load to clear stale secondary ExplainO polynomial when poly_coeffs_b is absent\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_sample_tier_state.json";
        WriteMinimalStateWithExtraParams(statePath, "", "    \"sample_tier\": \"warp_speed\"");
        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "sample_tier", &error)) {
          std::cerr << "Expected unknown sample_tier to fail: " << error << "\n";
          return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "negative_explaino_seed_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino_dual",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.5,
    "explaino_seed_drift": 0.25,
    "explaino_seed_tween": true,
    "explaino_phase_strength": -1.25
  },
  "params": {
    "max_iter": 650,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": -3.0,
    "explaino_seed_b": -7.5,
    "explaino_mix": 0.35,
    "explaino_warp_strength": 0.2,
    "explaino_root_spread": 2.25,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected negative Explaino seeds to load: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, -3.0, 1.0e-9) || !NearlyEqual(params.explaino_seed_b, -7.5, 1.0e-9)) {
            std::cerr << "Expected Explaino seed controls to preserve negative values\n";
            return 1;
        }
        if (!NearlyEqual(view.explaino_phase_strength, -1.25f, 1.0e-6) || !NearlyEqual(params.explaino_root_spread, 2.25f, 1.0e-6)) {
            std::cerr << "Expected new Explaino state fields to round-trip from state JSON\n";
            return 1;
        }
    }


    {
        const fs::path statePath = tempRoot / "sidecar_orientation_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 7.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [1.0, 0.0, 0.0, 1.0, 1.0]
  },
  "render": {
    "width": 800,
    "height": 600,
    "block_size": 256,
    "device_id": 0
  },
  "sidecar_orientation": {
    "import_signature": "9007199254740993",
    "pack_projection_hash": "18446744073709551614",
    "field_embedding_stats": 3.5,
    "slime_energy_delta": 1.25,
    "busy_beaver_metrics": 0.75,
    "decode_stability": 0.5,
    "diff_magnitude": 2.0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        SidecarOrientationVector orientation{};
        bool hasOrientation = false;
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &orientation, &hasOrientation, &error)) {
            std::cerr << "Expected sidecar orientation state to load: " << error << "\n";
            return 1;
        }
        if (!hasOrientation) {
            std::cerr << "Expected diagnostics state load to report persisted sidecar orientation when present\n";
            return 1;
        }
        if (orientation.import_signature != 9007199254740993ull ||
            orientation.pack_projection_hash != 18446744073709551614ull ||
            !NearlyEqual(orientation.field_embedding_stats, 3.5) ||
            !NearlyEqual(orientation.slime_energy_delta, 1.25) ||
            !NearlyEqual(orientation.busy_beaver_metrics, 0.75) ||
            !NearlyEqual(orientation.decode_stability, 0.5) ||
            !NearlyEqual(orientation.diff_magnitude, 2.0)) {
            std::cerr << "Expected persisted sidecar orientation values to round-trip through diagnostics state loading\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "sidecar_controller_policy_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino_dual",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 6.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [1.0, 0.0, 0.0, 1.0, 1.0]
  },
  "render": {
    "width": 800,
    "height": 600,
    "block_size": 256,
    "device_id": 0
  },
  "sidecar_auto_demo_policy": {
    "enabled": true,
    "allow_runtime_mutation": true,
    "run_paced_loop": true,
    "paced_loop_interval_seconds": 2.5,
    "stop_demonstrated_fraction": 0.75,
    "stop_uncertain_count": 3
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        SidecarOrientationVector orientation{};
        bool hasOrientation = true;
        SidecarAutoDemoControllerPolicy controllerPolicy{};
        bool hasControllerPolicy = false;
        std::string error;
        if (!LoadDiagnosticsStateFile(
                statePath.string(),
                &view,
                &params,
                &render,
                &orientation,
                &hasOrientation,
                &controllerPolicy,
                &hasControllerPolicy,
                &error)) {
            std::cerr << "Expected sidecar controller policy state to load: " << error << "\n";
            return 1;
        }
        if (hasOrientation) {
            std::cerr << "Expected controller-policy state without sidecar_orientation to report no persisted orientation\n";
            return 1;
        }
        if (!hasControllerPolicy) {
            std::cerr << "Expected diagnostics state load to report persisted sidecar controller policy when present\n";
            return 1;
        }

        SidecarAutoDemoControllerPolicy expectedPolicy{};
        expectedPolicy.enabled = true;
        expectedPolicy.allow_runtime_mutation = true;
        expectedPolicy.run_paced_loop = true;
        expectedPolicy.paced_loop_interval_seconds = 2.5;
        expectedPolicy.stop_demonstrated_fraction = 0.75;
        expectedPolicy.stop_uncertain_count = 3;
        if (!ControllerPoliciesMatch(controllerPolicy, expectedPolicy)) {
            std::cerr << "Expected persisted sidecar controller policy values to round-trip through diagnostics state loading\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "sidecar_mutation_history_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 7.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [1.0, 0.0, 0.0, 1.0, 1.0]
  },
  "render": {
    "width": 800,
    "height": 600,
    "block_size": 256,
    "device_id": 0
  },
  "sidecar_mutation_history": [
    {
      "label": "Ripple amplitude",
      "path": "fractal.params.ripple_amplitude",
      "type": "float",
      "target_value": 0.15,
      "utility": 1.25
    },
    {
      "label": "Seed",
      "path": "fractal.params.explaino_seed",
      "type": "double",
      "target_value": 3.5,
      "utility": 0.75
    },
    {
      "label": "Max Iter",
      "path": "fractal.params.max_iter",
      "type": "int",
      "target_value": 650,
      "utility": 0.25
    }
  ]
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        SidecarOrientationVector orientation{};
        bool hasOrientation = false;
        SidecarAutoDemoControllerPolicy controllerPolicy{};
        bool hasControllerPolicy = false;
        SidecarAutoDemoMutationHistory mutationHistory;
        bool hasMutationHistory = false;
        std::string error;
        if (!LoadDiagnosticsStateFile(
                statePath.string(),
                &view,
                &params,
                &render,
                &orientation,
                &hasOrientation,
                &controllerPolicy,
                &hasControllerPolicy,
                &mutationHistory,
                &hasMutationHistory,
                &error)) {
            std::cerr << "Expected sidecar mutation history state to load: " << error << "\n";
            return 1;
        }
        if (hasOrientation) {
            std::cerr << "Expected mutation-history state without sidecar_orientation to report no persisted orientation\n";
            return 1;
        }
        if (hasControllerPolicy) {
            std::cerr << "Expected mutation-history state without sidecar_auto_demo_policy to report no persisted controller policy\n";
            return 1;
        }
        if (!hasMutationHistory) {
            std::cerr << "Expected diagnostics state load to report persisted sidecar mutation history when present\n";
            return 1;
        }

        SidecarAutoDemoMutationHistory expectedHistory;
        expectedHistory.push_back({"Ripple amplitude", "fractal.params.ripple_amplitude", "float", 0.15, 1.25});
        expectedHistory.push_back({"Seed", "fractal.params.explaino_seed", "double", 3.5, 0.75});
        expectedHistory.push_back({"Max Iter", "fractal.params.max_iter", "int", 650.0, 0.25});
        if (!MutationHistoriesMatch(mutationHistory, expectedHistory)) {
            std::cerr << "Expected persisted sidecar mutation history values to round-trip through diagnostics state loading\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "invalid_sidecar_mutation_history_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 7.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [1.0, 0.0, 0.0, 1.0, 1.0]
  },
  "render": {
    "width": 800,
    "height": 600,
    "block_size": 256,
    "device_id": 0
  },
  "sidecar_mutation_history": [
    {
      "label": "Ripple amplitude",
      "path": "fractal.params.ripple_amplitude",
      "type": "float",
      "target_value": "oops",
      "utility": 1.25
    }
  ]
})";
        file.close();

        std::string observedError;
        if (!ExpectLoadDiagnosticsStateFailure(
                statePath,
                "Invalid sidecar_mutation_history[0].target_value",
                &observedError)) {
            std::cerr << "Unexpected invalid mutation-history error text: " << observedError << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "invalid_sidecar_controller_policy_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino_dual",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 6.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [1.0, 0.0, 0.0, 1.0, 1.0]
  },
  "render": {
    "width": 800,
    "height": 600,
    "block_size": 256,
    "device_id": 0
  },
  "sidecar_auto_demo_policy": {
    "enabled": true,
    "allow_runtime_mutation": true,
    "run_paced_loop": true,
    "paced_loop_interval_seconds": 0.0,
    "stop_demonstrated_fraction": 0.75,
    "stop_uncertain_count": 3
  }
})";
        file.close();

        std::string observedError;
        if (!ExpectLoadDiagnosticsStateFailure(
                statePath,
                "sidecar_auto_demo_policy.paced_loop_interval_seconds must be > 0",
                &observedError)) {
            std::cerr << "Unexpected invalid controller-policy error text: " << observedError << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "fractional_sidecar_controller_policy_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino_dual",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 6.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [1.0, 0.0, 0.0, 1.0, 1.0]
  },
  "render": {
    "width": 800,
    "height": 600,
    "block_size": 256,
    "device_id": 0
  },
  "sidecar_auto_demo_policy": {
    "enabled": true,
    "allow_runtime_mutation": true,
    "run_paced_loop": true,
    "paced_loop_interval_seconds": 1.0,
    "stop_demonstrated_fraction": 0.75,
    "stop_uncertain_count": 3.5
  }
})";
        file.close();

        std::string observedError;
        if (!ExpectLoadDiagnosticsStateFailure(
                statePath,
                "Invalid integer field: stop_uncertain_count",
                &observedError)) {
            std::cerr << "Unexpected fractional controller-policy error text: " << observedError << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "sidecar_orientation_numeric_legacy_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 7.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [1.0, 0.0, 0.0, 1.0, 1.0]
  },
  "render": {
    "width": 800,
    "height": 600,
    "block_size": 256,
    "device_id": 0
  },
  "sidecar_orientation": {
    "import_signature": 11,
    "pack_projection_hash": 17,
    "field_embedding_stats": 3.5,
    "slime_energy_delta": 1.25,
    "busy_beaver_metrics": 0.75,
    "decode_stability": 0.5,
    "diff_magnitude": 2.0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        SidecarOrientationVector orientation{};
        bool hasOrientation = false;
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &orientation, &hasOrientation, &error)) {
            std::cerr << "Expected legacy numeric sidecar hashes to remain loadable: " << error << "\n";
            return 1;
        }
        if (!hasOrientation || orientation.import_signature != 11u || orientation.pack_projection_hash != 17u) {
            std::cerr << "Expected legacy numeric sidecar hashes to remain backward compatible\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "composed_variant_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino_vortex",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true,
    "auto_max_iter": false,
    "auto_increment_seed": false,
    "explaino_seed_rate": 0.001,
    "explaino_phase_strength": 1.0
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 2.9685855,
    "lambda_imag": -0.27446103,
    "explaino_seed": 2.0,
    "explaino_seed_b": 1.0,
    "explaino_mix": 0.5,
    "explaino_warp_strength": 0.0,
    "explaino_root_spread": 0.5,
    "explaino_root_count": 4,
    "explaino_cluster_radius": 0.0,
    "joy_coupling": 0.0,
    "fold_coupling": 0.0,
    "bell_coupling": 0.0,
    "ripple_amplitude": 0.0,
    "splice_offset": 0.0,
    "vortex_strength": 0.3,
    "tension_strength": 0.0,
    "transcendental_func": "f_sin",
    "momentum_beta": 0.0,
    "mcmullen_preset": "z3_z3",
    "poly_coeffs": [1.0, 0.0, 0.0, 1.0, 1.0],
    "color_saturation": 1.15,
    "color_contrast": 1.1,
    "color_tint_r": 1.0,
    "color_tint_g": 1.0,
    "color_tint_b": 1.0
  },
  "render": {
    "width": 320,
    "height": 240,
    "interaction_debounce_ms": 200,
    "preview_target_fps": 30.0,
    "preview_min_scale": 0.5,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Composed variant state should load: " << error << "\n";
            return 1;
        }

        if (view.fractal_type != FractalType::explaino_vortex) {
            std::cerr << "Composed variant fractal_type should round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.ripple_amplitude, 0.0f, 1.0e-6) ||
            !NearlyEqual(params.splice_offset, 0.0f, 1.0e-6) ||
            !NearlyEqual(params.vortex_strength, 0.3f, 1.0e-6) ||
            !NearlyEqual(params.tension_strength, 0.0f, 1.0e-6)) {
            std::cerr << "Composed Explaino strength params should round-trip through diagnostics state loading\n";
            return 1;
        }
    }
    {
        const fs::path statePath = tempRoot / "legacy_explaino_nova_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino_nova",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true,
    "explaino_phase_strength": 1.0
  },
  "params": {
    "max_iter": 300,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 0.0,
    "explaino_seed_b": 1.0,
    "explaino_mix": 0.5,
    "explaino_warp_strength": 0.0,
    "explaino_root_spread": 0.5,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected legacy Explaino-Nova state to load: " << error << "\n";
            return 1;
        }
        if (!view.auto_max_iter) {
            std::cerr << "Legacy Explaino-Nova states should default auto_max_iter on when the field is missing\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "multibrot_float_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "multibrot",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": false
  },
  "params": {
    "max_iter": 1000,
    "epsilon": 0.000001,
    "exposure": 1.4,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 2.5,
    "explaino_seed": 0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected non-integer multibrot state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::multibrot) {
            std::cerr << "Expected multibrot fractal type to load\n";
            return 1;
        }
        if (!NearlyEqual(params.multibrot_power_float, 2.5f, 1.0e-6)) {
            std::cerr << "Expected multibrot_power_float to round-trip from saved state\n";
            return 1;
        }
    }

    {
        const char* stateText = R"({
  "state_version": 3,
  "fractal_type": "explaino_inertial",
  "view": {
    "center_x": -0.30367326736450195,
    "center_y": 0.20949007570743561,
    "zoom": 251.63772583007812,
    "rotation_degrees": 0,
    "center_hp_x": -0.30367326545671292,
    "center_hp_y": 0.20949006969551054,
    "log2_zoom": 7.9752043774962251,
    "explaino_phase": -3.1405699253082275,
    "explaino_seed_drift": 0.76309001445770264,
    "explaino_seed_tween": true,
    "auto_max_iter": false,
    "auto_increment_seed": false,
    "explaino_seed_rate": 0.0010000000474974513,
    "explaino_phase_strength": 2.0722899436950684
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "mirror_repeat",
    "color_palette": "cyclic_escape",
    "color_grading": "escape_default",
    "nova_alpha": 0.75,
    "phoenix_p_real": 0.25,
    "phoenix_p_imag": -0.125,
    "multibrot_power": 3,
    "multibrot_power_float": 3,
    "lambda_real": 1.25,
    "lambda_imag": -0.5,
    "explaino_seed": -1,
    "explaino_seed_b": 1,
    "explaino_mix": 0.5,
    "explaino_warp_strength": 0,
    "explaino_root_spread": 0,
    "explaino_damping": 0.42,
    "explaino_root_count": 4,
    "poly_coeffs": [0.58701878786087036, 1.0171422958374023, 1.9830961227416992, 1.3209570646286011, 1]
  },
  "render": {
    "width": 256,
    "height": 256,
    "block_size": 256,
    "device_id": 0,
    "sample_tier": "tier_auto"
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";

        ViewState view{};
        KernelParams params{};
        params.nova_alpha = 0.1f;
        params.phoenix_p_real = -0.2f;
        params.phoenix_p_imag = 0.3f;
        params.multibrot_power = 2;
        params.multibrot_power_float = 2.0f;
        params.explaino_damping = 1.25f;
        params.lambda_real = -9.0f;
        params.lambda_imag = 9.0f;
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateJson(stateText, &view, &params, &render, &error)) {
            std::cerr << "Expected split-color Explaino-Inertial state to load: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(params.nova_alpha, 0.75f, 1.0e-6) ||
            !NearlyEqual(params.phoenix_p_real, 0.25f, 1.0e-6) ||
            !NearlyEqual(params.phoenix_p_imag, -0.125f, 1.0e-6) ||
            params.multibrot_power != 3 ||
            !NearlyEqual(params.multibrot_power_float, 3.0f, 1.0e-6) ||
            !NearlyEqual(params.explaino_damping, 0.42f, 1.0e-6) ||
            !NearlyEqual(params.lambda_real, 1.25f, 1.0e-6) ||
            !NearlyEqual(params.lambda_imag, -0.5f, 1.0e-6)) {
            std::cerr << "Expected split-color diagnostics state to preserve common fractal params\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "lambda_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "lambda",
  "view": {
    "center_x": 0.5,
    "center_y": 0.0,
    "zoom": 2.0,
    "rotation_degrees": 0,
    "center_hp_x": 0.5,
    "center_hp_y": 0.0,
    "log2_zoom": 1.0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": false
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.4,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 1.0,
    "lambda_imag": 0.25,
    "explaino_seed": 0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected lambda state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::lambda_map) {
            std::cerr << "Expected lambda fractal type to round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.lambda_real, 1.0f, 1.0e-6) || !NearlyEqual(params.lambda_imag, 0.25f, 1.0e-6)) {
            std::cerr << "Expected lambda_real/lambda_imag to round-trip from saved state\n";
            return 1;
        }
    }

    // Explaino-Lambda state round-trip
    {
        const fs::path statePath = tempRoot / "explaino_lambda_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino_lambda",
  "view": {
    "center_x": 0.5,
    "center_y": 0.0,
    "zoom": 4.5,
    "rotation_degrees": 0,
    "center_hp_x": 0.5,
    "center_hp_y": 0.0,
    "log2_zoom": 2.17,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.4,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "lambda_real": 2.0,
    "lambda_imag": -0.5,
    "explaino_seed": 3.5,
    "explaino_warp_strength": 0.2,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected explaino_lambda state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::explaino_lambda) {
            std::cerr << "Expected explaino_lambda fractal type to round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.lambda_real, 2.0f, 1.0e-6) || !NearlyEqual(params.lambda_imag, -0.5f, 1.0e-6)) {
            std::cerr << "Expected lambda_real/lambda_imag to round-trip from explaino_lambda state\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Expected explaino_lambda to use smooth_escape coloring\n";
            return 1;
        }
    }

    // Explaino-Rational-Escape state round-trip
    {
        const fs::path statePath = tempRoot / "explaino_rational_escape_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino_rational_escape",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.8,
    "rotation_degrees": 0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.85,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.2,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 4.2,
    "explaino_warp_strength": 0.3,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected explaino_rational_escape state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::explaino_rational_escape) {
            std::cerr << "Expected explaino_rational_escape fractal type to round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, 4.0, 1.0e-6) || !NearlyEqual(view.explaino_seed_drift, 0.2f, 1.0e-4)) {
            std::cerr << "Expected explaino_seed/drift to round-trip from explaino_rational_escape state\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Expected explaino_rational_escape to use smooth_escape coloring\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "seed_motion_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino_fp",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0.125,
    "explaino_seed_drift": 0.625,
    "explaino_seed_tween": true,
    "auto_increment_seed": true,
    "explaino_seed_rate": 0.6
  },
  "params": {
    "max_iter": 650,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": -0.5,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 4,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 1]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected seed-motion state to load: " << error << "\n";
            return 1;
        }
        if (view.explaino_alive || !view.auto_increment_seed) {
          std::cerr << "Expected auto-increment seed toggle to round-trip while explaino_alive stays at its default\n";
            return 1;
        }
        if (!NearlyEqual(view.explaino_seed_rate, 0.6f, 1.0e-6)) {
          std::cerr << "Expected seed increment rate to round-trip from saved state\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "dual_seed_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino_dual",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0.25,
    "explaino_seed_drift": 0.4,
    "explaino_seed_tween": true,
    "auto_increment_seed": false,
    "explaino_seed_rate": 0.05
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": -0.5,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 4,
    "explaino_seed_b": 9.5,
    "explaino_mix": 0.35,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 1]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected dual-seed state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::explaino_dual) {
            std::cerr << "Expected explaino_dual fractal type to round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, 4.0, 1.0e-9) || !NearlyEqual(params.explaino_seed_b, 9.5, 1.0e-9)) {
            std::cerr << "Expected dual-seed endpoints to round-trip from saved state\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_mix, 0.35f, 1.0e-6)) {
            std::cerr << "Expected dual-seed mix to round-trip from saved state\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "split_color_pipeline_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "iteration_count",
    "color_signal": "root_index",
    "color_palette": "joy",
    "color_grading": "basin_default",
    "nova_alpha": 0.50,
    "phoenix_p_real": -0.50,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected split-color state to load: " << error << "\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::joy_basins) {
            std::cerr << "Expected explicit split-color fields to override conflicting legacy coloring_mode\n";
            return 1;
        }
        if (params.color_pipeline.signal != ColorSignal::root_index ||
            params.color_pipeline.palette != ColorPalette::joy ||
            params.color_pipeline.grading != ColorGradingPreset::basin_default) {
            std::cerr << "Expected split-color fields to round-trip into the runtime color pipeline\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "split_color_pipeline_without_legacy_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "color_signal": "root_index",
    "color_palette": "root_classic",
    "color_grading": "basin_default",
    "nova_alpha": 0.50,
    "phoenix_p_real": -0.50,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected split-only color state to load without requiring legacy coloring_mode: " << error << "\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::root_basin) {
            std::cerr << "Expected split-only color state to synthesize the legacy coloring_mode for the runtime\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 1,
  "fractal_type": "mystery_fractal",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1,
    "poly_kind": 0,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected unknown fractal type to fail\n";
            return 1;
        }
        if (error.find("Unknown fractal_type") == std::string::npos) {
            std::cerr << "Unexpected error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_zero_max_iter_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 0,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected zero max_iter to fail\n";
            return 1;
        }
        if (error.find("max_iter") == std::string::npos) {
            std::cerr << "Unexpected zero-max_iter error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_zero_width_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 0,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected zero width to fail\n";
            return 1;
        }
        if (error.find("width") == std::string::npos) {
            std::cerr << "Unexpected zero-width error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_negative_height_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": -1,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected negative height to fail\n";
            return 1;
        }
        if (error.find("height") == std::string::npos) {
            std::cerr << "Unexpected negative-height error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_coloring_mode_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "phoenix",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.6,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.5667,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected incompatible coloring_mode for phoenix to fail\n";
            return 1;
        }
        if (error.find("not allowed for fractal_type phoenix") == std::string::npos) {
            std::cerr << "Unexpected invalid-coloring error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_explaino_poly_kind_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": -0.5,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 1,
    "explaino_warp_strength": 0,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected explaino state with non-custom poly_kind to fail\n";
            return 1;
        }
        if (error.find("poly_kind must be custom for fractal_type explaino") == std::string::npos) {
            std::cerr << "Unexpected explaino poly_kind error text: " << error << "\n";
            return 1;
        }
    }

      {
        const fs::path statePath = tempRoot / "bad_transcendental_func_type_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"transcendental_func\": 123");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "transcendental_func", &error)) {
          std::cerr << "Expected invalid transcendental_func type to fail: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "bad_transcendental_func_value_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"transcendental_func\": \"f_unknown\"");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "transcendental_func", &error)) {
          std::cerr << "Expected unknown transcendental_func to fail: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "bad_momentum_beta_type_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"momentum_beta\": \"fast\"");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "momentum_beta", &error)) {
          std::cerr << "Expected invalid momentum_beta type to fail: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "bad_mcmullen_preset_type_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"mcmullen_preset\": 7");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "mcmullen_preset", &error)) {
          std::cerr << "Expected invalid mcmullen_preset type to fail: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "bad_mcmullen_preset_value_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"mcmullen_preset\": \"z9_z9\"");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "mcmullen_preset", &error)) {
          std::cerr << "Expected unknown mcmullen_preset to fail: " << error << "\n";
          return 1;
        }
      }

    {
        const fs::path statePath = tempRoot / "legacy_state_version1_phoenix.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 1,
  "fractal_type": "phoenix",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.6,
    "poly_kind": 2,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        params.coloring_mode = ColoringMode::root_basin;
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected legacy phoenix state to load: " << error << "\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Expected legacy phoenix state to default to smooth_escape\n";
            return 1;
        }
        if (params.color_pipeline.signal != ColorSignal::smooth_escape) {
          std::cerr << "Expected legacy phoenix state to synthesize smooth_escape signal\n";
          return 1;
        }
        if (params.color_pipeline.palette != ColorPalette::cyclic_escape) {
          std::cerr << "Expected legacy phoenix state to synthesize cyclic_escape palette\n";
          return 1;
        }
        if (params.color_pipeline.grading != ColorGradingPreset::escape_default) {
          std::cerr << "Expected legacy phoenix state to synthesize escape_default grading\n";
          return 1;
        }
    }

      {
        const fs::path findingDir = tempRoot / "resolve_finding_json";
        fs::create_directories(findingDir);

        const fs::path statePath = findingDir / "state.json";
        std::ofstream stateFile(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        stateFile << "{}";
        stateFile.close();

        const fs::path findingPath = findingDir / "finding.json";
        std::ofstream findingFile(findingPath, std::ios::out | std::ios::binary | std::ios::trunc);
        findingFile << R"({
      "finding_id": "resolve_me",
      "state_file": "state.json"
    })";
        findingFile.close();

        std::string resolvedStatePath;
        std::string error;
        if (!ResolveFindingStateJsonPath(findingPath.string(), &resolvedStatePath, &error)) {
          std::cerr << "ResolveFindingStateJsonPath failed: " << error << "\n";
          return 1;
        }
        if (resolvedStatePath != statePath.string()) {
          std::cerr << "Resolved state path mismatch\n";
          return 1;
        }
      }

      {
        const fs::path findingDir = tempRoot / "missing_finding_state";
        fs::create_directories(findingDir);

        const fs::path findingPath = findingDir / "finding.json";
        std::ofstream findingFile(findingPath, std::ios::out | std::ios::binary | std::ios::trunc);
        findingFile << R"({
      "finding_id": "missing_state",
      "state_file": "missing.json"
    })";
        findingFile.close();

        std::string resolvedStatePath;
        std::string error;
        if (ResolveFindingStateJsonPath(findingPath.string(), &resolvedStatePath, &error)) {
          std::cerr << "Expected missing state file reference to fail\n";
          return 1;
        }
        if (error.find("Finding metadata points to missing state file") == std::string::npos) {
          std::cerr << "Unexpected missing-state error text: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path safeRoot = tempRoot / "finding_state_traversal";
        const fs::path findingDir = safeRoot / "finding_bundle";
        const fs::path outsideDir = safeRoot / "outside";
        fs::create_directories(findingDir);
        fs::create_directories(outsideDir);

        const fs::path outsideStatePath = outsideDir / "state.json";
        std::ofstream stateFile(outsideStatePath, std::ios::out | std::ios::binary | std::ios::trunc);
        stateFile << "{}";
        stateFile.close();

        const fs::path findingPath = findingDir / "finding.json";
        std::ofstream findingFile(findingPath, std::ios::out | std::ios::binary | std::ios::trunc);
        findingFile << R"({
      "finding_id": "escape_attempt",
      "state_file": "../outside/state.json"
    })";
        findingFile.close();

        std::string resolvedStatePath;
        std::string error;
        if (ResolveFindingStateJsonPath(findingPath.string(), &resolvedStatePath, &error)) {
          std::cerr << "Expected finding state path traversal to fail\n";
          return 1;
        }
        if (error.find("state_file") == std::string::npos) {
          std::cerr << "Unexpected traversal error text: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path findingDir = tempRoot / "finding_state_directory_ref";
        fs::create_directories(findingDir);

        const fs::path findingPath = findingDir / "finding.json";
        std::ofstream findingFile(findingPath, std::ios::out | std::ios::binary | std::ios::trunc);
        findingFile << R"({
      "finding_id": "directory_ref",
      "state_file": "."
    })";
        findingFile.close();

        std::string resolvedStatePath;
        std::string error;
        if (ResolveFindingStateJsonPath(findingPath.string(), &resolvedStatePath, &error)) {
          std::cerr << "Expected directory state_file reference to fail\n";
          return 1;
        }
        if (error.find("state_file") == std::string::npos) {
          std::cerr << "Unexpected directory-ref error text: " << error << "\n";
          return 1;
        }
      }

    // V3 round-trip: color grading fields
    {
        const fs::path statePath = tempRoot / "v3_grading.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_saturation": 1.35,
    "color_contrast": 1.22,
    "color_tint_r": 0.9,
    "color_tint_g": 1.1,
    "color_tint_b": 0.85
  },
  "render": { "width": 512, "height": 384, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 grading round-trip load failed: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(p.color_saturation, 1.35, 0.001)) { std::cerr << "color_saturation mismatch\n"; return 1; }
        if (!NearlyEqual(p.color_contrast, 1.22, 0.001)) { std::cerr << "color_contrast mismatch\n"; return 1; }
        if (!NearlyEqual(p.color_tint_r, 0.9, 0.001)) { std::cerr << "color_tint_r mismatch\n"; return 1; }
        if (!NearlyEqual(p.color_tint_g, 1.1, 0.001)) { std::cerr << "color_tint_g mismatch\n"; return 1; }
        if (!NearlyEqual(p.color_tint_b, 0.85, 0.001)) { std::cerr << "color_tint_b mismatch\n"; return 1; }
    }

    // V2 backward compat: grading fields should get defaults
    {
        const fs::path statePath = tempRoot / "v2_grading_defaults.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": { "width": 512, "height": 384, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V2 backward compat grading load failed: " << error << "\n";
            return 1;
        }
        if (r.interaction_debounce_ms != 200 || !NearlyEqual(r.preview_target_fps, 30.0f, 0.01) || !NearlyEqual(r.preview_min_scale, 0.5f, 0.01)) {
          std::cerr << "v2 render pacing controls should fall back to defaults when missing\n";
          return 1;
        }
        // Should get struct defaults when fields are missing
        if (!NearlyEqual(p.color_saturation, 1.15, 0.01)) { std::cerr << "v2 color_saturation should be default 1.15\n"; return 1; }
        if (!NearlyEqual(p.color_contrast, 1.10, 0.01)) { std::cerr << "v2 color_contrast should be default 1.10\n"; return 1; }
        if (!NearlyEqual(p.color_tint_r, 1.0, 0.01)) { std::cerr << "v2 color_tint_r should be default 1.0\n"; return 1; }
    }

    {
        const fs::path statePath = tempRoot / "v3_phase_bands_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "phase",
    "color_signal": "phase_angle",
    "color_shape": "repeat",
    "color_palette": "phase_wheel",
    "color_grading": "phase_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_phase_signal_offset": 1.25,
    "color_phase_wrap_cycles": 2.5,
    "color_phase_palette_offset": -0.75,
    "color_shape_offset": 0.3,
    "color_shape_scale": 1.5,
    "color_shape_repeat_frequency": 6.0,
    "color_shape_repeat_phase": 0.2,
    "color_iteration_band_count": 5,
    "color_iteration_band_softness": 0.8,
    "color_iteration_band_emphasis": 1.6,
    "color_iteration_band_palette_offset": 0.4,
    "color_smooth_escape_scale": 1.75,
    "color_smooth_escape_bias": -0.2,
    "color_heatmap_cycle_scale": 1.5,
    "color_heatmap_saturation": 1.25,
    "color_contrast_lift_exposure": 1.6,
    "color_contrast_lift_saturation": 1.3
  },
  "render": { "width": 512, "height": 384, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 phase/bands parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_shape != ColorPipelineShape::repeat ||
          !NearlyEqual(p.color_phase_signal_offset, 1.25, 0.001) ||
            !NearlyEqual(p.color_phase_wrap_cycles, 2.5, 0.001) ||
            !NearlyEqual(p.color_phase_palette_offset, -0.75, 0.001) ||
          !NearlyEqual(p.color_shape_offset, 0.3, 0.001) ||
          !NearlyEqual(p.color_shape_scale, 1.5, 0.001) ||
          !NearlyEqual(p.color_shape_repeat_frequency, 6.0, 0.001) ||
          !NearlyEqual(p.color_shape_repeat_phase, 0.2, 0.001) ||
            p.color_iteration_band_count != 5 ||
            !NearlyEqual(p.color_iteration_band_softness, 0.8, 0.001) ||
            !NearlyEqual(p.color_iteration_band_emphasis, 1.6, 0.001) ||
          !NearlyEqual(p.color_iteration_band_palette_offset, 0.4, 0.001) ||
          !NearlyEqual(p.color_smooth_escape_scale, 1.75, 0.001) ||
          !NearlyEqual(p.color_smooth_escape_bias, -0.2, 0.001) ||
          !NearlyEqual(p.color_heatmap_cycle_scale, 1.5, 0.001) ||
          !NearlyEqual(p.color_heatmap_saturation, 1.25, 0.001) ||
          !NearlyEqual(p.color_contrast_lift_exposure, 1.6, 0.001) ||
          !NearlyEqual(p.color_contrast_lift_saturation, 1.3, 0.001)) {
          std::cerr << "phase/bands/advanced color parameter fields mismatch\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_explaino_cmap_palette_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "identity",
    "color_palette": "explaino_cmap",
    "color_grading": "escape_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_explaino_palette_seed_scale": 1.5,
    "color_explaino_palette_seed_phase": 0.25,
    "color_explaino_palette_colorfulness": 0.8
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 explaino_cmap palette parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_pipeline.palette != ColorPalette::explaino_cmap ||
            !NearlyEqual(p.color_explaino_palette_seed_scale, 1.5, 0.001) ||
            !NearlyEqual(p.color_explaino_palette_seed_phase, 0.25, 0.001) ||
            !NearlyEqual(p.color_explaino_palette_colorfulness, 0.8, 0.001)) {
            std::cerr << "Expected explaino_cmap to round-trip through diagnostics state load with its dedicated owner fields\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_posterize_shape_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "posterize",
    "color_palette": "cyclic_escape",
    "color_grading": "escape_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_shape_posterize_steps": 5,
    "color_shape_posterize_mix": 0.65
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 posterize shape parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_shape != ColorPipelineShape::posterize ||
            p.color_shape_posterize_steps != 5 ||
            !NearlyEqual(p.color_shape_posterize_mix, 0.65, 0.001)) {
            std::cerr << "Expected posterize shape parameter fields to round-trip through diagnostics state load\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_mirror_repeat_shape_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "mirror_repeat",
    "color_palette": "cyclic_escape",
    "color_grading": "escape_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_shape_repeat_frequency": 4.0,
    "color_shape_repeat_phase": 0.15
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 mirror_repeat shape parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_shape != ColorPipelineShape::mirror_repeat ||
            !NearlyEqual(p.color_shape_repeat_frequency, 4.0, 0.001) ||
            !NearlyEqual(p.color_shape_repeat_phase, 0.15, 0.001)) {
            std::cerr << "Expected mirror_repeat to round-trip through diagnostics state load while reusing the repeat owner fields\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_bias_gain_curve_shape_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "bias_gain_curve",
    "color_palette": "cyclic_escape",
    "color_grading": "escape_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_shape_bias": 0.25,
    "color_shape_gain": 0.75
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 bias_gain_curve shape parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_shape != ColorPipelineShape::bias_gain_curve ||
            !NearlyEqual(p.color_shape_bias, 0.25, 0.001) ||
            !NearlyEqual(p.color_shape_gain, 0.75, 0.001)) {
            std::cerr << "Expected bias_gain_curve to round-trip through diagnostics state load with its dedicated owner fields\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_smooth_window_shape_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "smooth_window",
    "color_palette": "cyclic_escape",
    "color_grading": "escape_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_shape_window_center": 0.35,
    "color_shape_window_width": 0.4,
    "color_shape_window_softness": 0.05
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 smooth_window shape parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_shape != ColorPipelineShape::smooth_window ||
            !NearlyEqual(p.color_shape_window_center, 0.35, 0.001) ||
            !NearlyEqual(p.color_shape_window_width, 0.4, 0.001) ||
            !NearlyEqual(p.color_shape_window_softness, 0.05, 0.001)) {
            std::cerr << "Expected smooth_window to round-trip through diagnostics state load with its dedicated owner fields\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_shape_stack_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "phase",
    "color_signal": "phase_angle",
    "color_shape": "repeat",
    "color_palette": "phase_wheel",
    "color_grading": "phase_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_shape_stack": [
      { "shape": "offset_scale", "offset": 0.25, "scale": 1.5 },
      { "shape": "repeat", "repeat_frequency": 6.0, "repeat_phase": 0.2 }
    ]
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 shape-stack parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_shape_stack_count != 2 ||
            p.color_shape_stack[0].shape != ColorPipelineShape::offset_scale ||
            !NearlyEqual(p.color_shape_stack[0].params.offset, 0.25, 0.001) ||
            !NearlyEqual(p.color_shape_stack[0].params.scale, 1.5, 0.001) ||
            p.color_shape_stack[1].shape != ColorPipelineShape::repeat ||
            !NearlyEqual(p.color_shape_stack[1].params.repeat_frequency, 6.0, 0.001) ||
            !NearlyEqual(p.color_shape_stack[1].params.repeat_phase, 0.2, 0.001) ||
            p.color_shape != ColorPipelineShape::repeat ||
            !NearlyEqual(p.color_shape_repeat_frequency, 6.0, 0.001) ||
            !NearlyEqual(p.color_shape_repeat_phase, 0.2, 0.001)) {
            std::cerr << "Expected supported Shape stacks to round-trip through diagnostics state load with a legacy mirror of the final row\n";
            return 1;
        }
    }



    {
        const fs::path statePath = tempRoot / "v3_grading_stack_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "identity",
    "color_palette": "cyclic_escape",
    "color_grading": "escape_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_grading_stack": [
      { "grading": "escape_default", "exposure": 1.4, "saturation": 1.2 },
      { "grading": "phase_default", "saturation": 0.8, "contrast": 1.6 }
    ]
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 grading-stack parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_grading_stack_count != 2 ||
            p.color_grading_stack[0].grading != ColorGradingPreset::escape_default ||
            !NearlyEqual(p.color_grading_stack[0].params.exposure, 1.4, 0.001) ||
            !NearlyEqual(p.color_grading_stack[0].params.saturation, 1.2, 0.001) ||
            p.color_grading_stack[1].grading != ColorGradingPreset::phase_default ||
            !NearlyEqual(p.color_grading_stack[1].params.saturation, 0.8, 0.001) ||
            !NearlyEqual(p.color_grading_stack[1].params.contrast, 1.6, 0.001) ||
            !NearlyEqual(p.color_saturation, 0.8, 0.001) ||
            !NearlyEqual(p.color_contrast, 1.6, 0.001)) {
            std::cerr << "Expected supported Grading stacks to round-trip through diagnostics state load with a legacy mirror of the final row\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_palette_stack_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "identity",
    "color_palette": "explaino_cmap",
    "color_grading": "escape_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_palette_stack": [
      { "palette": "cyclic_escape", "cycle_scale": 1.25, "saturation": 0.9, "blend_weight": 1.0, "blend_mode": "normal" },
      { "palette": "explaino_cmap", "seed_scale": 1.5, "seed_phase": 0.25, "colorfulness": 0.8, "blend_weight": 0.35, "blend_mode": "normal" }
    ]
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 palette-stack parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_palette_stack_count != 2 ||
            p.color_palette_stack[0].palette != ColorPalette::cyclic_escape ||
            !NearlyEqual(p.color_palette_stack[0].params.cycle_scale, 1.25, 0.001) ||
            !NearlyEqual(p.color_palette_stack[0].params.saturation, 0.9, 0.001) ||
            p.color_palette_stack[1].palette != ColorPalette::explaino_cmap ||
            !NearlyEqual(p.color_palette_stack[1].params.seed_scale, 1.5, 0.001) ||
            !NearlyEqual(p.color_palette_stack[1].params.seed_phase, 0.25, 0.001) ||
            !NearlyEqual(p.color_palette_stack[1].params.colorfulness, 0.8, 0.001) ||
            !NearlyEqual(p.color_palette_stack[1].params.blend_weight, 0.35, 0.001) ||
            p.color_palette_stack[1].params.blend_mode != ColorPaletteBlendMode::normal ||
            p.color_pipeline.palette != ColorPalette::explaino_cmap ||
            !NearlyEqual(p.color_explaino_palette_seed_scale, 1.5, 0.001) ||
            !NearlyEqual(p.color_explaino_palette_seed_phase, 0.25, 0.001) ||
            !NearlyEqual(p.color_explaino_palette_colorfulness, 0.8, 0.001)) {
            std::cerr << "Expected supported Palette stacks to round-trip through diagnostics state load with blend params and a legacy mirror of the final row\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_source_stack_params.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "escape_magnitude",
    "color_shape": "identity",
    "color_palette": "cyclic_escape",
    "color_grading": "escape_default",
    "color_source_stack": [
      { "signal": "smooth_escape", "scale": 0.5, "bias": 0.25, "blend_weight": 1.0 },
      { "signal": "escape_magnitude", "magnitude_scale": 1.5, "magnitude_bias": -0.25, "blend_weight": 0.25 }
    ],
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        ColorPipelineWindowState windowState{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &windowState, &error)) {
            std::cerr << "V3 source-stack parameter load failed: " << error << "\n";
            return 1;
        }
        if (p.color_source_stack_count != 2 ||
            p.color_source_stack[0].signal != ColorSignal::smooth_escape ||
            !NearlyEqual(p.color_source_stack[0].params.scale, 0.5, 0.001) ||
            !NearlyEqual(p.color_source_stack[0].params.bias, 0.25, 0.001) ||
            p.color_source_stack[1].signal != ColorSignal::escape_magnitude ||
            !NearlyEqual(p.color_source_stack[1].params.magnitude_scale, 1.5, 0.001) ||
            !NearlyEqual(p.color_source_stack[1].params.magnitude_bias, -0.25, 0.001) ||
            !NearlyEqual(p.color_source_stack[1].params.blend_weight, 0.25, 0.001) ||
            p.color_pipeline.signal != ColorSignal::escape_magnitude ||
            !NearlyEqual(p.color_escape_magnitude_scale, 1.5, 0.001) ||
            !NearlyEqual(p.color_escape_magnitude_bias, -0.25, 0.001) ||
            !windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() < 4 ||
            windowState.live_snapshot.lanes[0].rows.size() != 2 ||
            windowState.live_snapshot.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            windowState.live_snapshot.lanes[0].rows[1].function_id != "escape_magnitude") {
            std::cerr << "Expected supported Source stacks to round-trip through diagnostics state load with a live two-row Source lane and a legacy mirror of the final row\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_source_stack_root_index_reject.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "mandelbrot",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "color_signal": "smooth_escape",
    "color_shape": "identity",
    "color_palette": "cyclic_escape",
    "color_grading": "escape_default",
    "color_source_stack": [
      { "signal": "root_index", "blend_weight": 1.0 }
    ],
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error) ||
            error.find("root_index") == std::string::npos) {
            std::cerr << "Expected diagnostics state load to reject root_index inside generic Source composition\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_root_basin_pair_schedule.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "joy_basins",
    "color_signal": "root_index",
    "color_shape": "identity",
    "color_palette": "joy",
    "color_grading": "basin_default",
    "color_root_basin_pairs": [
      { "signal": "root_index", "palette": "root_classic", "grading": "basin_default" },
      { "signal": "root_index", "palette": "joy", "grading": "basin_default" }
    ],
    "color_grading_stack": [
      { "grading": "basin_default" }
    ],
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": { "width": 320, "height": 240, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "Root-basin pair schedule load failed: " << error << "\n";
            return 1;
        }
        if (p.color_root_basin_pair_count != 2 ||
            p.color_root_basin_pairs[0].signal != ColorSignal::root_index ||
            p.color_root_basin_pairs[0].palette != ColorPalette::root_classic ||
            p.color_root_basin_pairs[1].signal != ColorSignal::root_index ||
            p.color_root_basin_pairs[1].palette != ColorPalette::joy ||
            p.color_pipeline.grading != ColorGradingPreset::basin_default ||
            p.color_grading_stack_count != 1 ||
            p.color_grading_stack[0].grading != ColorGradingPreset::basin_default ||
            p.color_pipeline.signal != ColorSignal::root_index ||
            p.color_pipeline.palette != ColorPalette::joy ||
            p.coloring_mode != ColoringMode::joy_basins) {
            std::cerr << "Expected bounded root-basin pair schedules to round-trip through diagnostics state load while preserving the mirrored final live tuple and basin-default grading lane\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "v3_advanced_color_draft.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "phase",
    "color_signal": "phase_angle",
    "color_shape": "repeat",
    "color_palette": "phase_wheel",
    "color_grading": "phase_default",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": { "width": 512, "height": 384, "block_size": 256, "device_id": 0 },
  "color_pipeline_draft": {
    "next_row_id": 4,
    "lanes": [
      {
        "lane_id": "source",
        "label": "Source",
        "rows": [
          {
            "ui_row_id": 1,
            "enabled": true,
            "function_id": "phase_orbit",
            "parameter_values": [
              { "path": "signal.phase_offset", "type": "float", "number_value": 1.25 },
              { "path": "signal.wrap_cycles", "type": "float", "number_value": 2.5 }
            ]
          }
        ]
      },
      {
        "lane_id": "shape",
        "label": "Shape",
        "rows": [
          {
            "ui_row_id": 2,
            "enabled": true,
            "function_id": "repeat",
            "parameter_values": [
              { "path": "shape.frequency", "type": "float", "number_value": 6.0 },
              { "path": "shape.phase", "type": "float", "number_value": 0.2 }
            ]
          }
        ]
      },
      {
        "lane_id": "palette",
        "label": "Palette",
        "rows": [
          {
            "ui_row_id": 3,
            "enabled": true,
            "function_id": "phase_wheel_palette",
            "parameter_values": [
              { "path": "palette.phase_offset", "type": "float", "number_value": -0.75 },
              { "path": "palette.saturation", "type": "float", "number_value": 1.15 }
            ]
          }
        ]
      }
    ]
  }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        ColorPipelineWindowState draft{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &draft, &error)) {
            std::cerr << "V3 advanced color draft load failed: " << error << "\n";
            return 1;
        }
        if (draft.next_row_id != 5 || draft.lanes.size() != 4) {
          std::cerr << "Expected advanced color draft load to upgrade legacy three-lane drafts with the shipped Grading lane and next_row_id\n";
            return 1;
        }
        if (draft.lanes[1].rows.size() != 1 ||
            draft.lanes[1].rows[0].function_id != "repeat" ||
            !DraftRowHasNumberParam(draft.lanes[1].rows[0], "shape.frequency", 6.0) ||
            !DraftRowHasNumberParam(draft.lanes[1].rows[0], "shape.phase", 0.2)) {
            std::cerr << "Expected advanced color draft load to restore the programmable Shape repeat row and its params\n";
            return 1;
        }
        if (draft.lanes[3].lane_id != "grading" ||
          draft.lanes[3].rows.size() != 1 ||
          draft.lanes[3].rows[0].function_id != "contrast_lift" ||
          draft.lanes[3].rows[0].ui_row_id != 4 ||
          !DraftRowHasNumberParam(draft.lanes[3].rows[0], "grade.exposure", 1.0) ||
          !DraftRowHasNumberParam(draft.lanes[3].rows[0], "grade.saturation", 1.0)) {
          std::cerr << "Expected advanced color draft load to seed the bounded contrast_lift grading row when upgrading a legacy three-lane draft\n";
          return 1;
        }
    }


    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        SidecarOrientationVector orientation{};
        bool hasOrientation = false;
        SidecarAutoDemoControllerPolicy controllerPolicy{};
        bool hasControllerPolicy = false;
        SidecarAutoDemoMutationHistory mutationHistory;
        bool hasMutationHistory = false;
        ColorPipelineWindowState draft{};
        std::string error;
        if (!LoadDiagnosticsStateJson(
                ManualExplainoJoyCaptureStateJsonLowRes(),
                &view,
                &params,
                &render,
                &orientation,
                &hasOrientation,
                &controllerPolicy,
                &hasControllerPolicy,
                &mutationHistory,
                &hasMutationHistory,
                &draft,
                &error)) {
            std::cerr << "Manual explaino_joy capture fixture failed to load: " << error << "\n";
            return 1;
        }
        if (!ExpectManualExplainoJoyCaptureState(
                view,
                params,
                render,
                orientation,
                hasOrientation,
                controllerPolicy,
                hasControllerPolicy,
                hasMutationHistory,
                draft,
                "manual fixture load")) {
            return 1;
        }

        const double preciseCenterHpX = -1.840130123456789;
        const double preciseLog2Zoom = 1.375040123456789;
        const double preciseSmoothScale = 1.0103987654321;
        const double preciseGradeExposure = 1.1903987654321;
        view.center_hp_x = preciseCenterHpX;
        view.log2_zoom = preciseLog2Zoom;
        params.color_smooth_escape_scale = static_cast<float>(preciseSmoothScale);
        params.color_grading_stack[0].params.exposure = static_cast<float>(preciseGradeExposure);
        params.color_contrast_lift_exposure = static_cast<float>(preciseGradeExposure);
        params.color_palette_stack_count = 2;
        params.color_palette_stack[0].palette = ColorPalette::cyclic_escape;
        params.color_palette_stack[0].params.cycle_scale = 1.25f;
        params.color_palette_stack[0].params.saturation = 0.9f;
        params.color_palette_stack[1].palette = ColorPalette::explaino_cmap;
        params.color_palette_stack[1].params.seed_scale = 1.5f;
        params.color_palette_stack[1].params.seed_phase = 0.25f;
        params.color_palette_stack[1].params.colorfulness = 0.8f;
        params.color_palette_stack[1].params.blend_weight = 0.35f;
        params.color_pipeline.palette = ColorPalette::explaino_cmap;
        params.explaino_damping = 0.73f;
        params.explaino_root_count = 4;
        params.explaino_roots[0] = {0.125f, -0.25f};
        params.explaino_roots[1] = {0.5f, -0.75f};
        params.explaino_roots[2] = {-1.25f, 1.5f};
        params.explaino_roots[3] = {1.75f, 2.0f};
        params.poly_coeffs_b[0] = 9.0f;
        params.poly_coeffs_b[1] = 8.0f;
        params.poly_coeffs_b[2] = 7.0f;
        params.poly_coeffs_b[3] = 6.0f;
        params.poly_coeffs_b[4] = 5.0f;
        params.color_explaino_palette_seed_scale = 1.5f;
        params.color_explaino_palette_seed_phase = 0.25f;
        params.color_explaino_palette_colorfulness = 0.8f;
        if (!SetDraftRowNumberParam(&draft.lanes[0].rows[0], "signal.scale", preciseSmoothScale) ||
            !SetDraftRowNumberParam(&draft.lanes[3].rows[0], "grade.exposure", preciseGradeExposure)) {
            std::cerr << "Expected manual capture draft precision sentinels to be editable before save\n";
            return 1;
        }

        const fs::path runtimeDir = tempRoot / "manual_explaino_joy_capture_roundtrip_runtime";
        fs::remove_all(runtimeDir);
        fs::create_directories(runtimeDir);

        RenderStats stats{};
        stats.last_iters_avg = 9;
        std::vector<std::uint32_t> rgba(
            static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y),
            0xff224466u);
        DiagnosticsCaptureResult capture{};
        const SidecarAutoDemoMutationHistory* persistedHistory = hasMutationHistory ? &mutationHistory : nullptr;
        if (!CaptureDiagnosticsLastBundle(
                runtimeDir.string(),
                view,
                params,
                render,
                stats,
                rgba.data(),
                rgba.size(),
                &orientation,
                &controllerPolicy,
                persistedHistory,
                &draft,
                &capture,
                &error)) {
            std::cerr << "Manual explaino_joy capture fixture failed to serialize: " << error << "\n";
            return 1;
        }

        std::string serializedState;
        if (!ReadTextFile(capture.state_json_path, &serializedState)) {
            std::cerr << "Expected capture-backed serialization regression to write state.json\n";
            return 1;
        }
        if (serializedState.find("\"width\": 64") == std::string::npos ||
            serializedState.find("\"height\": 64") == std::string::npos ||
            serializedState.find("\"color_pipeline_draft\"") == std::string::npos ||
            serializedState.find("\"color_palette_stack\"") == std::string::npos ||
            serializedState.find("\"explaino_damping\": 0.73000001907348633") == std::string::npos ||
            serializedState.find("\"explaino_roots\"") == std::string::npos ||
            serializedState.find("\"poly_coeffs_b\"") == std::string::npos ||
            serializedState.find("\"blend_weight\": 0.34999999403953552") == std::string::npos ||
            serializedState.find("\"blend_mode\": \"normal\"") == std::string::npos ||
            serializedState.find("\"color_grading_stack\"") == std::string::npos ||
            serializedState.find("\"import_signature\": \"9475387712945145731\"") == std::string::npos) {
            std::cerr << "Expected capture-backed serialization regression to emit low-resolution render settings, ExplainO damping, draft state, grading stack, and 64-bit sidecar hashes\n";
            return 1;
        }

        ViewState roundTripView{};
        KernelParams roundTripParams{};
        RenderSettings roundTripRender{};
        SidecarOrientationVector roundTripOrientation{};
        bool roundTripHasOrientation = false;
        SidecarAutoDemoControllerPolicy roundTripControllerPolicy{};
        bool roundTripHasControllerPolicy = false;
        SidecarAutoDemoMutationHistory roundTripMutationHistory;
        bool roundTripHasMutationHistory = false;
        ColorPipelineWindowState roundTripDraft{};
        if (!LoadDiagnosticsStateJson(
                serializedState,
                &roundTripView,
                &roundTripParams,
                &roundTripRender,
                &roundTripOrientation,
                &roundTripHasOrientation,
                &roundTripControllerPolicy,
                &roundTripHasControllerPolicy,
                &roundTripMutationHistory,
                &roundTripHasMutationHistory,
                &roundTripDraft,
                &error)) {
            std::cerr << "Manual explaino_joy emitted capture state failed to reload: " << error << "\n";
            return 1;
        }
        if (!ExpectManualExplainoJoyCaptureState(
                roundTripView,
                roundTripParams,
                roundTripRender,
                roundTripOrientation,
                roundTripHasOrientation,
                roundTripControllerPolicy,
                roundTripHasControllerPolicy,
                roundTripHasMutationHistory,
                roundTripDraft,
                "manual fixture capture reload",
                preciseCenterHpX,
                preciseLog2Zoom,
                preciseSmoothScale,
                preciseGradeExposure,
                1.5,
                0.8)) {
            return 1;
        }
        if (!NearlyEqual(roundTripParams.explaino_damping, 0.73f, 1.0e-6)) {
            std::cerr << "Expected capture-backed serialization regression to reload ExplainO damping from emitted state.json\n";
            return 1;
        }
        if (roundTripParams.explaino_root_count != 4 ||
            !NearlyEqual(roundTripParams.explaino_roots[0].x, 0.125f, 1.0e-6) ||
            !NearlyEqual(roundTripParams.explaino_roots[0].y, -0.25f, 1.0e-6) ||
            !NearlyEqual(roundTripParams.explaino_roots[3].x, 1.75f, 1.0e-6) ||
            !NearlyEqual(roundTripParams.explaino_roots[3].y, 2.0f, 1.0e-6)) {
            std::cerr << "Expected capture-backed serialization regression to reload ExplainO roots from emitted state.json\n";
            return 1;
        }
        if (!NearlyEqual(roundTripParams.poly_coeffs_b[0], 9.0f, 1.0e-6) ||
            !NearlyEqual(roundTripParams.poly_coeffs_b[4], 5.0f, 1.0e-6)) {
            std::cerr << "Expected capture-backed serialization regression to reload secondary ExplainO polynomial coefficients from emitted state.json\n";
            return 1;
        }
        if (roundTripParams.color_palette_stack_count != 2 ||
            roundTripParams.color_palette_stack[0].palette != ColorPalette::cyclic_escape ||
            roundTripParams.color_palette_stack[1].palette != ColorPalette::explaino_cmap ||
            !NearlyEqual(roundTripParams.color_palette_stack[1].params.blend_weight, 0.35f, 1.0e-6) ||
            roundTripParams.color_palette_stack[1].params.blend_mode != ColorPaletteBlendMode::normal) {
            std::cerr << "Expected capture-backed serialization regression to reload Palette stack blend params from emitted state.json\n";
            return 1;
        }
    }

      {
        ViewState matrixView{};
        KernelParams matrixParams{};
        RenderSettings matrixRender{};
        ColorPipelineWindowState baseDraft{};
        std::string error;
        if (!LoadDiagnosticsStateJson(
            ManualExplainoJoyCaptureStateJsonLowRes(),
            &matrixView,
            &matrixParams,
            &matrixRender,
            &baseDraft,
            &error)) {
          std::cerr << "Expected advanced color matrix baseline fixture to load: " << error << "\n";
          return 1;
        }

        matrixRender.resolution = {1, 1};
        matrixRender.block_size = 1;
        matrixRender.device_id = 0;

        const fs::path runtimeDir = tempRoot / "advanced_color_function_matrix_runtime";
        fs::remove_all(runtimeDir);
        fs::create_directories(runtimeDir);

        RenderStats matrixStats{};
        std::vector<std::uint32_t> rgba(1, 0xff224466u);
        const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
        const std::size_t sourceLaneIndex = FindDraftLaneIndex(baseDraft, "source");
        const std::size_t shapeLaneIndex = FindDraftLaneIndex(baseDraft, "shape");
        const std::size_t paletteLaneIndex = FindDraftLaneIndex(baseDraft, "palette");
        const std::size_t gradingLaneIndex = FindDraftLaneIndex(baseDraft, "grading");
        if (sourceLaneIndex >= baseDraft.lanes.size() ||
          shapeLaneIndex >= baseDraft.lanes.size() ||
          paletteLaneIndex >= baseDraft.lanes.size() ||
          gradingLaneIndex >= baseDraft.lanes.size()) {
          std::cerr << "Expected advanced color matrix baseline draft to expose all four programmable lanes\n";
          return 1;
        }

        const ColorPipelineLaneCatalog& sourceCatalog = catalogs[sourceLaneIndex];
        const ColorPipelineLaneCatalog& shapeCatalog = catalogs[shapeLaneIndex];
        const ColorPipelineLaneCatalog& paletteCatalog = catalogs[paletteLaneIndex];
        const ColorPipelineLaneCatalog& gradingCatalog = catalogs[gradingLaneIndex];

        std::set<std::string> coveredSourceFunctions;
        std::set<std::string> coveredShapeFunctions;
        std::set<std::string> coveredPaletteFunctions;
        std::set<std::string> coveredGradingFunctions;
        int supportedTupleCount = 0;

        for (const FunctionDescriptor& sourceDescriptor : sourceCatalog.functions) {
          for (const FunctionDescriptor& shapeDescriptor : shapeCatalog.functions) {
            for (const FunctionDescriptor& paletteDescriptor : paletteCatalog.functions) {
              for (const FunctionDescriptor& gradingDescriptor : gradingCatalog.functions) {
                ColorPipelineWindowState candidateDraft = baseDraft;
                if (!SetColorPipelineRowFunction(&candidateDraft.lanes[sourceLaneIndex].rows[0], sourceDescriptor) ||
                  !SetColorPipelineRowFunction(&candidateDraft.lanes[shapeLaneIndex].rows[0], shapeDescriptor) ||
                  !SetColorPipelineRowFunction(&candidateDraft.lanes[paletteLaneIndex].rows[0], paletteDescriptor) ||
                  !SetColorPipelineRowFunction(&candidateDraft.lanes[gradingLaneIndex].rows[0], gradingDescriptor)) {
                  std::cerr << "Expected advanced color matrix draft row selection to succeed for supported runtime-backed functions\n";
                  return 1;
                }

                error.clear();
                if (!ConfigureDistinctDraftRowParams(sourceDescriptor, &candidateDraft.lanes[sourceLaneIndex].rows[0], &error) ||
                  !ConfigureDistinctDraftRowParams(shapeDescriptor, &candidateDraft.lanes[shapeLaneIndex].rows[0], &error) ||
                  !ConfigureDistinctDraftRowParams(paletteDescriptor, &candidateDraft.lanes[paletteLaneIndex].rows[0], &error) ||
                  !ConfigureDistinctDraftRowParams(gradingDescriptor, &candidateDraft.lanes[gradingLaneIndex].rows[0], &error)) {
                  std::cerr << "Expected advanced color matrix draft params to be configurable: " << error << "\n";
                  return 1;
                }

                const ColorPipelineDraftApplyState applyState =
                  DescribeColorPipelineDraftApplyState(candidateDraft, matrixView.fractal_type, &matrixParams);
                if (applyState.status == ColorPipelineDraftApplyStatus::unsupported_tuple ||
                  applyState.status == ColorPipelineDraftApplyStatus::disallowed_for_family) {
                  continue;
                }
                if (applyState.status != ColorPipelineDraftApplyStatus::can_apply &&
                  applyState.status != ColorPipelineDraftApplyStatus::matches_live) {
                  std::cerr << "Expected advanced color matrix tuple to be valid or explicitly unsupported, but got status "
                        << static_cast<int>(applyState.status) << " for tuple "
                        << sourceDescriptor.id << " / "
                        << shapeDescriptor.id << " / "
                        << paletteDescriptor.id << " / "
                        << gradingDescriptor.id << ": "
                        << applyState.message << "\n";
                  return 1;
                }

                KernelParams appliedParams = matrixParams;
                ColorPipelineWindowState appliedDraft = candidateDraft;
                bool changed = false;
                error.clear();
                if (!ApplyColorPipelineDraftToLiveState(&appliedDraft, matrixView.fractal_type, &appliedParams, &changed)) {
                  std::cerr << "Expected advanced color matrix tuple to apply: "
                        << sourceDescriptor.id << " / "
                        << shapeDescriptor.id << " / "
                        << paletteDescriptor.id << " / "
                        << gradingDescriptor.id << "\n";
                  return 1;
                }

                DiagnosticsCaptureResult capture{};
                if (!CaptureDiagnosticsLastBundle(
                    runtimeDir.string(),
                    matrixView,
                    appliedParams,
                    matrixRender,
                    matrixStats,
                    rgba.data(),
                    rgba.size(),
                    &appliedDraft,
                    &capture,
                    &error)) {
                  std::cerr << "Expected advanced color matrix tuple to serialize: " << error << "\n";
                  return 1;
                }

                std::string stateJson;
                if (!ReadTextFile(capture.state_json_path, &stateJson)) {
                  std::cerr << "Expected advanced color matrix tuple to write diagnostics state.json\n";
                  return 1;
                }

                ViewState loadedView{};
                KernelParams loadedParams{};
                RenderSettings loadedRender{};
                ColorPipelineWindowState loadedDraft{};
                error.clear();
                if (!LoadDiagnosticsStateJson(stateJson, &loadedView, &loadedParams, &loadedRender, &loadedDraft, &error)) {
                  std::cerr << "Expected advanced color matrix tuple to reload from diagnostics state.json: "
                        << sourceDescriptor.id << " / "
                        << shapeDescriptor.id << " / "
                        << paletteDescriptor.id << " / "
                        << gradingDescriptor.id << " => " << error << "\n";
                  return 1;
                }

                ColorPipelineWindowState normalizedExpectedDraft = appliedDraft;
                ColorPipelineWindowState normalizedLoadedDraft = loadedDraft;
                error.clear();
                if (!NormalizeDraftForLaneComparison(&normalizedExpectedDraft, &error) ||
                  !NormalizeDraftForLaneComparison(&normalizedLoadedDraft, &error)) {
                  std::cerr << "Expected advanced color matrix drafts to normalize for comparison: " << error << "\n";
                  return 1;
                }
                for (std::size_t laneIndex = 0; laneIndex < normalizedExpectedDraft.lanes.size(); ++laneIndex) {
                  if (!ColorPipelineLaneStatesEqual(normalizedExpectedDraft.lanes[laneIndex], normalizedLoadedDraft.lanes[laneIndex])) {
                    std::cerr << "Expected diagnostics state round-trip to preserve advanced color lane state for tuple "
                          << sourceDescriptor.id << " / "
                          << shapeDescriptor.id << " / "
                          << paletteDescriptor.id << " / "
                          << gradingDescriptor.id << "\n";
                    return 1;
                  }
                }

                ColorPipelineLiveSnapshot expectedSnapshot{};
                error.clear();
                if (!TryBuildColorPipelineLiveSnapshot(matrixView.fractal_type, appliedParams, &expectedSnapshot, &error)) {
                  std::cerr << "Expected applied advanced color matrix tuple to produce a live snapshot: " << error << "\n";
                  return 1;
                }

                ColorPipelineLiveSnapshot loadedSnapshot{};
                error.clear();
                if (!TryBuildColorPipelineLiveSnapshot(loadedView.fractal_type, loadedParams, &loadedSnapshot, &error)) {
                  std::cerr << "Expected reloaded advanced color matrix tuple to produce a live snapshot: " << error << "\n";
                  return 1;
                }

                if (!ColorPipelineLiveSnapshotsEqual(expectedSnapshot, loadedSnapshot)) {
                  std::cerr << "Expected reloaded advanced color matrix tuple to preserve the normalized live runtime snapshot for tuple "
                        << sourceDescriptor.id << " / "
                        << shapeDescriptor.id << " / "
                        << paletteDescriptor.id << " / "
                        << gradingDescriptor.id << "\n";
                  return 1;
                }

                coveredSourceFunctions.insert(sourceDescriptor.id);
                coveredShapeFunctions.insert(shapeDescriptor.id);
                coveredPaletteFunctions.insert(paletteDescriptor.id);
                coveredGradingFunctions.insert(gradingDescriptor.id);
                ++supportedTupleCount;
              }
            }
          }
        }

        if (supportedTupleCount <= 0) {
          std::cerr << "Expected advanced color diagnostics matrix to find at least one supported runtime-backed tuple\n";
          return 1;
        }
        if (coveredSourceFunctions.size() != sourceCatalog.functions.size()) {
          std::cerr << "Expected advanced color diagnostics matrix to cover every runtime-backed Source function\n";
          return 1;
        }
        if (coveredShapeFunctions.size() != shapeCatalog.functions.size()) {
          std::cerr << "Expected advanced color diagnostics matrix to cover every runtime-backed Shape function\n";
          return 1;
        }
        if (coveredPaletteFunctions.size() != paletteCatalog.functions.size()) {
          std::cerr << "Expected advanced color diagnostics matrix to cover every runtime-backed Palette function\n";
          return 1;
        }
        if (coveredGradingFunctions.size() != gradingCatalog.functions.size()) {
          std::cerr << "Expected advanced color diagnostics matrix to cover every runtime-backed Grading function\n";
          return 1;
        }
      }

      {
        ViewState stackView{};
        KernelParams stackParams{};
        RenderSettings stackRender{};
        ColorPipelineWindowState baseDraft{};
        std::string error;
        if (!LoadDiagnosticsStateJson(
            ManualExplainoJoyCaptureStateJsonLowRes(),
            &stackView,
            &stackParams,
            &stackRender,
            &baseDraft,
            &error)) {
          std::cerr << "Expected advanced color stack baseline fixture to load: " << error << "\n";
          return 1;
        }

        stackRender.resolution = {1, 1};
        stackRender.block_size = 1;
        stackRender.device_id = 0;

        const fs::path runtimeDir = tempRoot / "advanced_color_stack_matrix_runtime";
        fs::remove_all(runtimeDir);
        fs::create_directories(runtimeDir);

        RenderStats stackStats{};
        std::vector<std::uint32_t> rgba(1, 0xff557799u);
        const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
        const std::size_t sourceLaneIndex = FindDraftLaneIndex(baseDraft, "source");
        const std::size_t shapeLaneIndex = FindDraftLaneIndex(baseDraft, "shape");
        const std::size_t paletteLaneIndex = FindDraftLaneIndex(baseDraft, "palette");
        const std::size_t gradingLaneIndex = FindDraftLaneIndex(baseDraft, "grading");
        if (sourceLaneIndex >= baseDraft.lanes.size() ||
            shapeLaneIndex >= baseDraft.lanes.size() ||
            paletteLaneIndex >= baseDraft.lanes.size() ||
            gradingLaneIndex >= baseDraft.lanes.size()) {
          std::cerr << "Expected advanced color stack baseline draft to expose all four programmable lanes\n";
          return 1;
        }

        const ColorPipelineLaneCatalog& sourceCatalog = catalogs[sourceLaneIndex];
        const ColorPipelineLaneCatalog& shapeCatalog = catalogs[shapeLaneIndex];
        const ColorPipelineLaneCatalog& paletteCatalog = catalogs[paletteLaneIndex];
        const ColorPipelineLaneCatalog& gradingCatalog = catalogs[gradingLaneIndex];
        const FunctionDescriptor* identityDescriptor = FindColorPipelineFunctionDescriptor(shapeCatalog, "identity");
        const FunctionDescriptor* basePaletteDescriptor = FindColorPipelineFunctionDescriptor(paletteCatalog, "heatmap");
        const FunctionDescriptor* baseSourceDescriptor = FindColorPipelineFunctionDescriptor(sourceCatalog, "smooth_escape_ramp");
        const FunctionDescriptor* baseGradingDescriptor = FindColorPipelineFunctionDescriptor(gradingCatalog, "contrast_lift");
        if (!identityDescriptor || !basePaletteDescriptor || !baseSourceDescriptor || !baseGradingDescriptor) {
          std::cerr << "Expected advanced color stack matrix baseline descriptors to exist\n";
          return 1;
        }

        std::set<std::string> coveredPaletteStackFunctions;
        for (const FunctionDescriptor& paletteDescriptor : paletteCatalog.functions) {
          if (!IsSupportedColorPipelinePaletteStackFunctionId(paletteDescriptor.id)) {
            continue;
          }

          const FunctionDescriptor* supportedSourceDescriptor = nullptr;
          const FunctionDescriptor* supportedGradingDescriptor = nullptr;
          error.clear();
          if (!ResolveSupportedSourceAndGradingForPaletteFunction(
                  sourceCatalog,
                  paletteDescriptor,
                  gradingCatalog,
                  &supportedSourceDescriptor,
                  &supportedGradingDescriptor,
                  &error)) {
            std::cerr << "Expected advanced color Palette-stack matrix to resolve a supporting tuple: " << error << "\n";
            return 1;
          }

          ColorPipelineWindowState candidateDraft = baseDraft;
          if (!SetColorPipelineRowFunction(&candidateDraft.lanes[sourceLaneIndex].rows[0], *supportedSourceDescriptor) ||
              !SetColorPipelineRowFunction(&candidateDraft.lanes[shapeLaneIndex].rows[0], *identityDescriptor) ||
              !SetColorPipelineRowFunction(&candidateDraft.lanes[paletteLaneIndex].rows[0], paletteDescriptor) ||
              !SetColorPipelineRowFunction(&candidateDraft.lanes[gradingLaneIndex].rows[0], *supportedGradingDescriptor) ||
              !AppendDraftRowForFunction(&candidateDraft, paletteCatalog, paletteLaneIndex, paletteDescriptor, 100, &error)) {
            std::cerr << "Expected advanced color Palette-stack matrix draft setup to succeed: " << error << "\n";
            return 1;
          }

          if (!ConfigureDistinctDraftRowParams(*supportedSourceDescriptor, &candidateDraft.lanes[sourceLaneIndex].rows[0], 0, &error) ||
              !ConfigureDistinctDraftRowParams(*identityDescriptor, &candidateDraft.lanes[shapeLaneIndex].rows[0], 0, &error) ||
              !ConfigureDistinctDraftRowParams(paletteDescriptor, &candidateDraft.lanes[paletteLaneIndex].rows[0], 0, &error) ||
              !ConfigureDistinctDraftRowParams(*supportedGradingDescriptor, &candidateDraft.lanes[gradingLaneIndex].rows[0], 0, &error) ||
              !ConfigureDistinctDraftRowParams(paletteDescriptor, &candidateDraft.lanes[paletteLaneIndex].rows[1], 1, &error)) {
            std::cerr << "Expected advanced color Palette-stack matrix params to be configurable: " << error << "\n";
            return 1;
          }

          const ColorPipelineDraftApplyState applyState =
              DescribeColorPipelineDraftApplyState(candidateDraft, stackView.fractal_type, &stackParams);
          if (applyState.status != ColorPipelineDraftApplyStatus::can_apply &&
              applyState.status != ColorPipelineDraftApplyStatus::matches_live) {
            std::cerr << "Expected advanced color Palette-stack matrix tuple to apply for second-row function "
                      << paletteDescriptor.id << ": " << applyState.message << "\n";
            return 1;
          }

          ColorPipelinePaletteStackEntry expectedSecondEntry{};
          if (!TryBuildColorPipelinePaletteStackEntryFromRow(candidateDraft.lanes[paletteLaneIndex].rows[1], &expectedSecondEntry, &error)) {
            std::cerr << "Expected advanced color Palette-stack matrix to build the second row entry: " << error << "\n";
            return 1;
          }

          KernelParams appliedParams = stackParams;
          ColorPipelineWindowState appliedDraft = candidateDraft;
          bool changed = false;
          if (!ApplyColorPipelineDraftToLiveState(&appliedDraft, stackView.fractal_type, &appliedParams, &changed)) {
            std::cerr << "Expected advanced color Palette-stack matrix tuple to apply for second-row function "
                      << paletteDescriptor.id << "\n";
            return 1;
          }
          if (appliedParams.color_palette_stack_count != 2 ||
              !ColorPipelinePaletteStackEntriesEqual(appliedParams.color_palette_stack[1], expectedSecondEntry)) {
            std::cerr << "Expected advanced color Palette-stack matrix to preserve the second Palette row in live runtime state for function "
                      << paletteDescriptor.id << "\n";
            return 1;
          }

          DiagnosticsCaptureResult capture{};
          error.clear();
          if (!CaptureDiagnosticsLastBundle(
                  runtimeDir.string(),
                  stackView,
                  appliedParams,
                  stackRender,
                  stackStats,
                  rgba.data(),
                  rgba.size(),
                  &appliedDraft,
                  &capture,
                  &error)) {
            std::cerr << "Expected advanced color Palette-stack matrix tuple to serialize: " << error << "\n";
            return 1;
          }

          std::string stateJson;
          if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected advanced color Palette-stack matrix tuple to write diagnostics state.json\n";
            return 1;
          }

          ViewState loadedView{};
          KernelParams loadedParams{};
          RenderSettings loadedRender{};
          ColorPipelineWindowState loadedDraft{};
          error.clear();
          if (!LoadDiagnosticsStateJson(stateJson, &loadedView, &loadedParams, &loadedRender, &loadedDraft, &error)) {
            std::cerr << "Expected advanced color Palette-stack matrix tuple to reload: " << error << "\n";
            return 1;
          }

          if (loadedParams.color_palette_stack_count != 2 ||
              !ColorPipelinePaletteStackEntriesEqual(loadedParams.color_palette_stack[1], expectedSecondEntry)) {
            std::cerr << "Expected advanced color Palette-stack matrix to reload the second Palette row for function "
                      << paletteDescriptor.id << "\n";
            return 1;
          }

          ColorPipelineWindowState normalizedExpectedDraft = appliedDraft;
          ColorPipelineWindowState normalizedLoadedDraft = loadedDraft;
          error.clear();
          if (!NormalizeDraftForLaneComparison(&normalizedExpectedDraft, &error) ||
              !NormalizeDraftForLaneComparison(&normalizedLoadedDraft, &error)) {
            std::cerr << "Expected advanced color Palette-stack drafts to normalize for comparison: " << error << "\n";
            return 1;
          }
          if (!ColorPipelineLaneStatesEqual(
                  normalizedExpectedDraft.lanes[paletteLaneIndex],
                  normalizedLoadedDraft.lanes[paletteLaneIndex])) {
            std::cerr << "Expected advanced color Palette-stack matrix to preserve the serialized Palette lane for second-row function "
                      << paletteDescriptor.id << "\n";
            return 1;
          }

          coveredPaletteStackFunctions.insert(paletteDescriptor.id);
        }

        int expectedPaletteStackFunctionCount = 0;
        for (const FunctionDescriptor& paletteDescriptor : paletteCatalog.functions) {
          if (IsSupportedColorPipelinePaletteStackFunctionId(paletteDescriptor.id)) {
            ++expectedPaletteStackFunctionCount;
          }
        }
        if (static_cast<int>(coveredPaletteStackFunctions.size()) != expectedPaletteStackFunctionCount) {
          std::cerr << "Expected advanced color Palette-stack matrix to cover every supported non-first Palette function\n";
          return 1;
        }

        std::set<std::string> coveredGradingStackFunctions;
        for (const FunctionDescriptor& gradingDescriptor : gradingCatalog.functions) {
          if (!IsSupportedColorPipelineGradingFunctionId(gradingDescriptor.id)) {
            continue;
          }

          ColorPipelineWindowState candidateDraft = baseDraft;
          if (!SetColorPipelineRowFunction(&candidateDraft.lanes[sourceLaneIndex].rows[0], *baseSourceDescriptor) ||
              !SetColorPipelineRowFunction(&candidateDraft.lanes[shapeLaneIndex].rows[0], *identityDescriptor) ||
              !SetColorPipelineRowFunction(&candidateDraft.lanes[paletteLaneIndex].rows[0], *basePaletteDescriptor) ||
              !SetColorPipelineRowFunction(&candidateDraft.lanes[gradingLaneIndex].rows[0], *baseGradingDescriptor) ||
              !AppendDraftRowForFunction(&candidateDraft, gradingCatalog, gradingLaneIndex, gradingDescriptor, 200, &error)) {
            std::cerr << "Expected advanced color Grading-stack matrix draft setup to succeed: " << error << "\n";
            return 1;
          }

          if (!ConfigureDistinctDraftRowParams(*baseSourceDescriptor, &candidateDraft.lanes[sourceLaneIndex].rows[0], 0, &error) ||
              !ConfigureDistinctDraftRowParams(*identityDescriptor, &candidateDraft.lanes[shapeLaneIndex].rows[0], 0, &error) ||
              !ConfigureDistinctDraftRowParams(*basePaletteDescriptor, &candidateDraft.lanes[paletteLaneIndex].rows[0], 0, &error) ||
              !ConfigureDistinctDraftRowParams(*baseGradingDescriptor, &candidateDraft.lanes[gradingLaneIndex].rows[0], 0, &error) ||
              !ConfigureDistinctDraftRowParams(gradingDescriptor, &candidateDraft.lanes[gradingLaneIndex].rows[1], 1, &error)) {
            std::cerr << "Expected advanced color Grading-stack matrix params to be configurable: " << error << "\n";
            return 1;
          }

          const ColorPipelineDraftApplyState applyState =
              DescribeColorPipelineDraftApplyState(candidateDraft, stackView.fractal_type, &stackParams);
          if (applyState.status != ColorPipelineDraftApplyStatus::can_apply &&
              applyState.status != ColorPipelineDraftApplyStatus::matches_live) {
            std::cerr << "Expected advanced color Grading-stack matrix tuple to apply for second-row function "
                      << gradingDescriptor.id << ": " << applyState.message << "\n";
            return 1;
          }

          ColorPipelineGradingStackEntry expectedSecondEntry{};
          if (!TryBuildColorPipelineGradingStackEntryFromRow(candidateDraft.lanes[gradingLaneIndex].rows[1], &expectedSecondEntry, &error)) {
            std::cerr << "Expected advanced color Grading-stack matrix to build the second row entry: " << error << "\n";
            return 1;
          }

          KernelParams appliedParams = stackParams;
          ColorPipelineWindowState appliedDraft = candidateDraft;
          bool changed = false;
          if (!ApplyColorPipelineDraftToLiveState(&appliedDraft, stackView.fractal_type, &appliedParams, &changed)) {
            std::cerr << "Expected advanced color Grading-stack matrix tuple to apply for second-row function "
                      << gradingDescriptor.id << "\n";
            return 1;
          }
          if (appliedParams.color_grading_stack_count != 2 ||
              !ColorPipelineGradingStackEntriesEqual(appliedParams.color_grading_stack[1], expectedSecondEntry)) {
            std::cerr << "Expected advanced color Grading-stack matrix to preserve the second Grading row in live runtime state for function "
                      << gradingDescriptor.id << "\n";
            return 1;
          }

          DiagnosticsCaptureResult capture{};
          error.clear();
          if (!CaptureDiagnosticsLastBundle(
                  runtimeDir.string(),
                  stackView,
                  appliedParams,
                  stackRender,
                  stackStats,
                  rgba.data(),
                  rgba.size(),
                  &appliedDraft,
                  &capture,
                  &error)) {
            std::cerr << "Expected advanced color Grading-stack matrix tuple to serialize: " << error << "\n";
            return 1;
          }

          std::string stateJson;
          if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected advanced color Grading-stack matrix tuple to write diagnostics state.json\n";
            return 1;
          }

          ViewState loadedView{};
          KernelParams loadedParams{};
          RenderSettings loadedRender{};
          ColorPipelineWindowState loadedDraft{};
          error.clear();
          if (!LoadDiagnosticsStateJson(stateJson, &loadedView, &loadedParams, &loadedRender, &loadedDraft, &error)) {
            std::cerr << "Expected advanced color Grading-stack matrix tuple to reload: " << error << "\n";
            return 1;
          }

          if (loadedParams.color_grading_stack_count != 2 ||
              !ColorPipelineGradingStackEntriesEqual(loadedParams.color_grading_stack[1], expectedSecondEntry)) {
            std::cerr << "Expected advanced color Grading-stack matrix to reload the second Grading row for function "
                      << gradingDescriptor.id << "\n";
            return 1;
          }

          ColorPipelineWindowState normalizedExpectedDraft = appliedDraft;
          ColorPipelineWindowState normalizedLoadedDraft = loadedDraft;
          error.clear();
          if (!NormalizeDraftForLaneComparison(&normalizedExpectedDraft, &error) ||
              !NormalizeDraftForLaneComparison(&normalizedLoadedDraft, &error)) {
            std::cerr << "Expected advanced color Grading-stack drafts to normalize for comparison: " << error << "\n";
            return 1;
          }
          if (!ColorPipelineLaneStatesEqual(
                  normalizedExpectedDraft.lanes[gradingLaneIndex],
                  normalizedLoadedDraft.lanes[gradingLaneIndex])) {
            std::cerr << "Expected advanced color Grading-stack matrix to preserve the serialized Grading lane for second-row function "
                      << gradingDescriptor.id << "\n";
            return 1;
          }

          coveredGradingStackFunctions.insert(gradingDescriptor.id);
        }

        if (coveredGradingStackFunctions.size() != gradingCatalog.functions.size()) {
          std::cerr << "Expected advanced color Grading-stack matrix to cover every supported non-first Grading function\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "v3_widened_source_runtime_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
      "state_version": 3,
      "fractal_type": "newton",
      "view": {
      "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
      "rotation_degrees": 0.0,
      "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
      "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
      },
      "params": {
      "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
      "poly_kind": 0,
      "coloring_mode": "iteration_count",
      "color_signal": "root_proximity",
      "color_shape": "identity",
      "color_palette": "cyclic_escape",
      "color_grading": "escape_default",
      "nova_alpha": 0.5,
      "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
      "multibrot_power": 3,
      "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
      "poly_coeffs": [-1, 0, 0, 1, 0],
      "color_escape_magnitude_scale": 1.8,
      "color_escape_magnitude_bias": -0.25,
      "color_orbit_stripe_frequency": 3.5,
      "color_orbit_stripe_phase": 0.4,
      "color_root_proximity_scale": 2.25,
      "color_root_proximity_bias": -0.1
      },
      "render": { "width": 256, "height": 192, "block_size": 256, "device_id": 0 }
    })";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
          std::cerr << "V3 widened source runtime state load failed: " << error << "\n";
          return 1;
        }
        if (p.color_pipeline.signal != ColorSignal::root_proximity ||
          p.color_pipeline.palette != ColorPalette::cyclic_escape ||
          p.color_pipeline.grading != ColorGradingPreset::escape_default ||
          p.coloring_mode != ColoringMode::smooth_escape ||
          !NearlyEqual(p.color_escape_magnitude_scale, 1.8, 0.001) ||
          !NearlyEqual(p.color_escape_magnitude_bias, -0.25, 0.001) ||
          !NearlyEqual(p.color_orbit_stripe_frequency, 3.5, 0.001) ||
          !NearlyEqual(p.color_orbit_stripe_phase, 0.4, 0.001) ||
          !NearlyEqual(p.color_root_proximity_scale, 2.25, 0.001) ||
          !NearlyEqual(p.color_root_proximity_bias, -0.1, 0.001)) {
          std::cerr << "Expected widened source runtime state load to restore new source ids, owner fields, and the derived mirrored coloring_mode\n";
          return 1;
        }
      }

    std::cout << "test_diagnostics_state_io: all passed\n";
    return 0;
}