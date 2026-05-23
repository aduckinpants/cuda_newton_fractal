import json
import math
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


def _capture_state(exe_path: Path, fractal_type: str) -> dict[str, object]:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        fractal_type,
        "--width",
        "320",
        "--height",
        "240",
    )
    return json.loads(json.dumps(capture["state"]))


def test_batch1_new_formula_controls_change_live_output_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer automation is Windows-only")

    exe_path = active_runtime_exe()
    spider_state = _capture_state(exe_path, "spider")
    spider_state["params"]["spider_feedback"] = 0.5
    spider_state["params"]["max_iter"] = 700
    spider_state_path = write_state_bundle(tmp_path / "spider_feedback", spider_state)

    rational_state = _capture_state(exe_path, "explaino_rational_escape")
    rational_state["params"]["explaino_rational_escape_denominator_power"] = 3
    rational_state["params"]["max_iter"] = 800
    rational_state_path = write_state_bundle(tmp_path / "rational_escape_denominator_power", rational_state)

    collatz_state = _capture_state(exe_path, "collatz")
    collatz_state["params"]["collatz_transition_strength"] = 1.0
    collatz_state["params"]["max_iter"] = 500
    collatz_state["params"]["coloring_mode"] = "smooth_escape"
    collatz_state["params"]["color_signal"] = "smooth_escape"
    collatz_state["params"]["color_palette"] = "cyclic_escape"
    collatz_state["params"]["color_grading"] = "escape_default"
    collatz_state_path = write_state_bundle(tmp_path / "collatz_transition_strength", collatz_state)

    phoenix_state = _capture_state(exe_path, "phoenix")
    phoenix_state["params"]["phoenix_p_real"] = 0.5667
    phoenix_state["params"]["phoenix_p_imag"] = 0.0
    phoenix_state["params"]["max_iter"] = 900
    phoenix_state_path = write_state_bundle(tmp_path / "phoenix_parameterization", phoenix_state)

    burning_ship_state = _capture_state(exe_path, "burning_ship")
    burning_ship_state["params"]["burning_ship_fold_mix"] = 1.0
    burning_ship_state["params"]["max_iter"] = 900
    burning_ship_state_path = write_state_bundle(tmp_path / "burning_ship_fold_mix", burning_ship_state)

    celtic_state = _capture_state(exe_path, "celtic_mandelbrot")
    celtic_state["params"]["celtic_abs_mix"] = 1.0
    celtic_state["params"]["max_iter"] = 900
    celtic_state_path = write_state_bundle(tmp_path / "celtic_abs_mix", celtic_state)

    perpendicular_state = _capture_state(exe_path, "perpendicular_burning_ship")
    perpendicular_state["params"]["perpendicular_fold_mix"] = 1.0
    perpendicular_state["params"]["max_iter"] = 900
    perpendicular_state_path = write_state_bundle(tmp_path / "perpendicular_fold_mix", perpendicular_state)

    nova_state = _capture_state(exe_path, "nova")
    nova_state["view"]["center_x"] = -0.5
    nova_state["view"]["center_y"] = 0.0
    nova_state["view"]["center_hp_x"] = -0.5
    nova_state["view"]["center_hp_y"] = 0.0
    nova_state["view"]["zoom"] = 800.0
    nova_state["view"]["log2_zoom"] = math.log2(800.0)
    nova_state["params"]["epsilon"] = 1.0e-12
    nova_state["params"]["max_iter"] = 1200
    nova_state["params"]["coloring_mode"] = "iteration_count"
    nova_state["params"]["color_signal"] = "iteration_count"
    nova_state["params"]["color_palette"] = "cyclic_escape"
    nova_state["params"]["color_grading"] = "escape_default"
    nova_state_path = write_state_bundle(tmp_path / "nova_epsilon_near_threshold", nova_state)

    with runtime_automation_lock(timeout_seconds=20.0):
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=spider_state_path,
            report_path=tmp_path / "parameter_functionality_report.json",
            command_path=tmp_path / "parameter_functionality_command.json",
        ) as viewer:
            spider_baseline = viewer.wait_for_report(timeout_seconds=15.0)
            spider_baseline_hash = spider_baseline.get("rendered_frame_hash")
            assert spider_baseline.get("current_fractal_type") == "spider"
            assert isinstance(spider_baseline_hash, str) and spider_baseline_hash

            spider_control = "fractal_control.spider_feedback.primary"
            viewer.wait_for_control(spider_control, timeout_seconds=15.0)
            spider_edited = viewer.set_control_value(spider_control, 0.9, timeout_seconds=15.0)
            spider_edited_hash = spider_edited.get("rendered_frame_hash")
            assert spider_edited.get("current_fractal_type") == "spider"
            assert spider_edited.get("set_value_consumed") is True
            assert isinstance(spider_edited_hash, str) and spider_edited_hash
            assert spider_edited_hash != spider_baseline_hash

            rational_baseline = viewer.load_state_json(
                rational_state_path,
                expected_fractal_type="explaino_rational_escape",
                timeout_seconds=15.0,
            )
            rational_baseline_hash = rational_baseline.get("rendered_frame_hash")
            assert isinstance(rational_baseline_hash, str) and rational_baseline_hash

            rational_control = "fractal_control.explaino_rational_escape_denominator_power.primary"
            viewer.wait_for_control(rational_control, timeout_seconds=15.0)
            rational_edited = viewer.set_control_value(rational_control, 5.0, timeout_seconds=15.0)
            rational_edited_hash = rational_edited.get("rendered_frame_hash")
            assert rational_edited.get("current_fractal_type") == "explaino_rational_escape"
            assert rational_edited.get("set_value_consumed") is True
            assert isinstance(rational_edited_hash, str) and rational_edited_hash
            assert rational_edited_hash != rational_baseline_hash

            collatz_baseline = viewer.load_state_json(
                collatz_state_path,
                expected_fractal_type="collatz",
                timeout_seconds=15.0,
            )
            collatz_baseline_hash = collatz_baseline.get("rendered_frame_hash")
            assert isinstance(collatz_baseline_hash, str) and collatz_baseline_hash

            collatz_control = "fractal_control.collatz_transition_strength.primary"
            viewer.wait_for_control(collatz_control, timeout_seconds=15.0)
            collatz_edited = viewer.set_control_value(collatz_control, 0.35, timeout_seconds=15.0)
            collatz_edited_hash = collatz_edited.get("rendered_frame_hash")
            assert collatz_edited.get("current_fractal_type") == "collatz"
            assert collatz_edited.get("set_value_consumed") is True
            assert isinstance(collatz_edited_hash, str) and collatz_edited_hash
            assert collatz_edited_hash != collatz_baseline_hash

            phoenix_baseline = viewer.load_state_json(
                phoenix_state_path,
                expected_fractal_type="phoenix",
                timeout_seconds=15.0,
            )
            phoenix_baseline_hash = phoenix_baseline.get("rendered_frame_hash")
            assert isinstance(phoenix_baseline_hash, str) and phoenix_baseline_hash

            phoenix_real_control = "fractal_control.phoenix_p_real.primary"
            viewer.wait_for_control(phoenix_real_control, timeout_seconds=15.0)
            phoenix_real_edited = viewer.set_control_value(phoenix_real_control, 0.25, timeout_seconds=15.0)
            phoenix_real_hash = phoenix_real_edited.get("rendered_frame_hash")
            assert phoenix_real_edited.get("current_fractal_type") == "phoenix"
            assert phoenix_real_edited.get("set_value_consumed") is True
            assert isinstance(phoenix_real_hash, str) and phoenix_real_hash
            assert phoenix_real_hash != phoenix_baseline_hash

            phoenix_imag_control = "fractal_control.phoenix_p_imag.primary"
            viewer.wait_for_control(phoenix_imag_control, timeout_seconds=15.0)
            phoenix_imag_edited = viewer.set_control_value(phoenix_imag_control, 0.35, timeout_seconds=15.0)
            phoenix_imag_hash = phoenix_imag_edited.get("rendered_frame_hash")
            assert phoenix_imag_edited.get("current_fractal_type") == "phoenix"
            assert phoenix_imag_edited.get("set_value_consumed") is True
            assert isinstance(phoenix_imag_hash, str) and phoenix_imag_hash
            assert phoenix_imag_hash != phoenix_real_hash

            burning_ship_baseline = viewer.load_state_json(
                burning_ship_state_path,
                expected_fractal_type="burning_ship",
                timeout_seconds=15.0,
            )
            burning_ship_hash = burning_ship_baseline.get("rendered_frame_hash")
            assert isinstance(burning_ship_hash, str) and burning_ship_hash

            burning_ship_control = "fractal_control.burning_ship_fold_mix.primary"
            viewer.wait_for_control(burning_ship_control, timeout_seconds=15.0)
            burning_ship_edited = viewer.set_control_value(burning_ship_control, 0.25, timeout_seconds=15.0)
            burning_ship_edited_hash = burning_ship_edited.get("rendered_frame_hash")
            assert burning_ship_edited.get("current_fractal_type") == "burning_ship"
            assert burning_ship_edited.get("set_value_consumed") is True
            assert isinstance(burning_ship_edited_hash, str) and burning_ship_edited_hash
            assert burning_ship_edited_hash != burning_ship_hash

            celtic_baseline = viewer.load_state_json(
                celtic_state_path,
                expected_fractal_type="celtic_mandelbrot",
                timeout_seconds=15.0,
            )
            celtic_hash = celtic_baseline.get("rendered_frame_hash")
            assert isinstance(celtic_hash, str) and celtic_hash

            celtic_control = "fractal_control.celtic_abs_mix.primary"
            viewer.wait_for_control(celtic_control, timeout_seconds=15.0)
            celtic_edited = viewer.set_control_value(celtic_control, 0.25, timeout_seconds=15.0)
            celtic_edited_hash = celtic_edited.get("rendered_frame_hash")
            assert celtic_edited.get("current_fractal_type") == "celtic_mandelbrot"
            assert celtic_edited.get("set_value_consumed") is True
            assert isinstance(celtic_edited_hash, str) and celtic_edited_hash
            assert celtic_edited_hash != celtic_hash

            perpendicular_baseline = viewer.load_state_json(
                perpendicular_state_path,
                expected_fractal_type="perpendicular_burning_ship",
                timeout_seconds=15.0,
            )
            perpendicular_hash = perpendicular_baseline.get("rendered_frame_hash")
            assert isinstance(perpendicular_hash, str) and perpendicular_hash

            perpendicular_control = "fractal_control.perpendicular_fold_mix.primary"
            viewer.wait_for_control(perpendicular_control, timeout_seconds=15.0)
            perpendicular_edited = viewer.set_control_value(perpendicular_control, 0.25, timeout_seconds=15.0)
            perpendicular_edited_hash = perpendicular_edited.get("rendered_frame_hash")
            assert perpendicular_edited.get("current_fractal_type") == "perpendicular_burning_ship"
            assert perpendicular_edited.get("set_value_consumed") is True
            assert isinstance(perpendicular_edited_hash, str) and perpendicular_edited_hash
            assert perpendicular_edited_hash != perpendicular_hash

            nova_baseline = viewer.load_state_json(
                nova_state_path,
                expected_fractal_type="nova",
                timeout_seconds=15.0,
            )
            nova_baseline_hash = nova_baseline.get("rendered_frame_hash")
            assert isinstance(nova_baseline_hash, str) and nova_baseline_hash

            epsilon_control = "fractal_control.epsilon.primary"
            viewer.wait_for_control(epsilon_control, timeout_seconds=15.0)
            nova_edited = viewer.set_control_value(epsilon_control, 0.01, timeout_seconds=15.0)
            nova_edited_hash = nova_edited.get("rendered_frame_hash")
            assert nova_edited.get("current_fractal_type") == "nova"
            assert nova_edited.get("set_value_consumed") is True
            assert isinstance(nova_edited_hash, str) and nova_edited_hash
            assert nova_edited_hash != nova_baseline_hash
