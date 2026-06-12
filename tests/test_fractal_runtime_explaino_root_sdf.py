from __future__ import annotations

import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    RUNTIME_DIR,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _require_frame_hash(payload: dict[str, Any]) -> str:
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), payload
    return frame_hash


def _require_root_hash(payload: dict[str, Any], key: str) -> str:
    value = payload.get(key)
    assert isinstance(value, str) and value.startswith("fnv1a64:"), payload
    return value


def _payload_has_control(payload: dict[str, Any], control_id: str) -> bool:
    controls = payload.get("controls")
    assert isinstance(controls, list), payload
    return any(isinstance(control, dict) and control.get("control_id") == control_id for control in controls)


def _root_sdf_state(exe_path: Path) -> dict[str, object]:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_root_sdf",
        "--width",
        "256",
        "--height",
        "192",
    )
    state = json.loads(json.dumps(capture["state"]))
    assert state.get("fractal_type") == "explaino_root_sdf", state
    return state


def test_explaino_root_sdf_selects_reports_and_mutates_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer UI automation is Windows-only")

    exe_path = active_runtime_exe()
    state = _root_sdf_state(exe_path)
    state_path = write_state_bundle(tmp_path / "explaino_root_sdf_seed", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "explaino_root_sdf_report.json",
        command_path=tmp_path / "explaino_root_sdf_command.json",
        open_color_pipeline=True,
    ) as viewer:
        baseline = viewer.wait_for_report(timeout_seconds=60.0)
        assert baseline.get("current_fractal_type") == "explaino_root_sdf", baseline
        assert baseline.get("lens_sdf_field_source") == "explaino_root_sdf", baseline
        assert baseline.get("lens_sdf_field_producer_kind") == "explaino_root_sdf", baseline
        assert baseline.get("lens_sdf_field_capability_fail_closed_reason") is None, baseline
        assert set(baseline.get("lens_sdf_supported_signals", [])) >= {
            "sdf_signed_distance",
            "sdf_inside_outside",
            "sdf_boundary_band",
            "sdf_normal_angle",
            "sdf_curvature",
        }, baseline
        assert baseline.get("source_stack_kind") == "sdf_only", baseline
        assert baseline.get("lens_sdf_valid") is True, baseline
        assert baseline.get("lens_sdf_width", 0) > 0, baseline
        assert baseline.get("lens_sdf_height", 0) > 0, baseline
        assert baseline.get("lens_sdf_backend_used") in {"cuda_jfa", "cpu_chamfer", "cuda_sample", "cpu_reference"}, baseline
        assert baseline.get("explaino_root_sdf_root_count") == 4, baseline
        assert baseline.get("explaino_root_sdf_bridge_count") == 2, baseline
        assert baseline.get("explaino_root_sdf_h_source") == "none", baseline

        baseline_frame_hash = _require_frame_hash(baseline)
        baseline_effective_hash = _require_root_hash(baseline, "explaino_root_sdf_effective_root_hash")
        assert not _payload_has_control(baseline, "fractal_control.explaino_warp_strength.primary"), baseline
        assert not _payload_has_control(baseline, "fractal_control.color_smooth_escape_interior_strength.primary"), baseline

        for control_id in [
            "fractal_control.explaino_root_sdf_radius.primary",
            "fractal_control.explaino_root_sdf_bridge_width.primary",
            "fractal_control.explaino_root_sdf_smooth_blend.primary",
            "fractal_control.explaino_root_sdf_h_amplitude.primary",
            "fractal_control.explaino_root_sdf_h_frequency.primary",
            "fractal_control.explaino_seed.primary",
            "fractal_control.explaino_phase.primary",
        ]:
            viewer.wait_for_control(control_id, timeout_seconds=30.0)

        next_seed = viewer.click_control("next_seed", timeout_seconds=60.0)
        assert next_seed.get("click_consumed") is True, next_seed
        assert next_seed.get("current_fractal_type") == "explaino_root_sdf", next_seed
        assert _require_root_hash(next_seed, "explaino_root_sdf_effective_root_hash") != baseline_effective_hash
        assert _require_frame_hash(next_seed) != baseline_frame_hash

        prev_seed = viewer.click_control("prev_seed", timeout_seconds=60.0)
        assert prev_seed.get("click_consumed") is True, prev_seed
        assert prev_seed.get("current_fractal_type") == "explaino_root_sdf", prev_seed
        assert _require_root_hash(prev_seed, "explaino_root_sdf_effective_root_hash") != _require_root_hash(next_seed, "explaino_root_sdf_effective_root_hash")

        radius_edited = viewer.set_control_value(
            "fractal_control.explaino_root_sdf_radius.primary",
            0.28,
            timeout_seconds=60.0,
        )
        assert radius_edited.get("current_fractal_type") == "explaino_root_sdf", radius_edited
        assert radius_edited.get("set_value_consumed") is True, radius_edited
        assert _require_frame_hash(radius_edited) != baseline_frame_hash, radius_edited
        assert _require_root_hash(radius_edited, "explaino_root_sdf_effective_root_hash") == baseline_effective_hash

        bridge_edited = viewer.set_control_value(
            "fractal_control.explaino_root_sdf_bridge_width.primary",
            0.0,
            timeout_seconds=60.0,
        )
        assert bridge_edited.get("explaino_root_sdf_bridge_count") == 0, bridge_edited
        assert _require_frame_hash(bridge_edited) != _require_frame_hash(radius_edited), bridge_edited

        smooth_edited = viewer.set_control_value(
            "fractal_control.explaino_root_sdf_smooth_blend.primary",
            0.32,
            timeout_seconds=60.0,
        )
        assert _require_frame_hash(smooth_edited) != _require_frame_hash(bridge_edited), smooth_edited

        seed_edited = viewer.set_control_value(
            "fractal_control.explaino_seed.primary",
            0.41,
            timeout_seconds=60.0,
        )
        assert _require_root_hash(seed_edited, "explaino_root_sdf_effective_root_hash") != baseline_effective_hash
        assert _require_frame_hash(seed_edited) != _require_frame_hash(smooth_edited), seed_edited

        inert_amplitude = viewer.set_control_value(
            "fractal_control.explaino_root_sdf_h_amplitude.primary",
            0.22,
            timeout_seconds=60.0,
        )
        inert_amplitude_hash = _require_root_hash(inert_amplitude, "explaino_root_sdf_effective_root_hash")
        inert_frequency = viewer.set_control_value(
            "fractal_control.explaino_root_sdf_h_frequency.primary",
            2.0,
            timeout_seconds=60.0,
        )
        assert inert_frequency.get("explaino_root_sdf_h_source") == "none", inert_frequency
        assert _require_root_hash(inert_frequency, "explaino_root_sdf_effective_root_hash") == inert_amplitude_hash

        h_enabled = viewer.set_enum_id(
            "fractal.params.explaino_root_sdf_h_source",
            "phase_sine",
            expected_fractal_type="explaino_root_sdf",
            timeout_seconds=60.0,
        )
        assert h_enabled.get("explaino_root_sdf_h_source") == "phase_sine", h_enabled
        h_enabled_hash = _require_root_hash(h_enabled, "explaino_root_sdf_effective_root_hash")
        assert h_enabled_hash != inert_amplitude_hash, h_enabled

        phase_edited = viewer.set_control_value(
            "fractal_control.explaino_phase.primary",
            0.33,
            timeout_seconds=60.0,
        )
        assert phase_edited.get("explaino_root_sdf_h_source") == "phase_sine", phase_edited
        assert _require_root_hash(phase_edited, "explaino_root_sdf_effective_root_hash") != h_enabled_hash

        viewer.wait_for_control("fractal_control.explaino_root_authority.primary", timeout_seconds=30.0)
        custom_authority = viewer.set_enum_id(
            "fractal.params.explaino_root_authority",
            "custom",
            expected_fractal_type="explaino_root_sdf",
            timeout_seconds=60.0,
        )
        assert custom_authority.get("current_fractal_type") == "explaino_root_sdf", custom_authority
        assert _payload_has_control(custom_authority, "fractal_control.explaino_root_0_x.primary")
        custom_hash = _require_root_hash(custom_authority, "explaino_root_sdf_effective_root_hash")

        root_edited = viewer.set_control_value(
            "fractal_control.explaino_root_0_x.primary",
            1.45,
            timeout_seconds=60.0,
        )
        assert root_edited.get("current_fractal_type") == "explaino_root_sdf", root_edited
        assert _require_root_hash(root_edited, "explaino_root_sdf_effective_root_hash") != custom_hash
        assert _require_frame_hash(root_edited) != _require_frame_hash(custom_authority), root_edited
        assert viewer.launch_count == 1


def test_explaino_root_sdf_rejects_non_sdf_source_rows_with_structured_reason() -> None:
    exe_path = active_runtime_exe()
    result = subprocess.run(
        [
            str(exe_path),
            "--fractal-type",
            "explaino_root_sdf",
            "--width",
            "96",
            "--height",
            "72",
            "--color-pipeline-action",
            "select_function:source:0:smooth_escape_ramp",
            "--capture-diagnostic",
        ],
        cwd=str(exe_path.parent),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode != 0, result.stdout + result.stderr
    combined = result.stdout + result.stderr
    assert "unsupported_source_for_producer" in combined, combined
    assert "producer_kind=explaino_root_sdf" in combined, combined
    assert "row_index=0" in combined, combined
    assert "source_id=smooth_escape" in combined, combined


def test_explaino_root_sdf_capture_finding_sidecar_and_replay(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    state = _root_sdf_state(exe_path)
    state["params"]["explaino_root_sdf_h_source"] = "phase_sine"
    state["params"]["explaino_root_sdf_h_amplitude"] = 0.18
    state["view"]["explaino_phase"] = 0.25
    state_path = write_state_bundle(tmp_path / "explaino_root_sdf_capture", state)

    group = f"pytest_explaino_root_sdf_{tmp_path.name}"
    group_root = RUNTIME_DIR.parent / "findings" / group
    if group_root.exists():
        shutil.rmtree(group_root)
    result = subprocess.run(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--capture-finding",
            "--finding-group",
            group,
            "--finding-why",
            "root sdf runtime sidecar proof",
        ],
        cwd=str(exe_path.parent),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    state_paths = sorted(group_root.rglob("state.json"))
    assert len(state_paths) == 1, state_paths
    finding_dir = state_paths[0].parent
    finding_state_path = finding_dir / "state.json"
    fractal_state_path = finding_dir / "fractal-state.json"
    finding_json_path = finding_dir / "finding.json"
    assert finding_state_path.exists(), finding_dir
    assert fractal_state_path.exists(), finding_dir
    assert finding_json_path.exists(), finding_dir

    finding_json = json.loads(finding_json_path.read_text(encoding="utf-8"))
    assert finding_json.get("fractal_state_file") == "fractal-state.json", finding_json
    sidecar = json.loads(fractal_state_path.read_text(encoding="utf-8"))
    assert sidecar.get("schema_id") == "viewer.finding_fractal_state.v1", sidecar
    capture_context = sidecar.get("capture_context")
    assert isinstance(capture_context, dict), sidecar
    assert capture_context.get("fractal_type") == "explaino_root_sdf", capture_context
    assert capture_context.get("selected_fractal_type") == "explaino_root_sdf", capture_context
    assert capture_context.get("public_selector_id") == "explaino_root_sdf", capture_context
    active_controls = sidecar.get("active_fractal_controls")
    assert isinstance(active_controls, dict), sidecar
    assert active_controls.get("explaino_root_sdf_h_source") == "phase_sine", active_controls
    assert active_controls.get("explaino_root_sdf_h_amplitude") == pytest.approx(0.18), active_controls
    assert "magnet_relaxation" not in active_controls, active_controls
    derived = sidecar.get("derived_runtime_values")
    assert isinstance(derived, dict), sidecar
    root_sdf = derived.get("explaino_root_sdf")
    assert isinstance(root_sdf, dict), derived
    assert root_sdf.get("producer_kind") == "explaino_root_sdf", root_sdf
    assert root_sdf.get("root_count") == 4, root_sdf
    assert root_sdf.get("bridge_count") == 2, root_sdf
    assert root_sdf.get("h_source") == "phase_sine", root_sdf
    assert isinstance(root_sdf.get("base_root_hash"), str), root_sdf
    assert isinstance(root_sdf.get("effective_root_hash"), str), root_sdf
    assert root_sdf.get("base_root_hash") != root_sdf.get("effective_root_hash"), root_sdf

    replay_a = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(finding_state_path),
        "--capture-diagnostic",
    )
    replay_b = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(finding_state_path),
        "--capture-diagnostic",
    )
    assert replay_b["frame_hash"] == replay_a["frame_hash"]
