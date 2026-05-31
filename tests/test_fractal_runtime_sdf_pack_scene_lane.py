from __future__ import annotations

import json
import subprocess
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


def _built_in_pack_ids(payload: dict[str, Any]) -> set[str]:
    options = _sdf_pack_report(payload).get("built_in_packs")
    assert isinstance(options, list), payload
    ids: set[str] = set()
    for option in options:
        assert isinstance(option, dict), option
        pack_id = option.get("pack_id")
        assert isinstance(pack_id, str) and pack_id, option
        assert isinstance(option.get("label"), str) and option.get("label"), option
        ids.add(pack_id)
    return ids


def test_published_runtime_stages_builtin_sdf_pack_files() -> None:
    exe_path = active_runtime_exe()
    runtime_dir = exe_path.parent
    expected = [
        "sdf_smooth_lattice_2d.sdf_pack.json",
        "sdf_capsule_weave_2d.sdf_pack.json",
        "sdf_ring_cells_2d.sdf_pack.json",
    ]
    for file_name in expected:
        staged = runtime_dir / "docs" / "examples" / "sdf_packs" / file_name
        assert staged.exists(), f"published runtime did not stage built-in SDF pack {staged}"


def test_field_primary_lanes_fail_closed_for_mixed_renderer_source_rows() -> None:
    exe_path = active_runtime_exe()
    for fractal_type in ("sdf_pack_scene", "generic_equation_pack"):
        result = subprocess.run(
            [
                str(exe_path),
                "--fractal-type",
                fractal_type,
                "--width",
                "96",
                "--height",
                "72",
                "--color-pipeline-action",
                "select_function:source:0:smooth_escape_ramp",
                "--color-pipeline-action",
                "add_row:source:sdf_signed_distance",
                "--capture-diagnostic",
            ],
            cwd=str(exe_path.parent),
            text=True,
            capture_output=True,
            check=False,
        )
        assert result.returncode != 0, result.stdout + result.stderr
        combined = result.stdout + result.stderr
        assert (
            f"{fractal_type} mixed Source rows require renderer-backed non-SDF source signals"
            in combined
        ), combined


def _first_pack_control_id(payload: dict[str, Any]) -> str:
    controls = _sdf_pack_report(payload).get("controls")
    assert isinstance(controls, list), payload
    for control in controls:
        assert isinstance(control, dict), control
        control_id = control.get("control_id")
        if isinstance(control_id, str) and control_id.endswith(".primary"):
            assert control_id != "sdf_pack.use_as_sdf_field_source.primary"
            return control_id
    raise AssertionError(f"no editable SDF pack control reported: {payload!r}")


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
        assert selected.get("lens_sdf_field_producer_kind") == "sdf_pack_scene", selected
        assert set(selected.get("lens_sdf_supported_signals", [])) >= {
            "sdf_signed_distance",
            "sdf_inside_outside",
            "sdf_boundary_band",
            "sdf_normal_angle",
            "sdf_curvature",
            "lens_field_v2_distance",
        }, selected
        assert selected.get("lens_sdf_field_capability_fail_closed_reason") is None, selected
        assert selected.get("lens_sdf_field_source_pack_id") == "sdf_smooth_lattice_2d", selected
        assert selected.get("lens_sdf_valid") is True, selected
        assert selected.get("lens_sdf_pack_backend_used") in {"cuda_sample", "cpu_reference"}, selected

        viewer.wait_for_control("sdf_pack.builtin_pack", timeout_seconds=20.0)
        for control_id in expected_controls:
            viewer.wait_for_control(control_id, timeout_seconds=20.0)
        assert expected_controls.issubset(_control_ids(selected)), selected
        report = _sdf_pack_report(selected)
        assert report.get("pack_id") == "sdf_smooth_lattice_2d", selected
        assert report.get("built_in_pack_selector_control_id") == "sdf_pack.builtin_pack", selected
        assert report.get("selected_built_in_pack_id") == "sdf_smooth_lattice_2d", selected
        assert {
            "sdf_smooth_lattice_2d",
            "sdf_capsule_weave_2d",
            "sdf_ring_cells_2d",
        }.issubset(_built_in_pack_ids(selected)), selected

        previous_hash = _require_frame_hash(selected)
        selected_center_y = selected.get("view_center_hp_y")
        assert isinstance(selected_center_y, (int, float)), selected
        panned = viewer.pan_viewport_pixels(
            0.0,
            48.0,
            expected_fractal_type="sdf_pack_scene",
            timeout_seconds=60.0,
        )
        assert panned.get("current_fractal_type") == "sdf_pack_scene", panned
        panned_center_y = panned.get("view_center_hp_y")
        assert isinstance(panned_center_y, (int, float)), panned
        assert panned_center_y < selected_center_y, panned
        assert _require_frame_hash(panned) != previous_hash, panned
        previous_hash = _require_frame_hash(panned)

        for pack_id in ["sdf_capsule_weave_2d", "sdf_ring_cells_2d", "sdf_smooth_lattice_2d"]:
            switched = viewer.set_enum_id(
                "sdf_pack.builtin_pack",
                pack_id,
                expected_fractal_type="sdf_pack_scene",
                timeout_seconds=60.0,
            )
            assert switched.get("lens_sdf_field_source") == "authored_sdf_pack", switched
            assert switched.get("lens_sdf_field_source_pack_id") == pack_id, switched
            switched_report = _sdf_pack_report(switched)
            assert switched_report.get("pack_id") == pack_id, switched
            assert switched_report.get("selected_built_in_pack_id") == pack_id, switched
            assert _first_pack_control_id(switched).startswith("sdf_pack."), switched
            switched_hash = _require_frame_hash(switched)
            assert switched_hash != previous_hash, (pack_id, switched)
            previous_hash = switched_hash

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
