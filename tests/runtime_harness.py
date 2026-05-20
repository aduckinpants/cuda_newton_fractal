from __future__ import annotations

from contextlib import contextmanager
import ctypes
from ctypes import wintypes
from dataclasses import dataclass
import hashlib
import json
import os
import subprocess
import time
from pathlib import Path
from typing import Any

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
DIAGNOSTICS_STATE_FILE = RUNTIME_DIR / "diagnostics" / "last" / "state.json"
DIAGNOSTICS_FRAME_FILE = RUNTIME_DIR / "diagnostics" / "last" / "frame.bmp"
RUNTIME_AUTOMATION_LOCK_FILE = RUNTIME_DIR / ".runtime_ui_automation.lock"

WM_CLOSE = 0x0010
SRCCOPY = 0x00CC0020
DIB_RGB_COLORS = 0
PW_RENDERFULLCONTENT = 0x00000002
RUNTIME_AUTOMATION_LOCK_STALE_SECONDS = 300.0


class RECT(ctypes.Structure):
    _fields_ = [
        ("left", ctypes.c_long),
        ("top", ctypes.c_long),
        ("right", ctypes.c_long),
        ("bottom", ctypes.c_long),
    ]


class POINT(ctypes.Structure):
    _fields_ = [
        ("x", ctypes.c_long),
        ("y", ctypes.c_long),
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


@dataclass(frozen=True)
class HeadlessLoadedStateScenario:
    name: str
    state: dict[str, object]
    action_args: tuple[str, ...]


@dataclass(frozen=True)
class HeadlessLoadedStateScenarioResult:
    state_path: Path
    baseline_capture: dict[str, object]
    scenario_capture: dict[str, object]


@dataclass(frozen=True)
class UiAutomationRect:
    control_id: str
    screen_left: int
    screen_top: int
    screen_right: int
    screen_bottom: int

    @property
    def width(self) -> int:
        return self.screen_right - self.screen_left

    @property
    def height(self) -> int:
        return self.screen_bottom - self.screen_top


@contextmanager
def runtime_automation_lock(*, timeout_seconds: float = 120.0):
    RUNTIME_DIR.mkdir(parents=True, exist_ok=True)
    deadline = time.monotonic() + timeout_seconds
    fd: int | None = None
    while time.monotonic() < deadline:
        try:
            fd = os.open(str(RUNTIME_AUTOMATION_LOCK_FILE), os.O_CREAT | os.O_EXCL | os.O_RDWR)
            os.write(fd, f"pid={os.getpid()} acquired_at={time.time():.6f}\n".encode("ascii"))
            break
        except FileExistsError:
            try:
                lock_age = time.time() - RUNTIME_AUTOMATION_LOCK_FILE.stat().st_mtime
            except OSError:
                lock_age = 0.0
            if lock_age > RUNTIME_AUTOMATION_LOCK_STALE_SECONDS:
                try:
                    RUNTIME_AUTOMATION_LOCK_FILE.unlink()
                    continue
                except OSError:
                    pass
            time.sleep(0.1)
    if fd is None:
        raise AssertionError(f"runtime UI automation lock was not acquired: {RUNTIME_AUTOMATION_LOCK_FILE}")
    try:
        yield
    finally:
        os.close(fd)
        try:
            RUNTIME_AUTOMATION_LOCK_FILE.unlink()
        except FileNotFoundError:
            pass


def active_runtime_exe() -> Path:
    if not ACTIVE_RUNTIME_FILE.exists():
        pytest.skip(f"missing active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    active_name = ACTIVE_RUNTIME_FILE.read_text(encoding="utf-8").strip()
    if not active_name:
        pytest.skip(f"empty active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    exe_path = RUNTIME_DIR / active_name
    if not exe_path.exists():
        pytest.skip(f"active runtime missing: {exe_path}")
    return exe_path


def run_headless_capture(*args: str) -> dict[str, object]:
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
    frame_bytes = DIAGNOSTICS_FRAME_FILE.read_bytes()

    return {
        "state": json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8")),
        "frame_hash": hashlib.sha256(frame_bytes).hexdigest(),
        "frame_bytes": frame_bytes,
    }


def capture_explaino_runtime_baseline(exe_path: Path, *, width: int = 320, height: int = 240) -> dict[str, object]:
    return run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        str(width),
        "--height",
        str(height),
    )


def write_state_bundle(tmp_path: Path, state: dict[str, object]) -> Path:
    state_path = tmp_path / "state.json"
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
    return state_path


def run_headless_loaded_state_scenario(
    tmp_path: Path,
    *,
    exe_path: Path,
    scenario: HeadlessLoadedStateScenario,
) -> HeadlessLoadedStateScenarioResult:
    state_path = write_state_bundle(tmp_path / scenario.name, scenario.state)
    baseline_capture = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--capture-diagnostic",
    )
    scenario_capture = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        *scenario.action_args,
        "--capture-diagnostic",
    )
    return HeadlessLoadedStateScenarioResult(
        state_path=state_path,
        baseline_capture=baseline_capture,
        scenario_capture=scenario_capture,
    )


def find_window_for_pid(pid: int) -> int | None:
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


def wait_for_window(proc: subprocess.Popen[object], *, timeout_seconds: float = 10.0) -> int:
    deadline = time.monotonic() + timeout_seconds
    while time.monotonic() < deadline:
        hwnd = find_window_for_pid(proc.pid)
        if hwnd is not None:
            return hwnd
        if proc.poll() is not None:
            raise AssertionError(f"viewer exited before creating a window; returncode={proc.returncode}")
        time.sleep(0.1)
    raise AssertionError(f"viewer never created a top-level window for pid {proc.pid}")


def focus_window(hwnd: int) -> None:
    user32 = ctypes.windll.user32
    user32.ShowWindow(wintypes.HWND(hwnd), 5)
    user32.SetForegroundWindow(wintypes.HWND(hwnd))


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


def capture_ready_window_pixels(hwnd: int, *, timeout_seconds: float = 5.0) -> bytes:
    deadline = time.monotonic() + timeout_seconds
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


def mean_abs_diff(left: bytes, right: bytes) -> float:
    assert len(left) == len(right)
    total = 0
    for left_byte, right_byte in zip(left, right):
        total += abs(left_byte - right_byte)
    return total / len(left)


def close_runtime(hwnd: int | None, proc: subprocess.Popen[object]) -> None:
    if hwnd is not None:
        ctypes.windll.user32.PostMessageW(wintypes.HWND(hwnd), WM_CLOSE, 0, 0)
    if proc.poll() is None:
        try:
            proc.wait(timeout=5.0)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait(timeout=5.0)


def wait_for_ui_automation_rect(
    report_path: Path,
    control_id: str,
    *,
    timeout_seconds: float = 10.0,
) -> UiAutomationRect:
    deadline = time.monotonic() + timeout_seconds
    last_payload: object | None = None
    while time.monotonic() < deadline:
        if report_path.exists():
            try:
                payload = json.loads(report_path.read_text(encoding="utf-8"))
            except (FileNotFoundError, json.JSONDecodeError, OSError):
                time.sleep(0.05)
                continue
            last_payload = payload
            controls = payload.get("controls", []) if isinstance(payload, dict) else []
            for control in controls:
                if not isinstance(control, dict):
                    continue
                if str(control.get("control_id", "")).strip() != control_id:
                    continue
                rect = control.get("screen_rect")
                if not isinstance(rect, list) or len(rect) != 4:
                    continue
                left, top, right, bottom = [int(value) for value in rect]
                ui_rect = UiAutomationRect(control_id, left, top, right, bottom)
                if ui_rect.width > 0 and ui_rect.height > 0:
                    return ui_rect
        time.sleep(0.1)

    raise AssertionError(
        f"UI automation report never exposed control '{control_id}' in {report_path}; last_payload={last_payload!r}"
    )


class PersistentRuntimeViewerAutomation:
    def __init__(
        self,
        *,
        exe_path: Path,
        state_path: Path,
        report_path: Path,
        command_path: Path,
        open_color_pipeline: bool = False,
    ) -> None:
        self.exe_path = exe_path
        self.state_path = state_path
        self.report_path = report_path
        self.command_path = command_path
        self.open_color_pipeline = open_color_pipeline
        self.proc: subprocess.Popen[object] | None = None
        self.hwnd: int | None = None
        self.sequence = 0
        self.launch_count = 0

    def __enter__(self) -> "PersistentRuntimeViewerAutomation":
        self.report_path.unlink(missing_ok=True)
        self.command_path.unlink(missing_ok=True)
        args = [
            str(self.exe_path),
            "--load-state-json",
            str(self.state_path),
            "--ui-automation-report-json",
            str(self.report_path),
            "--ui-automation-command-json",
            str(self.command_path),
        ]
        if self.open_color_pipeline:
            args.append("--open-color-pipeline-window")
        self.proc = subprocess.Popen(args, cwd=str(RUNTIME_DIR))
        self.launch_count += 1
        self.hwnd = wait_for_window(self.proc)
        return self

    def __exit__(self, _exc_type: object, _exc: object, _tb: object) -> None:
        if self.proc is not None:
            close_runtime(self.hwnd, self.proc)
        self.proc = None
        self.hwnd = None

    def _load_report(self) -> dict[str, Any] | None:
        try:
            payload = json.loads(self.report_path.read_text(encoding="utf-8"))
        except (FileNotFoundError, json.JSONDecodeError, OSError):
            return None
        return payload if isinstance(payload, dict) else None

    def wait_for_report(self, *, timeout_seconds: float = 10.0) -> dict[str, Any]:
        deadline = time.monotonic() + timeout_seconds
        last_payload: dict[str, Any] | None = None
        while time.monotonic() < deadline:
            if self.proc is not None and self.proc.poll() is not None:
                raise AssertionError(f"viewer exited while waiting for automation report; returncode={self.proc.returncode}")
            payload = self._load_report()
            if payload is not None:
                last_payload = payload
                if payload.get("rendered_frame_ready") is True and isinstance(payload.get("rendered_frame_hash"), str):
                    return payload
            time.sleep(0.05)
        raise AssertionError(f"persistent viewer report never reached ready frame; last_payload={last_payload!r}")

    def wait_for_control(self, control_id: str, *, timeout_seconds: float = 10.0) -> UiAutomationRect:
        return wait_for_ui_automation_rect(self.report_path, control_id, timeout_seconds=timeout_seconds)

    def load_state_json(self, state_path: Path, *, expected_fractal_type: str | None = None, timeout_seconds: float = 10.0) -> dict[str, Any]:
        self.sequence += 1
        command = {
            "sequence": self.sequence,
            "load_state_json": str(state_path),
        }
        self.command_path.write_text(json.dumps(command, indent=2), encoding="utf-8")
        deadline = time.monotonic() + timeout_seconds
        last_payload: dict[str, Any] | None = None
        while time.monotonic() < deadline:
            if self.proc is not None and self.proc.poll() is not None:
                raise AssertionError(f"viewer exited while waiting for load-state command {self.sequence}; returncode={self.proc.returncode}")
            payload = self._load_report()
            if payload is None:
                time.sleep(0.05)
                continue
            last_payload = payload
            if payload.get("ui_automation_command_sequence") != self.sequence:
                time.sleep(0.05)
                continue
            if expected_fractal_type is not None and payload.get("current_fractal_type") != expected_fractal_type:
                time.sleep(0.05)
                continue
            if payload.get("rendered_frame_ready") is True:
                return payload
            time.sleep(0.05)
        raise AssertionError(
            f"persistent viewer never loaded state command {self.sequence} for {state_path}; last_payload={last_payload!r}"
        )

    def set_enum_id(
        self,
        path: str,
        enum_id: str,
        *,
        expected_fractal_type: str | None = None,
        timeout_seconds: float = 10.0,
    ) -> dict[str, Any]:
        self.sequence += 1
        command = {
            "sequence": self.sequence,
            "set_enum_id": {
                "path": path,
                "id": enum_id,
            },
        }
        self.command_path.write_text(json.dumps(command, indent=2), encoding="utf-8")
        deadline = time.monotonic() + timeout_seconds
        last_payload: dict[str, Any] | None = None
        while time.monotonic() < deadline:
            if self.proc is not None and self.proc.poll() is not None:
                raise AssertionError(f"viewer exited while waiting for enum command {self.sequence}; returncode={self.proc.returncode}")
            payload = self._load_report()
            if payload is not None:
                last_payload = payload
                if payload.get("ui_automation_command_sequence") == self.sequence:
                    if expected_fractal_type is None or payload.get("current_fractal_type") == expected_fractal_type:
                        if payload.get("rendered_frame_ready") is True:
                            return payload
            time.sleep(0.05)
        raise AssertionError(f"persistent viewer never consumed enum command {self.sequence} for {path}={enum_id}; last_payload={last_payload!r}")

    def set_control_value(self, control_id: str, value: float, *, timeout_seconds: float = 10.0) -> dict[str, Any]:
        self.sequence += 1
        command = {
            "sequence": self.sequence,
            "set_control_value": {
                "control_id": control_id,
                "value": value,
            },
        }
        self.command_path.write_text(json.dumps(command, indent=2), encoding="utf-8")
        deadline = time.monotonic() + timeout_seconds
        last_payload: dict[str, Any] | None = None
        while time.monotonic() < deadline:
            if self.proc is not None and self.proc.poll() is not None:
                raise AssertionError(f"viewer exited while waiting for command {self.sequence}; returncode={self.proc.returncode}")
            payload = self._load_report()
            if payload is None:
                time.sleep(0.05)
                continue
            last_payload = payload
            if payload.get("ui_automation_command_sequence") != self.sequence:
                time.sleep(0.05)
                continue
            if payload.get("requested_set_control_id") != control_id:
                time.sleep(0.05)
                continue
            set_error = payload.get("set_value_error")
            assert not set_error, f"set-value automation failed for {control_id}: {set_error}"
            if payload.get("set_value_consumed") is True and payload.get("rendered_frame_ready") is True:
                return payload
            time.sleep(0.05)
        raise AssertionError(f"persistent viewer never consumed command {self.sequence} for {control_id}; last_payload={last_payload!r}")
