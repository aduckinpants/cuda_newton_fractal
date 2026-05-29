from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
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


def _sdf_pack_report(payload: dict[str, Any]) -> dict[str, Any]:
    report = payload.get("sdf_pack_viewer")
    assert isinstance(report, dict), payload
    return report


def _control_ids(payload: dict[str, Any]) -> set[str]:
    controls = _sdf_pack_report(payload).get("controls")
    assert isinstance(controls, list), payload
    ids: set[str] = set()
    for control in controls:
        assert isinstance(control, dict), control
        control_id = control.get("control_id")
        assert isinstance(control_id, str), control
        ids.add(control_id)
    return ids


def test_sdf_pack_scene_lane_selects_and_edits_built_in_pack_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer UI automation is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "julia",
        "--width",
        "256",
        "--height",
        "192",
    )
    state_path = write_state_bundle(tmp_path / "sdf_pack_scene_seed", json.loads(json.dumps(neutral_capture["state"])))

    expected_controls = {
        "sdf_pack.period.primary",
        "sdf_pack.radius.primary",
        "sdf_pack.smooth_blend.primary",
        "sdf_pack.rotation.primary",
        "sdf_pack.offset_x.primary",
        "sdf_pack.offset_y.primary",
    }

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "sdf_pack_scene_report.json",
        command_path=tmp_path / "sdf_pack_scene_command.json",
        open_color_pipeline=True,
    ) as viewer:
        selected = viewer.set_enum_id(
            "fractal.view.fractal_type",
            "sdf_pack_scene",
            expected_fractal_type="sdf_pack_scene",
            timeout_seconds=40.0,
        )
        assert selected.get("current_fractal_type") == "sdf_pack_scene", selected
        assert selected.get("lens_sdf_field_source") == "authored_sdf_pack", selected
        assert selected.get("lens_sdf_field_source_pack_id") == "sdf_smooth_lattice_2d", selected
        assert selected.get("lens_sdf_valid") is True, selected
        assert selected.get("lens_sdf_pack_backend_used") in {"cuda_sample", "cpu_reference"}, selected

        for control_id in expected_controls:
            viewer.wait_for_control(control_id, timeout_seconds=20.0)
        assert expected_controls.issubset(_control_ids(selected)), selected
        assert _sdf_pack_report(selected).get("pack_id") == "sdf_smooth_lattice_2d", selected

        previous_hash = _require_frame_hash(selected)
        edits = [
            ("sdf_pack.period.primary", 1.35),
            ("sdf_pack.radius.primary", 0.24),
            ("sdf_pack.smooth_blend.primary", 0.28),
            ("sdf_pack.rotation.primary", 1.05),
            ("sdf_pack.offset_x.primary", 0.32),
            ("sdf_pack.offset_y.primary", -0.28),
        ]
        for control_id, value in edits:
            edited = viewer.set_control_value(control_id, value, timeout_seconds=60.0)
            assert edited.get("current_fractal_type") == "sdf_pack_scene", edited
            assert edited.get("lens_sdf_field_source") == "authored_sdf_pack", edited
            assert edited.get("lens_sdf_field_source_pack_id") == "sdf_smooth_lattice_2d", edited
            assert edited.get("set_value_consumed") is True, edited
            edited_hash = _require_frame_hash(edited)
            assert edited_hash != previous_hash, (control_id, edited)
            previous_hash = edited_hash

        viewer.wait_for_control(
            "color_pipeline.source.sdf_signed_distance.signal.scale.primary",
            timeout_seconds=20.0,
        )
        color_edited = viewer.set_control_value(
            "color_pipeline.source.sdf_signed_distance.signal.scale.primary",
            0.28,
            timeout_seconds=60.0,
        )
        assert color_edited.get("current_fractal_type") == "sdf_pack_scene", color_edited
        assert color_edited.get("lens_sdf_field_source") == "authored_sdf_pack", color_edited
        assert _require_frame_hash(color_edited) != previous_hash, color_edited
        assert viewer.launch_count == 1


def test_sdf_pack_scene_lane_state_replays_built_in_pack_pixels(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    first = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "sdf_pack_scene",
        "--width",
        "192",
        "--height",
        "144",
    )
    state = first["state"]
    assert isinstance(state, dict)
    assert state.get("fractal_type") == "sdf_pack_scene", state
    sdf_pack = state.get("sdf_pack")
    assert isinstance(sdf_pack, dict), state
    assert sdf_pack.get("pack_id") == "sdf_smooth_lattice_2d", sdf_pack
    assert sdf_pack.get("params", {}).get("period") == pytest.approx(0.85), sdf_pack

    replay_state = write_state_bundle(tmp_path / "sdf_pack_scene_replay", state)
    replay = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_state),
        "--capture-diagnostic",
    )
    assert replay["frame_hash"] == first["frame_hash"]
