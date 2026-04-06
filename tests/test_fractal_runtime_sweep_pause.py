from __future__ import annotations

import ctypes
from ctypes import wintypes
import subprocess
import sys
import time
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"

WM_CLOSE = 0x0010
WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101
VK_SPACE = 0x20
SRCCOPY = 0x00CC0020
DIB_RGB_COLORS = 0
PW_RENDERFULLCONTENT = 0x00000002


class RECT(ctypes.Structure):
    _fields_ = [
        ("left", ctypes.c_long),
        ("top", ctypes.c_long),
        ("right", ctypes.c_long),
        ("bottom", ctypes.c_long),
    ]


class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [
        ("biSize", wintypes.DWORD),
        ("biWidth", wintypes.LONG),
        ("biHeight", wintypes.LONG),
        ("biPlanes", wintypes.WORD),
        ("biBitCount", wintypes.WORD),
        ("biCompression", wintypes.DWORD),
        ("biSizeImage", wintypes.DWORD),
        ("biXPelsPerMeter", wintypes.LONG),
        ("biYPelsPerMeter", wintypes.LONG),
        ("biClrUsed", wintypes.DWORD),
        ("biClrImportant", wintypes.DWORD),
    ]


class BITMAPINFO(ctypes.Structure):
    _fields_ = [
        ("bmiHeader", BITMAPINFOHEADER),
        ("bmiColors", wintypes.DWORD * 3),
    ]


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

    enum_proc = ctypes.WINFUNCTYPE(ctypes.c_bool, wintypes.HWND, wintypes.LPARAM)

    @enum_proc
    def callback(hwnd: int, _lparam: int) -> bool:
        window_pid = wintypes.DWORD()
        user32.GetWindowThreadProcessId(wintypes.HWND(hwnd), ctypes.byref(window_pid))
        if window_pid.value == pid and user32.IsWindow(wintypes.HWND(hwnd)):
            found.append(int(hwnd))
            return False
        return True

    user32.EnumWindows(callback, 0)
    return found[0] if found else None


def _capture_window_pixels(hwnd: int) -> bytes:
    user32 = ctypes.windll.user32
    gdi32 = ctypes.windll.gdi32

    rect = RECT()
    if not user32.GetClientRect(wintypes.HWND(hwnd), ctypes.byref(rect)):
        raise OSError("GetClientRect failed")

    width = rect.right - rect.left
    height = rect.bottom - rect.top
    hwnd_dc = user32.GetDC(wintypes.HWND(hwnd))
    mem_dc = gdi32.CreateCompatibleDC(hwnd_dc)
    bitmap = gdi32.CreateCompatibleBitmap(hwnd_dc, width, height)
    gdi32.SelectObject(mem_dc, bitmap)

    if not user32.PrintWindow(wintypes.HWND(hwnd), mem_dc, PW_RENDERFULLCONTENT):
        gdi32.BitBlt(mem_dc, 0, 0, width, height, hwnd_dc, 0, 0, SRCCOPY)

    bitmap_info = BITMAPINFO()
    bitmap_info.bmiHeader.biSize = ctypes.sizeof(BITMAPINFOHEADER)
    bitmap_info.bmiHeader.biWidth = width
    bitmap_info.bmiHeader.biHeight = -height
    bitmap_info.bmiHeader.biPlanes = 1
    bitmap_info.bmiHeader.biBitCount = 32
    bitmap_info.bmiHeader.biCompression = 0

    pixels = ctypes.create_string_buffer(width * height * 4)
    copied_rows = gdi32.GetDIBits(mem_dc, bitmap, 0, height, pixels, ctypes.byref(bitmap_info), DIB_RGB_COLORS)
    gdi32.DeleteObject(bitmap)
    gdi32.DeleteDC(mem_dc)
    user32.ReleaseDC(wintypes.HWND(hwnd), hwnd_dc)
    if copied_rows != height:
        raise OSError(f"GetDIBits copied {copied_rows} rows, expected {height}")
    return pixels.raw


def _mean_abs_diff(left: bytes, right: bytes) -> float:
    assert len(left) == len(right)
    total = 0
    for left_byte, right_byte in zip(left, right):
        total += abs(left_byte - right_byte)
    return total / len(left)


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

        user32 = ctypes.windll.user32
        user32.ShowWindow(wintypes.HWND(hwnd), 5)
        user32.SetForegroundWindow(wintypes.HWND(hwnd))

        time.sleep(0.8)
        running_frame_a = _capture_window_pixels(hwnd)
        time.sleep(0.8)
        running_frame_b = _capture_window_pixels(hwnd)
        running_diff = _mean_abs_diff(running_frame_a, running_frame_b)
        assert running_diff > 1.0, f"live sweep did not visibly change the viewer image; diff={running_diff:.3f}"

        user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYDOWN, VK_SPACE, 0)
        user32.PostMessageW(wintypes.HWND(hwnd), WM_KEYUP, VK_SPACE, 0)

        time.sleep(0.5)
        paused_frame_a = _capture_window_pixels(hwnd)
        time.sleep(0.8)
        paused_frame_b = _capture_window_pixels(hwnd)
        paused_diff = _mean_abs_diff(paused_frame_a, paused_frame_b)
        assert paused_diff < 0.1, f"Space pause did not freeze the live sweep image; diff={paused_diff:.3f}"
        assert paused_diff < running_diff * 0.1, (
            "paused image was not substantially more stable than the running sweep; "
            f"running_diff={running_diff:.3f} paused_diff={paused_diff:.3f}"
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