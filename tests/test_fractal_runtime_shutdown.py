from __future__ import annotations

import ctypes
import subprocess
import sys
import time
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"


def _active_runtime_exe() -> Path:
    if not ACTIVE_RUNTIME_FILE.exists():
        pytest.skip(f"missing active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    active_name = ACTIVE_RUNTIME_FILE.read_text(encoding="utf-8").strip()
    if not active_name:
        pytest.skip(f"empty active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    exe_path = RUNTIME_DIR / active_name
    if not exe_path.exists():
        pytest.skip(f"active runtime missing: {exe_path}")
    return exe_path


def _find_window_for_pid(pid: int) -> int | None:
    user32 = ctypes.windll.user32
    found: list[int] = []

    enum_proc = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_void_p, ctypes.c_void_p)

    @enum_proc
    def callback(hwnd: int, _lparam: int) -> bool:
        window_pid = ctypes.c_ulong()
        user32.GetWindowThreadProcessId(ctypes.c_void_p(hwnd), ctypes.byref(window_pid))
        if window_pid.value == pid and user32.IsWindowVisible(ctypes.c_void_p(hwnd)):
            found.append(hwnd)
            return False
        return True

    user32.EnumWindows(callback, 0)
    return found[0] if found else None


def test_runtime_process_exits_after_wm_close() -> None:
    if sys.platform != "win32":
        pytest.skip("viewer shutdown regression test is Windows-only")

    exe_path = _active_runtime_exe()
    proc = subprocess.Popen([str(exe_path)], cwd=str(RUNTIME_DIR))
    try:
        hwnd = None
        deadline = time.monotonic() + 10.0
        while time.monotonic() < deadline:
            hwnd = _find_window_for_pid(proc.pid)
            if hwnd is not None:
                break
            if proc.poll() is not None:
                raise AssertionError(f"viewer exited before creating a window; returncode={proc.returncode}")
            time.sleep(0.1)

        assert hwnd is not None, f"viewer never created a top-level window for pid {proc.pid}"

        ctypes.windll.user32.PostMessageW(ctypes.c_void_p(hwnd), 0x0010, 0, 0)  # WM_CLOSE
        try:
            proc.wait(timeout=10.0)
        except subprocess.TimeoutExpired:
            proc.kill()
            raise AssertionError(
                "viewer did not exit within 10s after WM_CLOSE; primary runtime would stay locked"
            )

        assert proc.returncode == 0, f"viewer returned non-zero exit code after WM_CLOSE: {proc.returncode}"
    finally:
        if proc.poll() is None:
            proc.kill()
            proc.wait(timeout=5.0)