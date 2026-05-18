from __future__ import annotations

import json
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
ANSWER_PATH = REPO_ROOT / "docs" / "notes" / "explaino_native_meta_basin_viability_owner_seam_answer.v1.json"
SCHEMA_PATH = REPO_ROOT / "ui" / "fractal_binding_surface_v1.ui_schema.json"
SOURCE_PATHS = {
    "enum_ids": REPO_ROOT / "ui_app" / "src" / "enum_id_utils.h",
    "fractal_types": REPO_ROOT / "ui_app" / "src" / "fractal_types.h",
    "family_rules": REPO_ROOT / "ui_app" / "src" / "fractal_family_rules.h",
    "sample_result": REPO_ROOT / "ui_app" / "src" / "fractal_sample_result.h",
    "sample_core": REPO_ROOT / "ui_app" / "src" / "fractal_sample_core.cu",
    "sample_device": REPO_ROOT / "ui_app" / "src" / "fractal_sample_device.inl",
    "renderer": REPO_ROOT / "ui_app" / "src" / "fractal_renderer.cu",
    "schema_binding": REPO_ROOT / "ui_app" / "src" / "schema_binding.cpp",
    "runtime_validation": REPO_ROOT / "ui_app" / "src" / "fractal_runtime_validation.h",
    "field_slime": REPO_ROOT / "ui_app" / "src" / "runtime_walk_field_slime.cpp",
    "sidecar_measurement": REPO_ROOT / "ui_app" / "src" / "explaino_sidecar_measurement.cpp",
    "sidecar_lens": REPO_ROOT / "ui_app" / "src" / "explaino_sidecar_lens.cpp",
    "sidecar_window": REPO_ROOT / "ui_app" / "src" / "explaino_sidecar_window.cpp",
}
EXPECTED_REQUIRED_OWNER_FILES = [
    "ui/fractal_binding_surface_v1.ui_schema.json",
    "ui_app/src/schema_binding.cpp",
    "ui_app/src/fractal_runtime_validation.h",
    "ui_app/src/fractal_sample_result.h",
    "ui_app/src/fractal_sample_core.cu",
    "ui_app/src/fractal_sample_device.inl",
    "ui_app/src/fractal_renderer.cu",
]
EXPECTED_BLOCKED_CONTROL_PATHS = [
    "fractal.view.auto_increment_seed",
    "fractal.params.epsilon",
    "fractal.params.projection_and_flow_root_family",
    "fractal.params.projection_and_flow_target_radius",
    "fractal.params.projection_and_flow_pressure_threshold",
    "fractal.params.explaino_seed",
    "fractal.actions.prev_seed",
    "fractal.actions.next_seed",
    "fractal.params.explaino_warp_strength",
    "fractal.params.explaino_root_spread",
    "fractal.view.explaino_phase_strength",
    "fractal.params.explaino_damping",
    "fractal.view.explaino_phase",
    "fractal.view.explaino_seed_drift",
    "fractal.view.explaino_seed_tween",
]
EXPECTED_STATE_FIELDS = [
    "measurement_state",
    "choice_state",
    "uncertainty_state",
    "action_sequence_state",
    "memory_trace_state",
    "motif_attractor_identity",
]
EXPECTED_STOP_CONDITIONS = [
    "fixed_step_budget",
    "attractor_convergence",
    "uncertainty_collapse",
    "return_to_baseline_event",
    "divergence_or_invalid_state_guard",
]
EXPECTED_REQUIRED_OUTPUTS = [
    "motif_attractor_identity",
    "action_itinerary_family",
    "cumulative_information_gain",
    "terminal_uncertainty",
    "explanatory_return_error",
    "basin_graph_transition_class",
    "reversible_vs_nonreversible_path_type",
]


def _load_json(path: Path) -> dict[str, object]:
    return json.loads(path.read_text(encoding="utf-8"))


def _all_controls() -> list[dict[str, object]]:
    schema = _load_json(SCHEMA_PATH)
    controls: list[dict[str, object]] = []
    for panel in schema["panels"]:
        controls.extend(panel.get("controls", []))
    return controls


def _fractal_selector_option_ids() -> list[str]:
    for control in _all_controls():
        if control.get("id") == "fractal_type":
            return [option["id"] for option in control.get("options", [])]
    raise AssertionError("fractal_type control missing from checked-in schema")


def _visible_for(control: dict[str, object], fractal_type_id: str) -> bool:
    visible_if = control.get("visible_if")
    if not isinstance(visible_if, dict):
        return False
    if visible_if.get("path") != "fractal.view.fractal_type":
        return False
    if visible_if.get("op") not in {"eq", "in"}:
        return False
    value = visible_if.get("value")
    if not isinstance(value, str):
        return False
    return fractal_type_id in value.split(",")


def _binding_paths_visible_for(fractal_type_id: str) -> list[str]:
    paths: list[str] = []
    for control in _all_controls():
        if not _visible_for(control, fractal_type_id):
            continue
        binding = control.get("binding")
        if not isinstance(binding, dict):
            continue
        path = binding.get("path")
        if isinstance(path, str):
            paths.append(path)
    return paths


def test_meta_basin_answer_is_explicit_and_blocked_on_current_head() -> None:
    answer = _load_json(ANSWER_PATH)

    assert answer["status"] == "blocked_with_exact_owner_seam_proof"
    assert answer["supportability_outcome"] == "blocked_without_larger_refactor"
    assert answer["owner_seam_classification"] == "another_owner_seam_entirely"

    owner_seam = answer["first_runtime_owner_seam"]
    assert owner_seam["kind"] == "new_explanation_state_runtime_contract"
    assert owner_seam["required_owner_files"] == EXPECTED_REQUIRED_OWNER_FILES
    rejected = {entry["carrier"]: entry["reason"] for entry in owner_seam["rejected_current_carriers"]}
    assert set(rejected) == {
        "explaino_projection_and_flow",
        "projection_and_flow",
        "explaino_sidecar_family",
        "runtime_walk_field_slime",
    }
    assert "projected-root class" in rejected["explaino_projection_and_flow"]
    assert "pressure-band" in rejected["projection_and_flow"]
    assert "post-sample" in rejected["explaino_sidecar_family"]
    assert "host/runtime walk" in rejected["runtime_walk_field_slime"]

    state_model = answer["state_attractor_model"]
    assert state_model["kind"] == "higher_order_explanation_state"
    assert state_model["required_state_fields"] == EXPECTED_STATE_FIELDS
    assert state_model["required_stop_conditions"] == EXPECTED_STOP_CONDITIONS

    render_output = answer["classification_render_output"]
    assert render_output["kind"] == "explanation_trajectory_outcome"
    assert render_output["primary_semantics"] == "basins are attractors of explanation trajectories, not roots of P(z)"
    assert render_output["required_outputs"] == EXPECTED_REQUIRED_OUTPUTS
    assert "no explanation-state class channel" in render_output["blocked_reason"]

    legacy_surface = answer["legacy_projection_surface"]
    assert legacy_surface["sample_fractal_points_preserved"] is True
    assert legacy_surface["fractal_sample_result_preserved"] is True
    assert legacy_surface["sample_fractal_points_semantics"] == "legacy_projection_only"

    assert answer["out_of_scope_confirmed"] == {
        "operator_itinerary": True,
        "dsl_program_space": True,
        "generic_engine_refactor": True,
        "broad_sample_evidence_reopen": True,
    }


def test_current_head_has_no_shipped_meta_basin_lane_and_legacy_projection_is_still_legacy() -> None:
    option_ids = _fractal_selector_option_ids()
    assert "meta_basin" not in option_ids
    assert "explaino_meta_basin" not in option_ids

    for source_path in SOURCE_PATHS.values():
        text = source_path.read_text(encoding="utf-8")
        assert "meta_basin" not in text
        assert "explaino_meta_basin" not in text

    family_rules = SOURCE_PATHS["family_rules"].read_text(encoding="utf-8")
    assert "return FractalType::explaino_ripple;" in family_rules
    assert "return FractalType::explaino_balance_void;" in family_rules
    assert "return FractalType::explaino;" in family_rules

    sample_device = SOURCE_PATHS["sample_device"].read_text(encoding="utf-8")
    assert "projection_and_flow_pressure_threshold" in sample_device
    assert "projectionClass =" in sample_device

    renderer = SOURCE_PATHS["renderer"].read_text(encoding="utf-8")
    assert "ResolveBasinRenderRootIndex" in renderer
    assert "MakeProgrammableBasinColor" in renderer

    sample_result_header = SOURCE_PATHS["sample_result"].read_text(encoding="utf-8")
    assert "struct FractalSampleEvidence" in sample_result_header
    assert "Double2 sample_coord{0.0, 0.0};" in sample_result_header
    assert "FractalSampleResult legacy_result{};" in sample_result_header
    assert "return evidence.legacy_result;" in sample_result_header


def test_explaino_projection_and_flow_control_surface_is_explicitly_blocked() -> None:
    answer = _load_json(ANSWER_PATH)
    audit = answer["visible_control_audit"]

    assert audit["status"] == "blocked_with_proof"
    assert audit["candidate_carrier_under_audit"] == "explaino_projection_and_flow"
    assert audit["authoritative_controls"] == []
    assert audit["intentionally_hidden_controls"] == []

    blocked_controls = audit["explicitly_blocked_controls"]
    assert [entry["binding_path"] for entry in blocked_controls] == EXPECTED_BLOCKED_CONTROL_PATHS
    assert all(entry["classification"] == "explicitly_blocked_with_proof" for entry in blocked_controls)
    assert audit["one_at_a_time_sensitivity"]["status"] == "not_applicable_no_authoritative_controls"

    visible_paths = _binding_paths_visible_for("explaino_projection_and_flow")
    assert set(EXPECTED_BLOCKED_CONTROL_PATHS).issubset(set(visible_paths))
    assert not any("meta_basin" in path for path in visible_paths)
