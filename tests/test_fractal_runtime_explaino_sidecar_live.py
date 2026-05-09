from __future__ import annotations

import json
import subprocess
import sys
import time
from pathlib import Path

import pytest

from tests.runtime_harness import (
    RUNTIME_DIR,
    active_runtime_exe as _active_runtime_exe,
    capture_ready_window_pixels as _capture_ready_window_pixels,
    close_runtime as _close_runtime,
    focus_window as _focus_window,
    mean_abs_diff as _mean_abs_diff,
    run_headless_capture as _run_headless_capture,
    wait_for_window as _wait_for_window,
    write_state_bundle as _write_state_bundle,
)


def _configure_sidecar_policy(state: dict[str, object], **updates: object) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    policy = configured_state["sidecar_auto_demo_policy"]
    assert isinstance(policy, dict)
    policy.update(updates)
    return configured_state


def test_runtime_default_explaino_startup_stays_visually_stable() -> None:
    if sys.platform != "win32":
        pytest.skip("live Explaino runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    proc = subprocess.Popen(
        [
            str(exe_path),
            "--fractal-type",
            "explaino",
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        _focus_window(hwnd)

        time.sleep(1.0)
        frame_a = _capture_ready_window_pixels(hwnd)
        time.sleep(0.8)
        frame_b = _capture_ready_window_pixels(hwnd)
        diff = _mean_abs_diff(frame_a, frame_b)
        assert diff < 0.1, (
            "default Explaino startup changed the live image without sidecar mutation or user input; "
            f"diff={diff:.3f}"
        )
    finally:
        _close_runtime(hwnd, proc)


def test_runtime_loaded_sidecar_paced_loop_changes_live_view_multiple_times(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("live Explaino runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )
    moving_state_path = _write_state_bundle(
        tmp_path / "moving",
        _configure_sidecar_policy(
            baseline_capture["state"],
            enabled=True,
            allow_runtime_mutation=True,
            run_paced_loop=True,
            paced_loop_interval_seconds=0.25,
            stop_demonstrated_fraction=1.0,
            stop_uncertain_count=0,
        ),
    )

    proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-state-json",
            str(moving_state_path),
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        _focus_window(hwnd)

        time.sleep(0.6)
        frame_a = _capture_ready_window_pixels(hwnd)
        time.sleep(0.8)
        frame_b = _capture_ready_window_pixels(hwnd)
        time.sleep(0.8)
        frame_c = _capture_ready_window_pixels(hwnd)
        diff_ab = _mean_abs_diff(frame_a, frame_b)
        diff_bc = _mean_abs_diff(frame_b, frame_c)

        assert diff_ab > 0.15, f"loaded sidecar paced loop did not visibly change the first live interval; diff={diff_ab:.3f}"
        assert diff_bc > 0.15, f"loaded sidecar paced loop did not visibly change the second live interval; diff={diff_bc:.3f}"
    finally:
        _close_runtime(hwnd, proc)


def test_runtime_loaded_sidecar_zero_threshold_stays_visually_stable(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("live Explaino runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )
    stopped_state_path = _write_state_bundle(
        tmp_path / "stopped",
        _configure_sidecar_policy(
            baseline_capture["state"],
            enabled=True,
            allow_runtime_mutation=True,
            run_paced_loop=True,
            paced_loop_interval_seconds=0.25,
            stop_demonstrated_fraction=0.0,
            stop_uncertain_count=0,
        ),
    )

    proc = subprocess.Popen(
        [
            str(exe_path),
            "--load-state-json",
            str(stopped_state_path),
        ],
        cwd=str(RUNTIME_DIR),
    )

    hwnd: int | None = None
    try:
        hwnd = _wait_for_window(proc)
        _focus_window(hwnd)

        time.sleep(1.0)
        frame_a = _capture_ready_window_pixels(hwnd)
        time.sleep(0.8)
        frame_b = _capture_ready_window_pixels(hwnd)
        diff = _mean_abs_diff(frame_a, frame_b)
        assert diff < 0.1, (
            "zero-threshold sidecar policy should stop live runtime motion immediately; "
            f"diff={diff:.3f}"
        )
    finally:
        _close_runtime(hwnd, proc)