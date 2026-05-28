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
    write_state_bundle,
)


def _sdf_pack() -> dict[str, object]:
    return {
        "schema": 1,
        "pack_id": "runtime_viewer_circle",
        "name": "Runtime Viewer Circle",
        "kind": "sdf_scene_2d",
        "params": [
            {"id": "radius", "type": "float", "default": 0.35, "range": [0.1, 1.0]},
            {"id": "offset_x", "type": "float", "default": 0.0, "range": [-1.0, 1.0]},
        ],
        "controls": [
            {"param": "radius", "label": "Radius", "ui_min": 0.1, "ui_max": 0.8},
            {"param": "offset_x", "label": "Offset X", "ui_min": -0.5, "ui_max": 0.5},
        ],
        "region": {"center": [0.0, 0.0], "half_height": 1.0},
        "ast": {
            "op": "circle",
            "center": [{"param": "offset_x"}, 0.0],
            "radius": {"param": "radius"},
        },
    }


def _sdf_pack_report(payload: dict[str, Any]) -> dict[str, Any]:
    report = payload.get("sdf_pack_viewer")
    assert isinstance(report, dict), payload
    return report


def _control_by_id(payload: dict[str, Any], control_id: str) -> dict[str, Any]:
    report = _sdf_pack_report(payload)
    controls = report.get("controls")
    assert isinstance(controls, list), report
    for control in controls:
        assert isinstance(control, dict), control
        if control.get("control_id") == control_id:
            return control
    raise AssertionError(f"control {control_id!r} missing from SDF pack report: {controls!r}")


@pytest.fixture(scope="module")
def sdf_pack_viewer_proof(tmp_path_factory: pytest.TempPathFactory) -> dict[str, Any]:
    if sys.platform != "win32":
        pytest.skip("viewer UI automation is Windows-only")

    tmp_path = tmp_path_factory.mktemp("sdf_pack_viewer")
    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "julia", "--width", "320", "--height", "240"
    )
    state_path = write_state_bundle(tmp_path / "sdf_pack_viewer_state", json.loads(json.dumps(neutral_capture["state"])))
    pack_path = tmp_path / "runtime_sdf_pack.json"
    pack_path.write_text(json.dumps(_sdf_pack(), indent=2), encoding="utf-8")

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "sdf_pack_viewer_report.json",
        command_path=tmp_path / "sdf_pack_viewer_command.json",
        extra_args=[
            "--open-sdf-pack-panel",
            "--sdf-pack-json",
            str(pack_path),
        ],
    ) as viewer:
        viewer.wait_for_control("sdf_pack.radius.primary", timeout_seconds=15.0)
        baseline_payload = viewer.wait_for_report(timeout_seconds=15.0)
        baseline_report = _sdf_pack_report(baseline_payload)
        baseline_hash = baseline_report.get("preview_field_hash")
        assert isinstance(baseline_hash, str) and baseline_hash.startswith("fnv1a64:"), baseline_report

        edited_payload = viewer.set_control_value("sdf_pack.radius.primary", 0.7, timeout_seconds=15.0)
        edited_report = _sdf_pack_report(edited_payload)
        edited_hash = edited_report.get("preview_field_hash")
        assert isinstance(edited_hash, str) and edited_hash.startswith("fnv1a64:"), edited_report

        viewer.wait_for_control("sdf_pack.reset_defaults", timeout_seconds=15.0)
        reset_payload = viewer.click_control("sdf_pack.reset_defaults", timeout_seconds=15.0)
        reset_hash = _sdf_pack_report(reset_payload).get("preview_field_hash")

        return {
            "baseline_payload": baseline_payload,
            "edited_payload": edited_payload,
            "reset_payload": reset_payload,
            "baseline_hash": baseline_hash,
            "edited_hash": edited_hash,
            "reset_hash": reset_hash,
            "launch_count": viewer.launch_count,
        }


def test_sdf_pack_viewer_no_mouse_set_value_changes_field_preview(
    sdf_pack_viewer_proof: dict[str, Any],
) -> None:
    baseline = _sdf_pack_report(sdf_pack_viewer_proof["baseline_payload"])
    edited = _sdf_pack_report(sdf_pack_viewer_proof["edited_payload"])

    assert baseline.get("have_pack") is True
    assert baseline.get("pack_id") == "runtime_viewer_circle"
    assert baseline.get("preview_ok") is True
    assert edited.get("preview_ok") is True
    assert sdf_pack_viewer_proof["edited_payload"].get("set_value_consumed") is True
    assert sdf_pack_viewer_proof["edited_hash"] != sdf_pack_viewer_proof["baseline_hash"]
    assert sdf_pack_viewer_proof["launch_count"] == 1


def test_sdf_pack_viewer_reports_controls_and_reset_defaults_no_mouse(
    sdf_pack_viewer_proof: dict[str, Any],
) -> None:
    baseline_payload = sdf_pack_viewer_proof["baseline_payload"]
    edited_payload = sdf_pack_viewer_proof["edited_payload"]
    reset_payload = sdf_pack_viewer_proof["reset_payload"]

    baseline_radius = _control_by_id(baseline_payload, "sdf_pack.radius.primary")
    assert baseline_radius.get("param") == "radius"
    assert baseline_radius.get("label") == "Radius"
    assert baseline_radius.get("value") == pytest.approx(0.35)
    assert baseline_radius.get("default") == pytest.approx(0.35)
    assert baseline_radius.get("min") == pytest.approx(0.1)
    assert baseline_radius.get("max") == pytest.approx(0.8)

    edited_radius = _control_by_id(edited_payload, "sdf_pack.radius.primary")
    assert edited_radius.get("value") == pytest.approx(0.7)

    reset_radius = _control_by_id(reset_payload, "sdf_pack.radius.primary")
    assert reset_payload.get("click_consumed") is True
    assert reset_radius.get("value") == pytest.approx(0.35)
    assert sdf_pack_viewer_proof["reset_hash"] == sdf_pack_viewer_proof["baseline_hash"]
    assert sdf_pack_viewer_proof["launch_count"] == 1
