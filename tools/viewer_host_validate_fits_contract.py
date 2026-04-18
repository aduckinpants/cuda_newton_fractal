from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

try:
    from tools.viewer_host_contract_state import REPO_ROOT, load_json_file
except ModuleNotFoundError:
    from viewer_host_contract_state import REPO_ROOT, load_json_file


DEFAULT_MAPPING_PATH = REPO_ROOT / "ui" / "runtime_walk_fits_mapping_profiles_v1.json"
BOOTSTRAP_HEADER_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk_bootstrap.h"
BOOTSTRAP_CPP_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk_bootstrap.cpp"
EXTRACTOR_PATH = REPO_ROOT / "tools" / "runtime_walk_extract_fits_orientation.py"
VIEWER_IMGUI_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk_viewer_imgui.cpp"
RUNTIME_WALK_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk.cpp"
RUNTIME_WALK_HEADER_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk.h"
RUNTIME_WALK_IMPORT_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk_viewer_import.cpp"
FIELD_SLIME_HEADER_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk_field_slime.h"
FIELD_SLIME_CPP_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk_field_slime.cpp"
MAIN_PATH = REPO_ROOT / "ui_app" / "src" / "main.cpp"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate the shipped FITS runtime-walk contract defaults")
    parser.add_argument("--contract", required=True, help="Path to the runtime_walk_fits contract JSON")
    parser.add_argument("--out-json", help="Optional JSON output path")
    args = parser.parse_args(argv)

    contract_path = Path(args.contract)
    if not contract_path.is_absolute():
        contract_path = REPO_ROOT / contract_path
    if not contract_path.exists():
        sys.stderr.write(f"viewer_host_validate_fits_contract: missing contract: {contract_path}\n")
        return 2

    contract_payload = load_json_file(contract_path)
    mapping_payload = load_json_file(DEFAULT_MAPPING_PATH)
    header_text = BOOTSTRAP_HEADER_PATH.read_text(encoding="utf-8")
    bootstrap_text = BOOTSTRAP_CPP_PATH.read_text(encoding="utf-8")
    extractor_text = EXTRACTOR_PATH.read_text(encoding="utf-8")
    imgui_text = VIEWER_IMGUI_PATH.read_text(encoding="utf-8")
    runtime_walk_text = RUNTIME_WALK_PATH.read_text(encoding="utf-8")
    runtime_walk_header_text = RUNTIME_WALK_HEADER_PATH.read_text(encoding="utf-8")
    import_text = RUNTIME_WALK_IMPORT_PATH.read_text(encoding="utf-8")
    field_slime_header_text = FIELD_SLIME_HEADER_PATH.read_text(encoding="utf-8")
    field_slime_text = FIELD_SLIME_CPP_PATH.read_text(encoding="utf-8")
    main_text = MAIN_PATH.read_text(encoding="utf-8")

    errors: list[str] = []
    bindings = []
    for profile in mapping_payload.get("profiles", []):
        bindings.extend(profile.get("bindings", []))

    checks = {
        "default_mapping_has_no_warp_binding": not any(
            "warp" in str(binding.get("target_path", "")) for binding in bindings
        ),
        "default_synthesized_fractal_is_explaino": 'FractalType base_fractal_type = FractalType::explaino;' in header_text,
        "default_ui_has_no_warp_control": '"Warp Motion"' not in imgui_text,
        "default_runtime_transport_has_no_warp_animation":
            "ioParams->explaino_warp_strength =" not in runtime_walk_text and "warpNorm" not in runtime_walk_text,
        "field_csv_paths_visible":
            "runtime_walk_flow_lines.csv" in imgui_text and "runtime_field_cells.csv" in imgui_text,
        "field_slime_runtime_export_wired":
            "WriteRuntimeWalkFieldSlimeCsv" in main_text and "StepRuntimeWalkFieldSlime" in main_text,
        "transport_metadata_has_no_warp_scale":
            "transport_warp_scale" not in runtime_walk_header_text and
            "transport_warp_scale" not in import_text and
            "warp_scale" not in import_text,
        "binding_workbench_controls_visible": all(
            needle in imgui_text for needle in [
                "Binding Workbench",
                "FITS Source",
                "Runtime Target",
                "Amount",
                "Smoothing",
                "Invert Polarity",
                "Clamp",
                "Add Binding",
                "Remove##binding",
            ]
        ),
        "adaptive_field_controls_visible": all(
            needle in imgui_text for needle in [
                "Adaptive Field Sampling",
                "Min Marbles",
                "Max Marbles",
                "Gradient Sensitivity",
                "Traveler Hysteresis",
                "Export Cadence",
            ]
        ),
        "effective_mapping_profile_visible": "effective_mapping_profile.json" in import_text,
        "fits_timeline_extractor_emits_frames": all(
            needle in extractor_text for needle in [
                '"frame_count"',
                '"frames"',
                'frame_index',
                'tangent_angle',
            ]
        ),
        "orientation_timeline_parser_wired": all(
            needle in header_text + bootstrap_text for needle in [
                "RuntimeWalkFitsSignalFrame",
                "EvaluateRuntimeWalkFitsSignalAtT",
                "inputs.frames",
            ]
        ),
        "live_binding_runtime_composes_offsets": all(
            needle in bootstrap_text for needle in [
                "ComposeRuntimeWalkFitsBindingsOverLiveBaseline",
                "baselineValue + offsetValue",
                "RuntimeWalkFitsFieldSignals",
            ]
        ),
        "measured_field_overlay_default_wired": (
            "BuildRuntimeWalkMeasuredFieldOverlay(fieldSlimeState" in main_text and
            "BuildRuntimeWalkGradientOverlay" in main_text and
            main_text.find("BuildRuntimeWalkMeasuredFieldOverlay(fieldSlimeState") < main_text.rfind("BuildRuntimeWalkGradientOverlay")
        ),
        "binding_samples_csv_export_wired": (
            "runtime_walk_binding_samples.csv" in main_text and
            "WriteRuntimeWalkBindingSamplesCsv" in main_text and
            "t,source_path,target_path,frame_index" in main_text and
            "Binding Samples CSV" in imgui_text
        ),
        "stale_live_exports_removed_on_regeneration": all(
            needle in import_text for needle in [
                "RemoveStaleLivePlaybackExports",
                "runtime_walk_flow_lines.csv.tmp",
                "runtime_walk_binding_samples.csv.tmp",
            ]
        ),
        "runtime_target_domain_clamps_mix": (
            "ClampRuntimeWalkTargetDomainValue" in bootstrap_text and
            'targetPath == "fractal.params.explaino_mix"' in bootstrap_text and
            not any(
                binding.get("target_path") == "fractal.params.explaino_mix" and float(binding.get("clamp_max", 0.0)) > 1.0
                for binding in bindings
            )
        ),
        "playback_delta_has_wall_clock_fallback": all(
            needle in main_text for needle in [
                "lastRuntimeWalkTick",
                "wallDeltaSeconds",
                "playbackDeltaSeconds",
            ]
        ),
        "flow_csv_fits_frame_context_wired": all(
            needle in field_slime_header_text + field_slime_text + main_text for needle in [
                "RuntimeWalkFieldSlimeExportContext",
                "fits_frame_index",
                "RuntimeWalkActiveFitsFrameIndex",
            ]
        ),
        "live_binding_controls_tunable": all(
            needle in imgui_text for needle in [
                "Live FITS Binding Workbench",
                "Curve##live_binding",
                "Invert Polarity##live_binding",
                "Clamp##live_binding",
                "Clamp Min##live_binding",
                "Clamp Max##live_binding",
            ]
        ),
    }
    if not checks["default_mapping_has_no_warp_binding"]:
        errors.append("default FITS mapping still binds a warp target")
    if not checks["default_synthesized_fractal_is_explaino"]:
        errors.append("runtime_walk_bootstrap.h does not default base_fractal_type to explaino")
    if not checks["default_ui_has_no_warp_control"]:
        errors.append("runtime_walk_viewer_imgui.cpp still exposes Warp Motion in the default UI")
    if not checks["default_runtime_transport_has_no_warp_animation"]:
        errors.append("runtime_walk.cpp still animates or writes explaino_warp_strength in default transport")
    if not checks["field_csv_paths_visible"]:
        errors.append("runtime_walk_viewer_imgui.cpp does not show field-flow CSV artifact paths")
    if not checks["field_slime_runtime_export_wired"]:
        errors.append("main.cpp does not wire field slime stepping/export into runtime playback")
    if not checks["transport_metadata_has_no_warp_scale"]:
        errors.append("runtime-walk generated transport/session metadata still exposes transport_warp_scale/warp_scale")
    if not checks["binding_workbench_controls_visible"]:
        errors.append("runtime_walk_viewer_imgui.cpp does not expose binding workbench source/target/safety controls")
    if not checks["adaptive_field_controls_visible"]:
        errors.append("runtime_walk_viewer_imgui.cpp does not expose adaptive field sampling controls")
    if not checks["effective_mapping_profile_visible"]:
        errors.append("runtime_walk_viewer_import.cpp does not write a durable effective mapping profile artifact")
    if not checks["fits_timeline_extractor_emits_frames"]:
        errors.append("runtime_walk_extract_fits_orientation.py does not emit a per-frame FITS timeline")
    if not checks["orientation_timeline_parser_wired"]:
        errors.append("runtime_walk_bootstrap does not parse/evaluate FITS signal timelines")
    if not checks["live_binding_runtime_composes_offsets"]:
        errors.append("runtime_walk_bootstrap does not compose live binding offsets over baseline state")
    if not checks["measured_field_overlay_default_wired"]:
        errors.append("main.cpp does not prefer measured field overlay before synthetic runtime-walk overlay")
    if not checks["binding_samples_csv_export_wired"]:
        errors.append("binding sample CSV export/path visibility is not wired")
    if not checks["stale_live_exports_removed_on_regeneration"]:
        errors.append("generated FITS session regeneration does not remove stale live export CSVs")
    if not checks["runtime_target_domain_clamps_mix"]:
        errors.append("live FITS binding defaults can still drive explaino_mix outside [0,1]")
    if not checks["playback_delta_has_wall_clock_fallback"]:
        errors.append("runtime-walk playback lacks a wall-clock delta fallback for zero ImGui delta")
    if not checks["flow_csv_fits_frame_context_wired"]:
        errors.append("field slime CSV export does not include active FITS frame context")
    if not checks["live_binding_controls_tunable"]:
        errors.append("live FITS binding workbench does not expose curve/polarity/clamp controls")

    expected_default = str(contract_payload.get("required_defaults", {}).get("base_fractal_type", ""))
    if expected_default and expected_default != "explaino":
        errors.append("runtime_walk_fits contract no longer declares explaino as the required default fractal")

    if errors:
        sys.stderr.write("viewer_host_validate_fits_contract: FAILED\n")
        for error in errors:
            sys.stderr.write(f"- {error}\n")
        return 2

    payload = {
        "ok": True,
        "contract": str(contract_path.relative_to(REPO_ROOT).as_posix()),
        "mapping": str(DEFAULT_MAPPING_PATH.relative_to(REPO_ROOT).as_posix()),
        "checks": checks,
    }
    if args.out_json:
        out_path = Path(args.out_json)
        if not out_path.is_absolute():
            out_path = REPO_ROOT / out_path
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
