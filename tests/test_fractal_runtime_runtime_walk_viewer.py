from __future__ import annotations

import ctypes
from ctypes import wintypes
import json
import subprocess
import sys
import time
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[1]
RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"

WM_CLOSE = 0x0010
WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101
VK_SPACE = 0x20
VK_RIGHT = 0x27
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


def _write_state_json(path: Path) -> None:
    payload = {
        "state_version": 3,
        "fractal_type": "explaino_fp",
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
            {"id": "start", "t": 0.0, "channels": [0.0] * 13},
            {"id": "mid", "t": 0.5, "channels": [0.4, 0.2, 0.8, 0.3, 0.6, 0.75, 0.9, 0.1, 0.2, 0.7, 0.3, 0.4, 1.0]},
            {"id": "end", "t": 1.0, "channels": [1.0] * 13},
        ],
        "branch_markers": [
            {"id": "fork_a", "label": "fork-a", "parent_id": "main", "t": 0.5, "sticky_radius": 0.1}
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
    assert len(left) == len(right)
    total = 0
    for left_byte, right_byte in zip(left, right):
        total += abs(left_byte - right_byte)
    return total / len(left)


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
        user32.ShowWindow(wintypes.HWND(hwnd), 5)
        user32.SetForegroundWindow(wintypes.HWND(hwnd))

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
        assert stepped_diff > 0.08, f"Right-arrow step did not visibly advance runtime-walk playback; diff={stepped_diff:.3f}"
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

        user32 = ctypes.windll.user32
        user32.ShowWindow(wintypes.HWND(hwnd), 5)
        user32.SetForegroundWindow(wintypes.HWND(hwnd))

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
