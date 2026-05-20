from __future__ import annotations

import json
import math
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


def _require_rendered_frame_hash(payload: dict[str, object]) -> str:
    assert payload.get("rendered_frame_ready") is True, (
        f"automation report did not include a ready rendered frame: {payload!r}"
    )
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), (
        f"automation report did not include a renderer-owned frame hash: {payload!r}"
    )
    return frame_hash


def _configure_view(state: dict[str, object], *, center_x: float, center_y: float, zoom: float) -> None:
    view = state["view"]
    assert isinstance(view, dict)
    view["center_x"] = center_x
    view["center_y"] = center_y
    view["center_hp_x"] = center_x
    view["center_hp_y"] = center_y
    view["zoom"] = zoom
    view["log2_zoom"] = math.log2(max(zoom, 1.0e-30))
    view["auto_max_iter"] = False


def _configure_smooth_escape_color(state: dict[str, object], *, max_iter: int = 64, signal: str = "smooth_escape") -> None:
    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "max_iter": max_iter,
            "coloring_mode": "smooth_escape",
            "color_signal": signal,
            "color_shape": "identity",
            "color_palette": "cyclic_escape",
            "color_grading": "escape_default",
            "color_smooth_escape_scale": 1.0,
            "color_smooth_escape_bias": 0.0,
            "color_heatmap_cycle_scale": 0.25,
            "color_heatmap_saturation": 1.0,
            "color_root_proximity_scale": 0.5,
            "color_root_proximity_bias": 0.0,
        }
    )


def _phase8_followup_state(fractal_type: str, control_id: str) -> dict[str, object]:
    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", fractal_type, "--width", "320", "--height", "240"
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    params = state["params"]
    assert isinstance(params, dict)
    if fractal_type == "nova":
        _configure_view(state, center_x=-0.2031297467797688, center_y=0.3483956144889109, zoom=16.0)
        params.update(
            {
                "max_iter": 40,
                "poly_kind": 2,
                "poly_coeffs": [-1.0, 0.0, 0.0, 1.0, 0.0],
                "nova_alpha": 0.1,
                "coloring_mode": "smooth_escape",
                "color_signal": "smooth_escape",
                "color_shape": "identity",
                "color_palette": "cyclic_escape",
                "color_grading": "escape_default",
            }
        )
    elif fractal_type == "explaino_y":
        _configure_view(state, center_x=0.25, center_y=-0.1, zoom=2.0)
        _configure_smooth_escape_color(state)
        params["epsilon"] = 1.0e-6
    elif fractal_type == "explaino_nova":
        _configure_smooth_escape_color(state)
        params.update({"poly_kind": 2, "poly_coeffs": [-1.0, 0.0, 0.0, 1.0, 0.0], "nova_alpha": 0.1})
    elif fractal_type == "explaino_transcendental":
        _configure_smooth_escape_color(state, signal="root_proximity")
    elif fractal_type in {"explaino_joy", "explaino_tension"}:
        _configure_view(state, center_x=1.0, center_y=0.0, zoom=2048.0)
        _configure_smooth_escape_color(state)
        params["epsilon"] = 1.0e-6
    elif fractal_type == "explaino_projection_and_flow":
        _configure_smooth_escape_color(state)
        params["epsilon"] = 1.0e-6
    elif fractal_type == "explaino_lambda":
        _configure_smooth_escape_color(state)
    return state


def test_persistent_runtime_viewer_batches_set_value_proofs_in_one_process(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "julia", "--width", "320", "--height", "240"
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    state_path = write_state_bundle(tmp_path / "julia_persistent", state)
    real_control = "fractal_control.julia_c_real.primary"
    imag_control = "fractal_control.julia_c_imag.primary"

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "persistent_report.json",
        command_path=tmp_path / "persistent_command.json",
    ) as viewer:
        viewer.wait_for_control(real_control)
        viewer.wait_for_control(imag_control)
        baseline_hash = _require_rendered_frame_hash(viewer.wait_for_report())

        real_payload = viewer.set_control_value(real_control, 0.285)
        real_hash = _require_rendered_frame_hash(real_payload)

        imag_payload = viewer.set_control_value(imag_control, 0.01)
        imag_hash = _require_rendered_frame_hash(imag_payload)

        assert viewer.launch_count == 1
        assert real_payload.get("ui_automation_command_sequence") == 1
        assert imag_payload.get("ui_automation_command_sequence") == 2

    assert real_hash != baseline_hash, "persistent viewer set-value for julia_c_real should change the rendered frame"
    assert imag_hash != real_hash, "persistent viewer second set-value should update the same running viewer"


def test_persistent_runtime_viewer_loads_multiple_fractal_states_in_one_process(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    julia_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "julia", "--width", "320", "--height", "240"
    )
    phoenix_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "phoenix", "--width", "320", "--height", "240"
    )
    julia_state_path = write_state_bundle(tmp_path / "julia_loaded", json.loads(json.dumps(julia_capture["state"])))
    phoenix_state_path = write_state_bundle(tmp_path / "phoenix_loaded", json.loads(json.dumps(phoenix_capture["state"])))

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=julia_state_path,
        report_path=tmp_path / "persistent_load_report.json",
        command_path=tmp_path / "persistent_load_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.julia_c_real.primary")
        julia_hash = _require_rendered_frame_hash(viewer.wait_for_report())

        loaded_payload = viewer.load_state_json(phoenix_state_path, expected_fractal_type="phoenix")
        phoenix_hash = _require_rendered_frame_hash(loaded_payload)
        viewer.wait_for_control("fractal_control.phoenix_p_real.primary")

        edited_payload = viewer.set_control_value("fractal_control.phoenix_p_real.primary", 0.15)
        edited_hash = _require_rendered_frame_hash(edited_payload)

        assert viewer.launch_count == 1
        assert loaded_payload.get("ui_automation_command_sequence") == 1
        assert edited_payload.get("ui_automation_command_sequence") == 2

    assert phoenix_hash != julia_hash, "persistent load-state command should replace the active fractal without relaunching"
    assert edited_hash != phoenix_hash, "set-value after persistent load-state should affect the newly loaded fractal"

def test_persistent_runtime_viewer_sets_enum_controls_in_one_process(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "nova", "--width", "320", "--height", "240"
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    view = state["view"]
    assert isinstance(view, dict)
    view["center_x"] = -0.2031297467797688
    view["center_y"] = 0.3483956144889109
    view["center_hp_x"] = -0.2031297467797688
    view["center_hp_y"] = 0.3483956144889109
    view["zoom"] = 16.0
    view["log2_zoom"] = 4.0
    view["auto_max_iter"] = False
    params = state["params"]
    assert isinstance(params, dict)
    params["max_iter"] = 40
    params["poly_kind"] = 2
    params["poly_coeffs"] = [-1.0, 0.0, 0.0, 1.0, 0.0]
    params["nova_alpha"] = 0.1
    params["coloring_mode"] = "smooth_escape"
    params["color_signal"] = "smooth_escape"
    params["color_shape"] = "identity"
    params["color_palette"] = "cyclic_escape"
    params["color_grading"] = "escape_default"
    state_path = write_state_bundle(tmp_path / "nova_enum_loaded", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "persistent_enum_report.json",
        command_path=tmp_path / "persistent_enum_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.poly_kind.primary")
        baseline_hash = _require_rendered_frame_hash(viewer.wait_for_report())
        edited_payload = viewer.set_enum_id("fractal.params.poly_kind", "z4_minus_1", expected_fractal_type="nova")
        edited_hash = _require_rendered_frame_hash(edited_payload)
        assert viewer.launch_count == 1
        assert edited_payload.get("ui_automation_command_sequence") == 1

    assert edited_hash != baseline_hash, "persistent enum automation for Nova poly_kind should change the rendered frame"

def test_persistent_runtime_viewer_phase8_followup_controls_change_frames_in_one_process(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    cases = [
        ("nova", "epsilon", 0.01),
        ("explaino_y", "epsilon", 0.01),
        ("explaino_nova", "epsilon", 0.01),
        ("explaino_transcendental", "explaino_seed", 0.5),
        ("explaino_lambda", "explaino_seed", 0.5),
        ("explaino_joy", "epsilon", 0.001),
        ("explaino_tension", "epsilon", 0.001),
        ("explaino_projection_and_flow", "epsilon", 0.01),
    ]
    state_paths = {
        f"{fractal_type}_{control_id}": write_state_bundle(
            tmp_path / f"phase8_{fractal_type}_{control_id}",
            _phase8_followup_state(fractal_type, control_id),
        )
        for fractal_type, control_id, _value in cases
    }

    first_fractal, first_control, _first_value = cases[0]
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_paths[f"{first_fractal}_{first_control}"],
        report_path=tmp_path / "persistent_phase8_followup_report.json",
        command_path=tmp_path / "persistent_phase8_followup_command.json",
    ) as viewer:
        for fractal_type, control_id, value in cases:
            payload = viewer.load_state_json(
                state_paths[f"{fractal_type}_{control_id}"],
                expected_fractal_type=fractal_type,
                timeout_seconds=15.0,
            )
            baseline_hash = _require_rendered_frame_hash(payload)
            full_control_id = f"fractal_control.{control_id}.primary"
            viewer.wait_for_control(full_control_id, timeout_seconds=15.0)
            edited_payload = viewer.set_control_value(full_control_id, value, timeout_seconds=15.0)
            edited_hash = _require_rendered_frame_hash(edited_payload)
            assert edited_hash != baseline_hash, (
                f"Phase 8 no-mouse follow-up control should change the rendered frame: "
                f"fractal={fractal_type} control={control_id} "
                f"baseline_hash={baseline_hash} edited_hash={edited_hash}"
            )
        assert viewer.launch_count == 1
