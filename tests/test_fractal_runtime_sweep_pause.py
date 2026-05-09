from __future__ import annotations

import ctypes
from ctypes import wintypes
import subprocess
import sys
import time

import pytest

from tests.runtime_harness import (
    RUNTIME_DIR,
    WM_CLOSE,
    active_runtime_exe as _active_runtime_exe,
    capture_ready_window_pixels as _capture_ready_window_pixels,
    find_window_for_pid as _find_window_for_pid,
    focus_window as _focus_window,
    mean_abs_diff as _mean_abs_diff,
)

WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101
VK_SPACE = 0x20


def test_runtime_sweep_changes_live_view_and_space_pauses_it() -> None:
    if sys.platform != "win32":
        pytest.skip("live sweep regression test is Windows-only")

    exe_path = _active_runtime_exe()
    proc = subprocess.Popen(
        [
            str(exe_path),
            "--fractal-type",
            "explaino",
            "--sweep-seed-start",
            "0.70",
            "--sweep-seed-stop",
            "0.72",
            "--sweep-seed-step",
            "0.02",
            "--sweep-dwell-ms",
            "300",
            "--sweep-loop",
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

        user32 = __import__("ctypes").windll.user32

        time.sleep(0.8)
        running_frame_a = _capture_ready_window_pixels(hwnd)
        time.sleep(0.8)
        running_frame_b = _capture_ready_window_pixels(hwnd)
        time.sleep(0.8)
        running_frame_c = _capture_ready_window_pixels(hwnd)
        running_diff_ab = _mean_abs_diff(running_frame_a, running_frame_b)
        running_diff_bc = _mean_abs_diff(running_frame_b, running_frame_c)
        running_diff = max(running_diff_ab, running_diff_bc)
        assert running_diff > 0.15, (
            "live sweep did not visibly change the viewer image across adjacent intervals; "
            f"diff_ab={running_diff_ab:.3f} diff_bc={running_diff_bc:.3f}"
        )

        user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYDOWN, VK_SPACE, 0)
        user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYUP, VK_SPACE, 0)

        time.sleep(0.5)
        paused_frame_a = _capture_ready_window_pixels(hwnd)
        time.sleep(0.8)
        paused_frame_b = _capture_ready_window_pixels(hwnd)
        paused_diff = _mean_abs_diff(paused_frame_a, paused_frame_b)
        assert paused_diff < 0.1, f"Space pause did not freeze the live sweep image; diff={paused_diff:.3f}"
    finally:
        if hwnd is not None:
            ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
        if proc.poll() is None:
            try:
                proc.wait(timeout=5.0)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5.0)