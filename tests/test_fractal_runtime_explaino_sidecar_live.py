from __future__ import annotations

import ctypes
from ctypes import wintypes
import json
import subprocess
import sys
import time
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
DIAGNOSTICS_STATE_FILE = RUNTIME_DIR / "diagnostics" / "last" / "state.json"
DIAGNOSTICS_FRAME_FILE = RUNTIME_DIR / "diagnostics" / "last" / "frame.bmp"

WM_CLOSE = 0x0010
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
        rect = RECT()
        is_ready = user32.GetClientRect(wintypes.HWND(hwnd), ctypes.byref(rect))
        if (
            window_pid.value == pid
            and user32.IsWindow(wintypes.HWND(hwnd))
            and user32.IsWindowVisible(wintypes.HWND(hwnd))
            and is_ready
            and rect.right - rect.left > 0
            and rect.bottom - rect.top > 0
        ):
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
    if width <= 0 or height <= 0:
        raise OSError(f"window client rect is not ready: width={width} height={height}")
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


def _capture_ready_window_pixels(hwnd: int) -> bytes:
    deadline = time.monotonic() + 5.0
    last_error: Exception | None = None
    while time.monotonic() < deadline:
        try:
            pixels = _capture_window_pixels(hwnd)
        except OSError as exc:
            last_error = exc
            time.sleep(0.1)
            continue
        if pixels:
            return pixels
        time.sleep(0.1)

    if last_error is not None:
        raise last_error
    raise AssertionError("window never produced a non-empty captured frame")


def _mean_abs_diff(left: bytes, right: bytes) -> float:
    assert len(left) == len(right)
    total = 0
    for left_byte, right_byte in zip(left, right):
        total += abs(left_byte - right_byte)
    return total / len(left)


def _run_headless_capture(*args: str) -> dict[str, object]:
    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)

    result = subprocess.run(
        list(args),
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"
    assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing diagnostics frame file: {DIAGNOSTICS_FRAME_FILE}"
    return json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))


def _write_state_bundle(tmp_path: Path, state: dict[str, object]) -> Path:
    state_path = tmp_path / "state.json"
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
    return state_path


def _configure_sidecar_policy(state: dict[str, object], **updates: object) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    policy = configured_state["sidecar_auto_demo_policy"]
    assert isinstance(policy, dict)
    policy.update(updates)
    return configured_state


def _wait_for_window(proc: subprocess.Popen[object]) -> int:
    deadline = time.monotonic() + 10.0
    while time.monotonic() < deadline:
        hwnd = _find_window_for_pid(proc.pid)
        if hwnd is not None:
            return hwnd
        if proc.poll() is not None:
            raise AssertionError(f"viewer exited before creating a window; returncode={proc.returncode}")
        time.sleep(0.1)
    raise AssertionError(f"viewer never created a top-level window for pid {proc.pid}")


def _focus_window(hwnd: int) -> None:
    user32 = ctypes.windll.user32
    user32.ShowWindow(wintypes.HWND(hwnd), 5)
    user32.SetForegroundWindow(wintypes.HWND(hwnd))


def _close_runtime(hwnd: int | None, proc: subprocess.Popen[object]) -> None:
    if hwnd is not None:
        ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
    if proc.poll() is None:
        try:
            proc.wait(timeout=5.0)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait(timeout=5.0)


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
    baseline_state = _run_headless_capture(
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
            baseline_state,
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
    baseline_state = _run_headless_capture(
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
            baseline_state,
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