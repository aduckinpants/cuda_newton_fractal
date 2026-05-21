from __future__ import annotations

import json
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


def _interactive_pack() -> dict[str, object]:
    return {
        "schema_version": 1,
        "pack_id": "interactive_quadratic",
        "name": "Interactive Quadratic",
        "formula": {
            "kind": "iterate_map",
            "iteration_param": "steps",
            "ast": {
                "op": "add",
                "args": [
                    {"op": "pow_int", "base": {"op": "var_z"}, "exponent": 2},
                    {"op": "complex_param", "name": "c"},
                ],
            },
        },
        "params": {"steps": 18.0, "c_real": -0.75, "c_imag": 0.1},
        "controls": [
            {"id": "steps", "param": "steps", "label": "Steps", "min": 1.0, "max": 80.0, "step": 1.0, "default": 18.0},
            {"id": "c_real", "param": "c_real", "label": "C Real", "min": -2.0, "max": 2.0, "step": 0.01, "default": -0.75},
        ],
        "epsilon": 1e-9,
        "escape_radius": 1000.0,
        "region": {"center_x": 0.0, "center_y": 0.0, "span_x": 2.0, "span_y": 2.0, "grid_width": 8, "grid_height": 6},
    }


def _preview_hashes(payload: dict[str, object]) -> tuple[str, str]:
    workbench = payload.get("equation_pack_workbench")
    assert isinstance(workbench, dict), payload
    assert workbench.get("preview_ok") is True, workbench
    assert workbench.get("preview_backend_used") == "cuda", workbench
    assert workbench.get("preview_image_width") == 8, workbench
    assert workbench.get("preview_image_height") == 6, workbench
    result_hash = workbench.get("preview_result_hash")
    assert isinstance(result_hash, str) and result_hash.startswith("fnv1a64:"), workbench
    image_hash = workbench.get("preview_image_hash")
    assert isinstance(image_hash, str) and image_hash.startswith("fnv1a64:"), workbench
    return result_hash, image_hash


def test_equation_pack_workbench_no_mouse_set_value_changes_preview(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer UI automation is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "julia", "--width", "320", "--height", "240"
    )
    state_path = write_state_bundle(tmp_path / "interactive_ui_state", json.loads(json.dumps(neutral_capture["state"])))
    pack_path = tmp_path / "interactive_pack.json"
    pack_path.write_text(json.dumps(_interactive_pack(), indent=2), encoding="utf-8")

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "equation_pack_ui_report.json",
        command_path=tmp_path / "equation_pack_ui_command.json",
        extra_args=[
            "--open-equation-pack-workbench",
            "--equation-pack-workbench-pack-json",
            str(pack_path),
        ],
    ) as viewer:
        viewer.wait_for_control("equation_pack.c_real.primary", timeout_seconds=15.0)
        viewer.wait_for_control("equation_pack.preview_canvas", timeout_seconds=15.0)
        baseline_payload = viewer.wait_for_report(timeout_seconds=15.0)
        baseline_result_hash, baseline_image_hash = _preview_hashes(baseline_payload)

        edited_payload = viewer.set_control_value("equation_pack.c_real.primary", 0.35, timeout_seconds=15.0)
        edited_result_hash, edited_image_hash = _preview_hashes(edited_payload)

        assert viewer.launch_count == 1
        assert edited_payload.get("set_value_consumed") is True
        assert edited_result_hash != baseline_result_hash
        assert edited_image_hash != baseline_image_hash
