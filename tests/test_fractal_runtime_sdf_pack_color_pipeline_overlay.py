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


def _sdf_pack() -> dict[str, object]:
    return {
        "schema": 1,
        "pack_id": "runtime_field_source_circle",
        "name": "Runtime Field Source Circle",
        "kind": "sdf_scene_2d",
        "params": [
            {"id": "radius", "type": "float", "default": 0.45, "range": [0.1, 1.2]},
            {"id": "offset_x", "type": "float", "default": 0.0, "range": [-1.0, 1.0]},
        ],
        "controls": [
            {"param": "radius", "label": "Radius", "ui_min": 0.1, "ui_max": 1.0},
            {"param": "offset_x", "label": "Offset X", "ui_min": -0.75, "ui_max": 0.75},
        ],
        "region": {"center": [0.0, 0.0], "half_height": 1.25},
        "ast": {
            "op": "circle",
            "center": [{"param": "offset_x"}, 0.0],
            "radius": {"param": "radius"},
        },
    }


def _sdf_source_state(exe_path: Path, tmp_path: Path) -> Path:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "mandelbrot",
        "--width",
        "192",
        "--height",
        "144",
        "--color-pipeline-action",
        "select_function:source:0:sdf_signed_distance",
        "--color-pipeline-action",
        "set_param:source:0:signal.scale:number:0.08",
        "--color-pipeline-action",
        "set_param:source:0:signal.bias:number:0.5",
    )
    state = json.loads(json.dumps(capture["state"]))
    lens = state.setdefault("lens", {})
    assert isinstance(lens, dict)
    lens["enabled"] = False
    lens["downsample"] = 2
    return write_state_bundle(tmp_path / "sdf_pack_color_pipeline_state", state)


def _sdf_pack_report(payload: dict[str, Any]) -> dict[str, Any]:
    report = payload.get("sdf_pack_viewer")
    assert isinstance(report, dict), payload
    return report


def test_authored_sdf_pack_field_drives_color_pipeline_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer UI automation is Windows-only")

    exe_path = active_runtime_exe()
    state_path = _sdf_source_state(exe_path, tmp_path)
    pack_path = tmp_path / "runtime_field_source_circle.sdf_pack.json"
    pack_path.write_text(json.dumps(_sdf_pack(), indent=2), encoding="utf-8")

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "sdf_pack_color_pipeline_report.json",
        command_path=tmp_path / "sdf_pack_color_pipeline_command.json",
        extra_args=[
            "--open-sdf-pack-panel",
            "--sdf-pack-json",
            str(pack_path),
        ],
    ) as viewer:
        viewer.wait_for_control("sdf_pack.use_as_sdf_field_source.primary", timeout_seconds=20.0)
        viewer.wait_for_control("sdf_pack.radius.primary", timeout_seconds=20.0)

        baseline = viewer.wait_for_report(timeout_seconds=30.0)
        baseline_hash = baseline.get("rendered_frame_hash")
        assert baseline.get("lens_sdf_field_source") == "mask_derived_lens_sdf", baseline
        assert baseline.get("lens_sdf_valid") is True, baseline
        assert _sdf_pack_report(baseline).get("use_as_sdf_field_source") is False

        authored = viewer.set_control_value(
            "sdf_pack.use_as_sdf_field_source.primary",
            1.0,
            timeout_seconds=40.0,
        )
        authored_hash = authored.get("rendered_frame_hash")
        assert authored.get("lens_sdf_field_source") == "authored_sdf_pack", authored
        assert authored.get("lens_sdf_field_source_pack_id") == "runtime_field_source_circle", authored
        assert authored.get("lens_sdf_valid") is True, authored
        assert authored.get("lens_sdf_field_source_error") is None, authored
        assert authored.get("lens_sdf_backend_used") in {"cuda_sample", "cpu_reference"}, authored
        assert authored.get("lens_sdf_pack_backend_used") in {"cuda_sample", "cpu_reference"}, authored
        assert authored.get("set_value_consumed") is True, authored
        assert authored_hash != baseline_hash, authored
        assert _sdf_pack_report(authored).get("use_as_sdf_field_source") is True

        edited = viewer.set_control_value("sdf_pack.radius.primary", 0.85, timeout_seconds=40.0)
        assert edited.get("lens_sdf_field_source") == "authored_sdf_pack", edited
        assert edited.get("rendered_frame_hash") != authored_hash, edited
        assert viewer.launch_count == 1


def test_authored_sdf_pack_field_reports_overlay_source_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer UI automation is Windows-only")

    exe_path = active_runtime_exe()
    state_path = _sdf_source_state(exe_path, tmp_path)
    state = json.loads(state_path.read_text(encoding="utf-8"))
    lens = state.setdefault("lens", {})
    assert isinstance(lens, dict)
    lens["sdf_overlay_mode"] = "field_debug"
    lens["sdf_overlay_opacity"] = 0.65
    state_path = write_state_bundle(tmp_path / "sdf_pack_overlay_state", state)
    pack_path = tmp_path / "runtime_field_source_circle.sdf_pack.json"
    pack_path.write_text(json.dumps(_sdf_pack(), indent=2), encoding="utf-8")

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "sdf_pack_overlay_report.json",
        command_path=tmp_path / "sdf_pack_overlay_command.json",
        extra_args=[
            "--open-sdf-pack-panel",
            "--sdf-pack-json",
            str(pack_path),
        ],
    ) as viewer:
        viewer.wait_for_control("sdf_pack.use_as_sdf_field_source.primary", timeout_seconds=20.0)
        authored = viewer.set_control_value(
            "sdf_pack.use_as_sdf_field_source.primary",
            1.0,
            timeout_seconds=40.0,
        )
        assert authored.get("lens_sdf_field_source") == "authored_sdf_pack", authored
        assert authored.get("lens_sdf_overlay_active") is True, authored
        assert authored.get("lens_sdf_overlay_mode") == "field_debug", authored
        assert authored.get("lens_sdf_valid") is True, authored
        assert authored.get("lens_sdf_width", 0) > 0, authored
        assert authored.get("lens_sdf_height", 0) > 0, authored
        assert viewer.launch_count == 1


def test_authored_sdf_pack_field_replays_from_state_json_headless(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    state_path = _sdf_source_state(exe_path, tmp_path)
    state = json.loads(state_path.read_text(encoding="utf-8"))
    pack = _sdf_pack()
    state["sdf_pack"] = {
        "open": True,
        "use_as_sdf_field_source": True,
        "pack_path": "",
        "pack_json": json.dumps(pack),
        "backend_preference": "auto",
        "params": {"radius": 0.72, "offset_x": 0.15},
    }
    pack_state_path = write_state_bundle(tmp_path / "authored_pack_replay_seed", state)

    first = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(pack_state_path),
        "--capture-diagnostic",
    )
    first_state = first["state"]
    assert isinstance(first_state, dict)
    first_sdf_pack = first_state.get("sdf_pack")
    assert isinstance(first_sdf_pack, dict), first_state
    assert first_sdf_pack.get("use_as_sdf_field_source") is True, first_sdf_pack
    assert first_sdf_pack.get("params", {}).get("radius") == pytest.approx(0.72), first_sdf_pack

    replay_state_path = write_state_bundle(tmp_path / "authored_pack_replay_emitted", first_state)
    replay = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_state_path),
        "--capture-diagnostic",
    )
    assert replay["frame_hash"] == first["frame_hash"]
