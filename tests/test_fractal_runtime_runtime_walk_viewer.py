from __future__ import annotations

import ctypes
from ctypes import wintypes
import json
import subprocess
import sys
import tempfile
import time
from pathlib import Path

import pytest

from tests.runtime_harness import (
    HeadlessLoadedStateScenario as _HeadlessLoadedStateScenario,
    RUNTIME_DIR,
    WM_CLOSE,
    active_runtime_exe as _active_runtime_exe,
    capture_explaino_runtime_baseline as _capture_explaino_runtime_baseline,
    capture_ready_window_pixels as _capture_ready_window_pixels,
    run_headless_capture as _run_headless_capture,
    find_window_for_pid as _find_window_for_pid,
    focus_window as _focus_window,
    run_headless_loaded_state_scenario as _run_headless_loaded_state_scenario,
    wait_for_ui_automation_rect,
    write_state_bundle as _write_state_bundle,
    wait_for_window as _wait_for_window,
)

from tests.test_fractal_runtime_explaino_escape_variants import _with_explaino_repeat_heatmap_draft_state


REPO_ROOT = Path(__file__).resolve().parents[1]
REAL_FITS_PATH = Path(
    r"D:\salt-output\results\godel_fits_entropy_campaign\core_round_20260407\core\fits\godel_g1_long\frames_delta_stack.fits"
)

WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101
VK_SPACE = 0x20
VK_RIGHT = 0x27
COLOR_PIPELINE_SCALE_CONTROL_ID = "color_pipeline.source.smooth_escape_ramp.signal.scale.primary"



def _describe_explaino_axis_registry(exe_path: Path) -> list[dict[str, object]]:
    result = subprocess.run(
        [str(exe_path), "--describe-explaino-axis-registry"],
        cwd=str(RUNTIME_DIR),
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert result.returncode == 0, result.stderr
    payload = json.loads(result.stdout)
    axes = payload.get("axes")
    assert isinstance(axes, list) and len(axes) == int(payload.get("count", -1))
    for axis in axes:
        assert isinstance(axis, dict)
        assert isinstance(axis.get("axis_id"), str) and axis["axis_id"]
        assert isinstance(axis.get("carrier_fractal_type"), str) and axis["carrier_fractal_type"]
        assert isinstance(axis.get("control_id"), str) and axis["control_id"].startswith("fractal_control.")
    return axes


def _axis_active_value(axis: dict[str, object]) -> float:
    default_value = float(axis.get("default_value", 0.0))
    return default_value if abs(default_value) > 1.0e-9 else 0.35


def _wait_for_ui_automation_set_value(
    report_path: Path,
    control_id: str,
    *,
    timeout_seconds: float = 10.0,
) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        payload = _load_ui_automation_report(report_path)
        if payload is None:
            time.sleep(0.05)
            continue
        last_payload = payload
        if payload.get("requested_set_control_id") != control_id:
            time.sleep(0.05)
            continue
        set_error = payload.get("set_value_error")
        assert not set_error, f"set-value automation failed for {control_id}: {set_error}"
        if payload.get("set_value_consumed") is True:
            return payload
        time.sleep(0.1)
    raise AssertionError(f"UI set-value automation never consumed {control_id}; last_payload={last_payload!r}")


def _wait_for_ui_automation_set_value_error(
    report_path: Path,
    control_id: str,
    *,
    timeout_seconds: float = 10.0,
) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        payload = _load_ui_automation_report(report_path)
        if payload is None:
            time.sleep(0.05)
            continue
        last_payload = payload
        if payload.get("requested_set_control_id") != control_id:
            time.sleep(0.05)
            continue
        set_error = payload.get("set_value_error")
        if isinstance(set_error, str) and set_error:
            assert payload.get("set_value_consumed") is False
            return payload
        time.sleep(0.1)
    raise AssertionError(f"UI set-value automation never failed closed for {control_id}; last_payload={last_payload!r}")


def _require_rendered_frame_hash(payload: dict[str, object]) -> str:
    assert payload.get("rendered_frame_ready") is True, f"automation report did not include a ready rendered frame: {payload!r}"
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), (
        f"automation report did not include a renderer-owned frame hash: {payload!r}"
    )
    assert isinstance(payload.get("rendered_frame_width"), int) and int(payload["rendered_frame_width"]) > 0
    assert isinstance(payload.get("rendered_frame_height"), int) and int(payload["rendered_frame_height"]) > 0
    return frame_hash


def _wait_for_ui_automation_rendered_frame(
    report_path: Path,
    *,
    timeout_seconds: float = 10.0,
) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        payload = _load_ui_automation_report(report_path)
        if payload is None:
            time.sleep(0.05)
            continue
        last_payload = payload
        if payload.get("rendered_frame_ready") is True and isinstance(payload.get("rendered_frame_hash"), str):
            return payload
        time.sleep(0.1)
    raise AssertionError(f"UI automation report never exposed a ready rendered frame; last_payload={last_payload!r}")


def _capture_controls_report_with_optional_set_value(
    exe_path: Path,
    state_path: Path,
    report_path: Path,
    control_id: str,
    set_value: float | None,
) -> dict[str, object]:
    args = [str(exe_path), "--load-state-json", str(state_path), "--ui-automation-report-json", str(report_path)]
    if set_value is not None:
        args.extend(["--ui-automation-set-control-value", f"{control_id}={set_value}"])
    proc = subprocess.Popen(args, cwd=str(RUNTIME_DIR))
    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        _focus_window(hwnd)
        wait_for_ui_automation_rect(report_path, control_id)
        if set_value is not None:
            return _wait_for_ui_automation_set_value(report_path, control_id)
        return _wait_for_ui_automation_rendered_frame(report_path)
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)


def _capture_runtime_walk_color_pipeline_report(
    exe_path: Path,
    request_path: Path,
    report_path: Path,
    set_value: float | None,
) -> dict[str, object]:
    args = [
        str(exe_path),
        "--load-runtime-walk-request-json",
        str(request_path),
        "--open-color-pipeline-window",
        "--ui-automation-report-json",
        str(report_path),
    ]
    if set_value is not None:
        args.extend(["--ui-automation-set-control-value", f"{COLOR_PIPELINE_SCALE_CONTROL_ID}={set_value}"])
    proc = subprocess.Popen(args, cwd=str(RUNTIME_DIR))
    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        _focus_window(hwnd)
        wait_for_ui_automation_rect(report_path, COLOR_PIPELINE_SCALE_CONTROL_ID)
        ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYDOWN, VK_SPACE, 0)
        ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYUP, VK_SPACE, 0)
        if set_value is not None:
            return _wait_for_ui_automation_set_value(report_path, COLOR_PIPELINE_SCALE_CONTROL_ID)
        return _wait_for_ui_automation_rendered_frame(report_path)
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)

def _write_state_json(path: Path) -> None:
    payload = {
        "state_version": 3,
        "fractal_type": "explaino",
        "view": {
            "center_x": 0.0,
            "center_y": 0.0,
            "zoom": 1.0,
            "rotation_degrees": 0.0,
            "center_hp_x": 0.0,
            "center_hp_y": 0.0,
            "log2_zoom": 0.0,
            "explaino_phase": 0.0,
            "explaino_seed_drift": 0.0,
            "explaino_seed_tween": True,
            "auto_max_iter": False,
            "auto_increment_seed": False,
            "explaino_seed_rate": 0.001,
            "explaino_phase_strength": 0.0,
        },
        "params": {
            "max_iter": 500,
            "epsilon": 1.0e-6,
            "exposure": 1.0,
            "poly_kind": 2,
            "coloring_mode": "smooth_escape",
            "nova_alpha": 0.5,
            "phoenix_p_real": 0.0,
            "phoenix_p_imag": 0.0,
            "multibrot_power": 3,
            "multibrot_power_float": 3.0,
            "lambda_real": 0.0,
            "lambda_imag": 0.0,
            "explaino_seed": 0.0,
            "explaino_seed_b": 0.0,
            "explaino_mix": 0.0,
            "explaino_warp_strength": 0.0,
            "explaino_root_spread": 0.0,
            "explaino_root_count": 0,
            "poly_coeffs": [-1, 0, 0, 1, 0],
            "color_signal": "smooth_escape",
            "color_shape": "identity",
            "color_palette": "cyclic_escape",
            "color_grading": "escape_default",
            "color_smooth_escape_scale": 1.0,
            "color_smooth_escape_bias": 0.0,
            "color_heatmap_cycle_scale": 1.0,
            "color_heatmap_saturation": 1.0,
            "color_contrast_lift_exposure": 1.0,
            "color_contrast_lift_saturation": 1.0,
            "color_saturation": 1.0,
            "color_contrast": 1.0,
            "color_tint_r": 1.0,
            "color_tint_g": 1.0,
            "color_tint_b": 1.0,
        },
        "render": {
            "width": 320,
            "height": 240,
            "interaction_debounce_ms": 250,
            "preview_target_fps": 30.0,
            "preview_min_scale": 0.5,
            "block_size": 256,
            "device_id": 0,
        },
    }
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _write_bundle_json(path: Path) -> None:
    payload = {
        "version": 1,
        "field_name": "mr_zipper_branch",
        "samples": [
            {"id": "loop_00", "t": 0.0, "channels": [0.82, 0.18, 0.54, 0.38, 0.76, 0.68, 0.88, 0.62, 0.34, 0.74, 0.26, 0.58, 0.72]},
            {"id": "loop_01", "t": 0.125, "channels": [0.76, 0.24, 0.79, 0.22, 0.84, 0.79, 0.93, 0.71, 0.28, 0.83, 0.20, 0.46, 0.86]},
            {"id": "loop_02", "t": 0.25, "channels": [0.51, 0.49, 0.88, 0.16, 0.77, 0.86, 0.89, 0.82, 0.24, 0.89, 0.23, 0.31, 0.92]},
            {"id": "loop_03", "t": 0.375, "channels": [0.25, 0.75, 0.73, 0.24, 0.58, 0.78, 0.72, 0.87, 0.33, 0.78, 0.35, 0.22, 0.81]},
            {"id": "loop_04", "t": 0.5, "channels": [0.18, 0.82, 0.46, 0.41, 0.41, 0.61, 0.50, 0.76, 0.48, 0.61, 0.49, 0.27, 0.63]},
            {"id": "loop_05", "t": 0.625, "channels": [0.24, 0.76, 0.21, 0.63, 0.30, 0.40, 0.29, 0.55, 0.67, 0.41, 0.63, 0.42, 0.39]},
            {"id": "loop_06", "t": 0.75, "channels": [0.49, 0.51, 0.12, 0.78, 0.37, 0.28, 0.25, 0.41, 0.74, 0.28, 0.77, 0.58, 0.23]},
            {"id": "loop_07", "t": 0.875, "channels": [0.74, 0.26, 0.27, 0.72, 0.55, 0.37, 0.46, 0.39, 0.61, 0.38, 0.80, 0.69, 0.34]},
            {"id": "loop_08", "t": 1.0, "channels": [0.82, 0.18, 0.54, 0.38, 0.76, 0.68, 0.88, 0.62, 0.34, 0.74, 0.26, 0.58, 0.72]},
        ],
        "branch_markers": [
            {"id": "coast_a", "label": "coast-a", "parent_id": "main", "t": 0.25, "sticky_radius": 0.08},
            {"id": "coast_b", "label": "coast-b", "parent_id": "main", "t": 0.625, "sticky_radius": 0.1},
        ],
    }
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _write_request_json(
    path: Path,
    state_path: Path,
    bundle_path: Path,
    out_dir: Path,
    *,
    comparison_fits: Path | None = None,
) -> None:
    payload = {
        "version": 1,
        "base_state_json": str(state_path),
        "bundle_json": str(bundle_path),
        "out_dir": str(out_dir),
        "ticks": 9,
    }
    if comparison_fits is not None:
        payload["comparison_fits"] = str(comparison_fits)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _capture_stable_window_pixels(hwnd: int) -> tuple[bytes, bytes]:
    deadline = time.monotonic() + 5.0
    left = _capture_ready_window_pixels(hwnd)
    while time.monotonic() < deadline:
        time.sleep(0.2)
        right = _capture_ready_window_pixels(hwnd)
        if len(left) == len(right):
            return left, right
        left = right
    raise AssertionError("window client size never stabilized for comparison capture")


def _mean_abs_diff(left: bytes, right: bytes) -> float:
    if len(left) != len(right):
        return 255.0
    total = 0
    for left_byte, right_byte in zip(left, right):
        total += abs(left_byte - right_byte)
    return total / len(left)


def _write_loaded_state_json(path: Path, state: dict[str, object]) -> None:
    path.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")


def _draft_row(state: dict[str, object], lane_id: str, row_index: int) -> dict[str, object]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), "expected loaded state to include color_pipeline_draft"
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), "expected color_pipeline_draft.lanes to be a list"
    for lane in lanes:
        if not isinstance(lane, dict) or lane.get("lane_id") != lane_id:
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list), f"expected lane {lane_id} rows to be a list"
        assert 0 <= row_index < len(rows), f"expected lane {lane_id} to contain row {row_index}"
        row = rows[row_index]
        assert isinstance(row, dict), f"expected lane {lane_id} row {row_index} to be an object"
        return row
    raise AssertionError(f"missing lane {lane_id}")


def _load_ui_automation_report(report_path: Path) -> dict[str, object] | None:
    if not report_path.exists():
        return None
    try:
        payload = json.loads(report_path.read_text(encoding="utf-8"))
    except (json.JSONDecodeError, OSError):
        return None
    return payload if isinstance(payload, dict) else None


def _wait_for_parseable_ui_automation_report(
    report_path: Path,
    *,
    timeout_seconds: float = 3.0,
) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_text: str | None = None
    while time.monotonic() < deadline:
        if report_path.exists():
            try:
                last_text = report_path.read_text(encoding="utf-8")
            except OSError:
                last_text = None
            payload = _load_ui_automation_report(report_path)
            if payload is not None:
                return payload
        time.sleep(0.1)
    raise AssertionError(f"automation report never became parseable JSON; last_text={last_text!r}")


def _lane_rows_from_report(payload: dict[str, object], lane_id: str) -> list[dict[str, object]]:
    rows = payload.get("rows", [])
    if not isinstance(rows, list):
        return []
    lane_rows: list[dict[str, object]] = []
    for row in rows:
        if not isinstance(row, dict):
            continue
        if str(row.get("lane_id", "")).strip() != lane_id:
            continue
        lane_rows.append(row)
    return lane_rows


def _wait_for_lane_rows(
    report_path: Path,
    lane_id: str,
    *,
    expected_count: int,
    target_row_id: int,
    target_enabled: bool,
    timeout_seconds: float = 10.0,
) -> list[dict[str, object]]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        payload = _load_ui_automation_report(report_path)
        if payload is None:
            time.sleep(0.1)
            continue
        last_payload = payload
        lane_rows = _lane_rows_from_report(payload, lane_id)
        if len(lane_rows) != expected_count:
            time.sleep(0.1)
            continue
        for row in lane_rows:
            try:
                row_id = int(row.get("ui_row_id", -1))
            except (TypeError, ValueError):
                continue
            enabled = row.get("enabled")
            if row_id == target_row_id and isinstance(enabled, bool) and enabled == target_enabled:
                return lane_rows
        time.sleep(0.1)
    raise AssertionError(
        f"automation report never reached lane={lane_id!r} count={expected_count} row_id={target_row_id} enabled={target_enabled}; last_payload={last_payload!r}"
    )


def test_explaino_registry_controls_no_mouse_set_value_change_live_viewport(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")

    exe_path = _active_runtime_exe()
    axes = _describe_explaino_axis_registry(exe_path)
    neutral_capture = _run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "explaino_all", "--width", "320", "--height", "240"
    )
    base_state = neutral_capture["state"]
    for axis in axes:
        axis_id = str(axis["axis_id"])
        control_id = str(axis["control_id"])
        active_value = _axis_active_value(axis)
        for fractal_type in ("explaino_all", str(axis["carrier_fractal_type"])):
            state = json.loads(json.dumps(base_state))
            state["fractal_type"] = fractal_type
            params = state["params"]
            assert isinstance(params, dict)
            for registry_axis in axes:
                params[str(registry_axis["axis_id"])] = 0.0
            state_path = _write_state_bundle(tmp_path / f"{fractal_type}_{axis_id}_neutral", state)
            baseline_payload = _capture_controls_report_with_optional_set_value(
                exe_path, state_path, tmp_path / f"{fractal_type}_{axis_id}_baseline_report.json", control_id, None
            )
            payload = _capture_controls_report_with_optional_set_value(
                exe_path, state_path, tmp_path / f"{fractal_type}_{axis_id}_edited_report.json", control_id, active_value
            )
            assert payload.get("current_fractal_type") == fractal_type
            baseline_hash = _require_rendered_frame_hash(baseline_payload)
            edited_hash = _require_rendered_frame_hash(payload)
            assert edited_hash != baseline_hash, (
                "No-mouse set-value automation for a visible Explaino registry control should change the rendered frame; "
                f"fractal_type={fractal_type!r} axis={axis_id!r} baseline_hash={baseline_hash} edited_hash={edited_hash}"
            )


def test_julia_controls_no_mouse_set_value_change_live_viewport(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")

    exe_path = _active_runtime_exe()
    neutral_capture = _run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "julia", "--width", "320", "--height", "240"
    )
    base_state = neutral_capture["state"]
    controls = [
        ("julia_c_real", "fractal_control.julia_c_real.primary", 0.285),
        ("julia_c_imag", "fractal_control.julia_c_imag.primary", 0.01),
    ]
    for param_id, control_id, set_value in controls:
        state = json.loads(json.dumps(base_state))
        state_path = _write_state_bundle(tmp_path / f"julia_{param_id}_neutral", state)
        baseline_payload = _capture_controls_report_with_optional_set_value(
            exe_path, state_path, tmp_path / f"julia_{param_id}_baseline_report.json", control_id, None
        )
        payload = _capture_controls_report_with_optional_set_value(
            exe_path, state_path, tmp_path / f"julia_{param_id}_edited_report.json", control_id, set_value
        )
        assert payload.get("current_fractal_type") == "julia"
        baseline_hash = _require_rendered_frame_hash(baseline_payload)
        edited_hash = _require_rendered_frame_hash(payload)
        assert edited_hash != baseline_hash, (
            "No-mouse set-value automation for standalone Julia constant controls should change the rendered frame; "
            f"param={param_id!r} baseline_hash={baseline_hash} edited_hash={edited_hash}"
        )


def test_nova_poly_c4_no_mouse_set_value_change_live_viewport(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")

    exe_path = _active_runtime_exe()
    neutral_capture = _run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "nova", "--width", "320", "--height", "240"
    )
    state = neutral_capture["state"]
    params = state["params"]
    assert isinstance(params, dict)
    params["poly_kind"] = 2
    params["poly_coeffs"] = [-1.0, 0.0, 0.0, 1.0, 0.0]
    params["coloring_mode"] = "smooth_escape"

    state_path = _write_state_bundle(tmp_path / "nova_poly_c4_neutral", state)
    control_id = "fractal_control.poly_c4.primary"
    baseline_payload = _capture_controls_report_with_optional_set_value(
        exe_path, state_path, tmp_path / "nova_poly_c4_baseline_report.json", control_id, None
    )
    payload = _capture_controls_report_with_optional_set_value(
        exe_path, state_path, tmp_path / "nova_poly_c4_edited_report.json", control_id, 0.65
    )
    assert payload.get("current_fractal_type") == "nova"
    baseline_hash = _require_rendered_frame_hash(baseline_payload)
    edited_hash = _require_rendered_frame_hash(payload)
    assert edited_hash != baseline_hash, (
        "No-mouse set-value automation for Nova poly_c4 should change the rendered frame; "
        f"baseline_hash={baseline_hash} edited_hash={edited_hash}"
    )



def test_runtime_walk_viewer_no_mouse_schema_int_set_value_consumes_visible_control(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")

    exe_path = _active_runtime_exe()
    neutral_capture = _run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "explaino_all", "--width", "320", "--height", "240"
    )
    state_path = _write_state_bundle(tmp_path / "schema_width_state", neutral_capture["state"])
    payload = _capture_controls_report_with_optional_set_value(
        exe_path,
        state_path,
        tmp_path / "schema_width_set_value_report.json",
        "fractal_control.width.primary",
        640.0,
    )
    assert payload.get("set_value_consumed") is True
    assert payload.get("set_value_error") is None


def test_runtime_walk_viewer_no_mouse_unknown_set_value_fails_closed(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")

    exe_path = _active_runtime_exe()
    neutral_capture = _run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "explaino_all", "--width", "320", "--height", "240"
    )
    state_path = _write_state_bundle(tmp_path / "schema_unknown_state", neutral_capture["state"])
    report_path = tmp_path / "schema_unknown_set_value_report.json"
    control_id = "fractal_control.not_a_real_visible_control.primary"
    args = [
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--ui-automation-report-json",
        str(report_path),
        "--ui-automation-set-control-value",
        f"{control_id}=0.5",
    ]
    proc = subprocess.Popen(args, cwd=str(RUNTIME_DIR))
    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        _focus_window(hwnd)
        payload = _wait_for_ui_automation_set_value_error(report_path, control_id)
        assert "not visible" in str(payload.get("set_value_error", "")).lower() or "unsupported" in str(payload.get("set_value_error", "")).lower()
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)


def test_runtime_walk_viewer_automation_report_escapes_dynamic_requested_ids(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")

    exe_path = _active_runtime_exe()
    neutral_capture = _run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "explaino_all", "--width", "320", "--height", "240"
    )
    state_path = _write_state_bundle(tmp_path / "quoted_click_state", neutral_capture["state"])
    report_path = tmp_path / "quoted_click_report.json"
    control_id = 'runtime_walk.bad"quoted\\control' + chr(1)
    args = [
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--ui-automation-report-json",
        str(report_path),
        "--ui-automation-click-control-id",
        control_id,
    ]
    proc = subprocess.Popen(args, cwd=str(RUNTIME_DIR))
    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        _focus_window(hwnd)
        payload = _wait_for_parseable_ui_automation_report(report_path)
        assert payload.get("requested_click_control_id") == control_id
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)


def _wait_for_ui_automation_click(
    report_path: Path,
    control_id: str,
    *,
    timeout_seconds: float = 10.0,
) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        payload = _load_ui_automation_report(report_path)
        if payload is None:
            time.sleep(0.1)
            continue
        last_payload = payload
        if payload.get("requested_click_control_id") != control_id:
            time.sleep(0.1)
            continue
        if payload.get("click_consumed") is True:
            return payload
        time.sleep(0.1)
    raise AssertionError(
        f"automation report never consumed requested click {control_id!r}; last_payload={last_payload!r}"
    )

def _seed_two_grading_row_state(tmp_path: Path, exe_path: Path) -> dict[str, object]:
    baseline_capture = _capture_explaino_runtime_baseline(exe_path)
    draft_state = _with_explaino_repeat_heatmap_draft_state(baseline_capture["state"])
    scenario_result = _run_headless_loaded_state_scenario(
        tmp_path / "seed_two_grading_rows",
        exe_path=exe_path,
        scenario=_HeadlessLoadedStateScenario(
            name="seed_two_grading_rows",
            state=draft_state,
            action_args=(
                "--color-pipeline-action",
                "add_row:grading:phase_finish",
                "--color-pipeline-action",
                "set_param:grading:1:grade.saturation:number:0.8",
                "--color-pipeline-action",
                "set_param:grading:1:grade.contrast:number:1.6",
            ),
        ),
    )
    return scenario_result.scenario_capture["state"]


def _seed_unsupported_palette_row_state(exe_path: Path) -> dict[str, object]:
    baseline_capture = _capture_explaino_runtime_baseline(exe_path)
    state = json.loads(json.dumps(_with_explaino_repeat_heatmap_draft_state(baseline_capture["state"])))
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), "expected baseline capture to expose color_pipeline_draft"
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), "expected color_pipeline_draft.lanes to be a list"
    next_row_id = int(draft.get("next_row_id", 0))
    assert next_row_id > 0, "expected color_pipeline_draft.next_row_id to stay positive"
    for lane in lanes:
        if not isinstance(lane, dict) or lane.get("lane_id") != "palette":
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list), "expected Palette lane rows to be a list"
        rows.append(
            {
                "ui_row_id": next_row_id,
                "enabled": True,
                "function_id": "root_classic_palette",
                "parameter_values": [],
            }
        )
        draft["next_row_id"] = next_row_id + 1
        return state
    raise AssertionError("missing Palette lane in baseline color_pipeline_draft")

def _load_recent_runtime_walk_sessions() -> dict:
    recent_path = RUNTIME_DIR / "diagnostics" / "runtime_walk_sessions" / "recent_sessions.json"
    return json.loads(recent_path.read_text(encoding="utf-8"))


def test_runtime_walk_viewer_replays_and_space_pauses(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")

    exe_path = _active_runtime_exe()
    state_path = tmp_path / "state.json"
    bundle_path = tmp_path / "bundle.json"
    request_path = tmp_path / "runtime_walk_viewer_request.json"
    out_dir = tmp_path / "out"
    _write_state_json(state_path)
    _write_bundle_json(bundle_path)
    _write_request_json(request_path, state_path, bundle_path, out_dir)

    proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-runtime-walk-request-json",
            str(request_path),
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        deadline = time.monotonic() + 10.0
        while time.monotonic() < deadline:
            hwnd = _find_window_for_pid(proc.pid)
            if hwnd is not None:
                break
            if proc.poll() is not None:
                raise AssertionError(f"viewer exited before creating a window; returncode={proc.returncode}")
            time.sleep(0.1)

        assert hwnd is not None, f"viewer never created a top-level window for pid {proc.pid}"

        user32 = ctypes.windll.user32
        _focus_window(hwnd)

        time.sleep(0.8)
        running_frame_a, running_frame_b = _capture_stable_window_pixels(hwnd)
        running_diff = _mean_abs_diff(running_frame_a, running_frame_b)
        assert running_diff > 0.05, f"runtime-walk viewer did not visibly animate while playing; diff={running_diff:.3f}"

        user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYDOWN, VK_SPACE, 0)
        user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYUP, VK_SPACE, 0)

        time.sleep(0.5)
        paused_frame_a, paused_frame_b = _capture_stable_window_pixels(hwnd)
        paused_diff = _mean_abs_diff(paused_frame_a, paused_frame_b)
        assert paused_diff < 0.1, f"Space pause did not freeze runtime-walk playback; diff={paused_diff:.3f}"

        user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYDOWN, VK_RIGHT, 0)
        user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYUP, VK_RIGHT, 0)
        time.sleep(0.5)
        stepped_frame, _ = _capture_stable_window_pixels(hwnd)
        stepped_diff = _mean_abs_diff(paused_frame_b, stepped_frame)
        assert stepped_diff > 0.06, f"Right-arrow step did not visibly advance runtime-walk playback; diff={stepped_diff:.3f}"
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)


def test_runtime_walk_viewer_no_mouse_color_pipeline_set_value_changes_frame(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")
    exe_path = _active_runtime_exe()
    state_path = tmp_path / "state.json"
    bundle_path = tmp_path / "bundle.json"
    request_path = tmp_path / "runtime_walk_viewer_request.json"
    out_dir = tmp_path / "out"
    _write_state_json(state_path)
    _write_bundle_json(bundle_path)
    _write_request_json(request_path, state_path, bundle_path, out_dir)
    baseline_payload = _capture_runtime_walk_color_pipeline_report(
        exe_path, request_path, tmp_path / "color_pipeline_baseline_report.json", None
    )
    payload = _capture_runtime_walk_color_pipeline_report(
        exe_path, request_path, tmp_path / "color_pipeline_edited_report.json", 1.65
    )
    assert payload.get("requested_set_control_id") == COLOR_PIPELINE_SCALE_CONTROL_ID
    assert payload.get("set_value_consumed") is True
    baseline_hash = _require_rendered_frame_hash(baseline_payload)
    edited_hash = _require_rendered_frame_hash(payload)
    assert edited_hash != baseline_hash, (
        "No-mouse set-value automation for the Color Pipeline Source Scale control should change the published rendered frame; "
        f"baseline_hash={baseline_hash} edited_hash={edited_hash}"
    )


def test_runtime_walk_viewer_first_party_tests_do_not_use_physical_mouse_automation() -> None:
    forbidden_tokens = [
        "Set" + "CursorPos",
        "Get" + "CursorPos",
        "mouse" + "_event",
        "MOUSE" + "EVENTF",
        "drag" + "_screen_rect",
        "Send" + "Input",
    ]
    for path in (REPO_ROOT / "tests" / "runtime_harness.py", Path(__file__)):
        source = path.read_text(encoding="utf-8")
        for token in forbidden_tokens:
            assert token not in source, f"first-party runtime tests must not use physical mouse automation token {token!r} in {path}"



def test_runtime_viewer_grading_enabled_checkbox_preserves_disabled_row(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("advanced-color viewer checkbox regression is Windows-only")

    exe_path = _active_runtime_exe()
    viewer_dir = Path(tempfile.mkdtemp())
    seeded_state = _seed_two_grading_row_state(viewer_dir, exe_path)
    toggled_row = _draft_row(seeded_state, "grading", 1)
    row_id = int(toggled_row["ui_row_id"])
    control_id = f"color_pipeline.grading.{row_id}.enabled"

    state_path = viewer_dir / "state.json"
    baseline_report_path = viewer_dir / "report.json"
    toggled_report_path = viewer_dir / "toggle.json"
    _write_loaded_state_json(state_path, seeded_state)

    baseline_proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--open-color-pipeline-window",
            "--ui-automation-report-json",
            str(baseline_report_path),
        ],
        cwd=str(RUNTIME_DIR),
    )

    baseline_hwnd: int | None = None
    try:
        baseline_hwnd = _wait_for_window(baseline_proc)
        wait_for_ui_automation_rect(baseline_report_path, control_id)
        baseline_rows = _wait_for_lane_rows(
            baseline_report_path,
            "grading",
            expected_count=2,
            target_row_id=row_id,
            target_enabled=True,
        )
        assert int(baseline_rows[1]["ui_row_id"]) == row_id
        assert str(baseline_rows[1]["function_id"]) == "phase_finish"
        assert [bool(row.get("enabled")) for row in baseline_rows] == [True, True]
    finally:
        if baseline_hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(baseline_hwnd), WM_CLOSE, 0, 0)
        if baseline_proc.poll() is None:
            try:
                baseline_proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                baseline_proc.kill()
                baseline_proc.wait(timeout=5.0)

    proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--open-color-pipeline-window",
            "--ui-automation-report-json",
            str(toggled_report_path),
            "--ui-automation-click-control-id",
            control_id,
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        payload = _wait_for_ui_automation_click(toggled_report_path, control_id)
        toggled_rows = _wait_for_lane_rows(
            toggled_report_path,
            "grading",
            expected_count=2,
            target_row_id=row_id,
            target_enabled=False,
        )
        assert int(toggled_rows[1]["ui_row_id"]) == row_id
        assert str(toggled_rows[1]["function_id"]) == "phase_finish"
        assert [bool(row.get("enabled")) for row in toggled_rows] == [True, False]
        assert payload.get("validation_messages") == []
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)


def test_runtime_viewer_unsupported_palette_checkbox_preserves_disabled_row(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("advanced-color viewer checkbox regression is Windows-only")

    exe_path = _active_runtime_exe()
    viewer_dir = Path(tempfile.mkdtemp())
    seeded_state = _seed_unsupported_palette_row_state(exe_path)
    toggled_row = _draft_row(seeded_state, "palette", 1)
    row_id = int(toggled_row["ui_row_id"])
    control_id = f"color_pipeline.palette.{row_id}.enabled"

    state_path = viewer_dir / "state.json"
    baseline_report_path = viewer_dir / "report.json"
    toggled_report_path = viewer_dir / "toggle.json"
    _write_loaded_state_json(state_path, seeded_state)

    baseline_proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--open-color-pipeline-window",
            "--ui-automation-report-json",
            str(baseline_report_path),
        ],
        cwd=str(RUNTIME_DIR),
    )

    baseline_hwnd: int | None = None
    try:
        baseline_hwnd = _wait_for_window(baseline_proc)
        wait_for_ui_automation_rect(baseline_report_path, control_id)
        baseline_rows = _wait_for_lane_rows(
            baseline_report_path,
            "palette",
            expected_count=2,
            target_row_id=row_id,
            target_enabled=True,
        )
        assert [str(row.get("function_id")) for row in baseline_rows] == ["heatmap", "root_classic_palette"]
        assert [bool(row.get("enabled")) for row in baseline_rows] == [True, True]
    finally:
        if baseline_hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(baseline_hwnd), WM_CLOSE, 0, 0)
        if baseline_proc.poll() is None:
            try:
                baseline_proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                baseline_proc.kill()
                baseline_proc.wait(timeout=5.0)

    proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--open-color-pipeline-window",
            "--ui-automation-report-json",
            str(toggled_report_path),
            "--ui-automation-click-control-id",
            control_id,
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        payload = _wait_for_ui_automation_click(toggled_report_path, control_id)
        toggled_rows = _wait_for_lane_rows(
            toggled_report_path,
            "palette",
            expected_count=2,
            target_row_id=row_id,
            target_enabled=False,
        )
        assert int(toggled_rows[1]["ui_row_id"]) == row_id
        assert str(toggled_rows[1]["function_id"]) == "root_classic_palette"
        assert [bool(row.get("enabled")) for row in toggled_rows] == [True, False]
        assert payload.get("validation_messages") == []
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)


def test_runtime_viewer_explicit_remove_button_still_removes_row(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("advanced-color viewer checkbox regression is Windows-only")

    exe_path = _active_runtime_exe()
    viewer_dir = Path(tempfile.mkdtemp())
    seeded_state = _seed_two_grading_row_state(viewer_dir, exe_path)
    remaining_row = _draft_row(seeded_state, "grading", 0)
    removed_row = _draft_row(seeded_state, "grading", 1)
    remaining_row_id = int(remaining_row["ui_row_id"])
    removed_row_id = int(removed_row["ui_row_id"])
    control_id = f"color_pipeline.grading.{removed_row_id}.remove"

    state_path = viewer_dir / "state.json"
    baseline_report_path = viewer_dir / "report.json"
    removed_report_path = viewer_dir / "remove.json"
    _write_loaded_state_json(state_path, seeded_state)

    baseline_proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--open-color-pipeline-window",
            "--ui-automation-report-json",
            str(baseline_report_path),
        ],
        cwd=str(RUNTIME_DIR),
    )

    baseline_hwnd: int | None = None
    try:
        baseline_hwnd = _wait_for_window(baseline_proc)
        wait_for_ui_automation_rect(baseline_report_path, control_id)
        baseline_rows = _wait_for_lane_rows(
            baseline_report_path,
            "grading",
            expected_count=2,
            target_row_id=removed_row_id,
            target_enabled=True,
        )
        assert [str(row.get("function_id")) for row in baseline_rows] == ["contrast_lift", "phase_finish"]
    finally:
        if baseline_hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(baseline_hwnd), WM_CLOSE, 0, 0)
        if baseline_proc.poll() is None:
            try:
                baseline_proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                baseline_proc.kill()
                baseline_proc.wait(timeout=5.0)

    proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--open-color-pipeline-window",
            "--ui-automation-report-json",
            str(removed_report_path),
            "--ui-automation-click-control-id",
            control_id,
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        payload = _wait_for_ui_automation_click(removed_report_path, control_id)
        remaining_rows = _wait_for_lane_rows(
            removed_report_path,
            "grading",
            expected_count=1,
            target_row_id=remaining_row_id,
            target_enabled=True,
        )
        assert [int(row.get("ui_row_id", -1)) for row in remaining_rows] == [remaining_row_id]
        assert [str(row.get("function_id")) for row in remaining_rows] == ["contrast_lift"]
        assert all(int(row.get("ui_row_id", -1)) != removed_row_id for row in remaining_rows)
        assert payload.get("validation_messages") == []
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)

def test_runtime_walk_viewer_tolerates_missing_companion_fits(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")

    exe_path = _active_runtime_exe()
    state_path = tmp_path / "state.json"
    bundle_path = tmp_path / "bundle.json"
    request_path = tmp_path / "runtime_walk_viewer_request.json"
    out_dir = tmp_path / "out"
    missing_fits = tmp_path / "missing_checkpoint_final.fits"
    _write_state_json(state_path)
    _write_bundle_json(bundle_path)
    _write_request_json(request_path, state_path, bundle_path, out_dir, comparison_fits=missing_fits)

    proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-runtime-walk-request-json",
            str(request_path),
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        deadline = time.monotonic() + 10.0
        while time.monotonic() < deadline:
            hwnd = _find_window_for_pid(proc.pid)
            if hwnd is not None:
                break
            if proc.poll() is not None:
                raise AssertionError(f"viewer exited before creating a window; returncode={proc.returncode}")
            time.sleep(0.1)

        assert hwnd is not None, f"viewer never created a top-level window for pid {proc.pid}"

        _focus_window(hwnd)

        time.sleep(0.8)
        running_frame_a, running_frame_b = _capture_stable_window_pixels(hwnd)
        running_diff = _mean_abs_diff(running_frame_a, running_frame_b)
        assert running_diff > 0.05, (
            "runtime-walk viewer should keep animating when companion FITS is missing; "
            f"diff={running_diff:.3f}"
        )
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)


def test_runtime_walk_viewer_can_boot_from_fits_only_cli() -> None:
    if sys.platform != "win32":
        pytest.skip("runtime-walk viewer regression is Windows-only")
    if not REAL_FITS_PATH.exists():
        pytest.skip(f"missing real FITS acceptance artifact: {REAL_FITS_PATH}")

    exe_path = _active_runtime_exe()
    artifact_cutoff = time.time() - 1.0
    proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-runtime-walk-fits",
            str(REAL_FITS_PATH),
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        deadline = time.monotonic() + 10.0
        while time.monotonic() < deadline:
            hwnd = _find_window_for_pid(proc.pid)
            if hwnd is not None:
                break
            if proc.poll() is not None:
                raise AssertionError(f"viewer exited before creating a window; returncode={proc.returncode}")
            time.sleep(0.1)

        assert hwnd is not None, f"viewer never created a top-level window for pid {proc.pid}"

        _focus_window(hwnd)

        time.sleep(0.8)
        running_frame_a, running_frame_b = _capture_stable_window_pixels(hwnd)
        running_diff = _mean_abs_diff(running_frame_a, running_frame_b)
        assert running_diff > 0.05, (
            "FITS-only runtime-walk viewer load did not visibly animate; "
            f"diff={running_diff:.3f}"
        )

        recent = _load_recent_runtime_walk_sessions()
        assert recent["sessions"], "FITS-only load did not record a recent runtime-walk session"
        latest = recent["sessions"][0]
        assert latest["authority_mode"] == "synthesized_fits_base"
        assert Path(latest["comparison_fits"]) == REAL_FITS_PATH
        assert latest["transport_generated"] is True
        assert latest["transport_generation_mode"] == "closed_loop_default"
        assert latest["transport_sample_count"] >= 33
        assert latest["transport_motion_scale"] == pytest.approx(0.75)
        assert "transport_warp_scale" not in latest
        synthesized_state = Path(latest["synthesized_base_state_json"])
        assert synthesized_state.exists(), "FITS-only load did not write synthesized state.json"
        synthesized_payload = json.loads(synthesized_state.read_text(encoding="utf-8"))
        assert synthesized_payload["fractal_type"] == "explaino"

        session_dir = Path(latest["request_json"]).parent
        orientation_json = Path(latest["orientation_inputs_json"])
        mapping_json = Path(latest["mapping_profile_json"])
        assert orientation_json.exists(), "FITS-only load did not write orientation_inputs.json"
        assert mapping_json.exists(), "FITS-only load did not write effective_mapping_profile.json"
        orientation_payload = json.loads(orientation_json.read_text(encoding="utf-8"))
        assert orientation_payload["frame_count"] >= 1
        assert orientation_payload["frames"], "orientation inputs should expose a per-frame FITS timeline"
        effective_profile = json.loads(mapping_json.read_text(encoding="utf-8"))
        bindings = effective_profile["profiles"][0]["bindings"]
        assert any(binding.get("source_kind") == "fits_frame" for binding in bindings)
        assert any(binding.get("source_kind") == "field" for binding in bindings)
        assert all("warp" not in binding["target_path"] for binding in bindings)
        flow_csv = session_dir / "runtime_walk_flow_lines.csv"
        cells_csv = session_dir / "runtime_field_cells.csv"
        binding_csv = session_dir / "runtime_walk_binding_samples.csv"

        def fresh_nonempty(path: Path) -> bool:
            return path.exists() and path.stat().st_size > 0 and path.stat().st_mtime >= artifact_cutoff

        deadline = time.monotonic() + 5.0
        while time.monotonic() < deadline:
            if (
                fresh_nonempty(flow_csv)
                and fresh_nonempty(cells_csv)
                and fresh_nonempty(binding_csv)
            ):
                break
            time.sleep(0.1)
        assert fresh_nonempty(flow_csv), "FITS-only viewer playback did not export fresh non-empty runtime_walk_flow_lines.csv"
        assert fresh_nonempty(cells_csv), "FITS-only viewer playback did not export fresh non-empty runtime_field_cells.csv"
        assert fresh_nonempty(binding_csv), "FITS-only viewer playback did not export fresh non-empty runtime_walk_binding_samples.csv"
        flow_text = flow_csv.read_text(encoding="utf-8")
        cells_text = cells_csv.read_text(encoding="utf-8")
        binding_text = binding_csv.read_text(encoding="utf-8")
        assert "traveler_cluster_id" in flow_text and "tangent_angle" in flow_text
        assert "fits_frame_index" in flow_text
        assert "traveler_centroid_x" in cells_text and "cluster_confidence" in cells_text
        assert "fits_frame_index" in cells_text
        assert "t,source_path,target_path,frame_index,source_value,baseline_value,offset_value,composed_value" in binding_text
        assert len(flow_text.splitlines()) > 1, "flow CSV should contain measured marble rows"
        assert len(cells_text.splitlines()) > 1, "field-cell CSV should contain quantized cell rows"
        assert len(binding_text.splitlines()) > 1, "binding CSV should contain live binding rows"
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)
