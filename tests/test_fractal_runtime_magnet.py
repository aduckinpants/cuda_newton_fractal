from __future__ import annotations

import ctypes
from ctypes import wintypes
import json
import subprocess
import sys
import time
from pathlib import Path

import pytest

from tests.runtime_harness import RUNTIME_DIR, WM_CLOSE, active_runtime_exe, run_headless_capture, wait_for_ui_automation_rect, wait_for_window, write_state_bundle

MAGNET_RELAXATION_CONTROL_ID = "fractal_control.magnet_relaxation.primary"


def _load_report(path: Path) -> dict[str, object] | None:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (FileNotFoundError, json.JSONDecodeError, OSError):
        return None


def _wait_for_set_value(report_path: Path, control_id: str, *, timeout_seconds: float = 10.0) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        payload = _load_report(report_path)
        if payload is None:
            time.sleep(0.05)
            continue
        last_payload = payload
        if payload.get("requested_set_control_id") != control_id:
            time.sleep(0.05)
            continue
        assert not payload.get("set_value_error"), payload
        if payload.get("set_value_consumed") is True:
            return payload
        time.sleep(0.1)
    raise AssertionError(f"UI set-value automation never consumed {control_id}; last_payload={last_payload!r}")


def _wait_for_rendered_frame(report_path: Path, *, timeout_seconds: float = 10.0) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        payload = _load_report(report_path)
        if payload is None:
            time.sleep(0.05)
            continue
        last_payload = payload
        if payload.get("rendered_frame_ready") is True and isinstance(payload.get("rendered_frame_hash"), str):
            return payload
        time.sleep(0.1)
    raise AssertionError(f"UI automation report never exposed a ready rendered frame; last_payload={last_payload!r}")


def _capture_magnet_ui_report(exe_path: Path, state_path: Path, report_path: Path, *, set_value: float | None = None) -> dict[str, object]:
    args = [str(exe_path), "--load-state-json", str(state_path), "--ui-automation-report-json", str(report_path)]
    if set_value is not None:
        args.extend(["--ui-automation-set-control-value", f"{MAGNET_RELAXATION_CONTROL_ID}={set_value}"])
    proc = subprocess.Popen(args, cwd=str(RUNTIME_DIR))
    hwnd: int | None = None
    try:
        hwnd = wait_for_window(proc)
        wait_for_ui_automation_rect(report_path, MAGNET_RELAXATION_CONTROL_ID)
        if set_value is not None:
            return _wait_for_set_value(report_path, MAGNET_RELAXATION_CONTROL_ID)
        return _wait_for_rendered_frame(report_path)
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)


def _rendered_hash(payload: dict[str, object]) -> str:
    assert payload.get("rendered_frame_ready") is True, payload
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), payload
    return frame_hash


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
def test_magnet_relaxation_visible_and_drives_rendered_frame_without_mouse(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    baseline_capture = run_headless_capture(str(exe_path), "--capture-diagnostic", "--fractal-type", "magnet", "--width", "320", "--height", "240")
    state = baseline_capture["state"]
    assert state["fractal_type"] == "magnet"
    assert state["params"]["magnet_relaxation"] == pytest.approx(1.0)
    state_path = write_state_bundle(tmp_path, state)
    baseline_report = _capture_magnet_ui_report(exe_path, state_path, tmp_path / "magnet_baseline_report.json")
    changed_report = _capture_magnet_ui_report(exe_path, state_path, tmp_path / "magnet_relaxation_report.json", set_value=0.42)
    assert changed_report.get("set_value_consumed") is True
    assert changed_report.get("requested_set_control_id") == MAGNET_RELAXATION_CONTROL_ID
    assert _rendered_hash(changed_report) != _rendered_hash(baseline_report)
