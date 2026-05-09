from __future__ import annotations

import contextlib
import io
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

import tools.viewer_host_runtime_pytest_lane as runtime_pytest_lane


def test_extract_pytest_counts_parses_summary_fields() -> None:
    counts = runtime_pytest_lane.extract_pytest_counts("47 passed, 3 skipped in 1.23s\n")

    assert counts == {"passed": 47, "skipped": 3}


def test_default_test_files_match_probe_session_bundle() -> None:
    assert runtime_pytest_lane.DEFAULT_TEST_FILES == [
        "tests/test_fractal_runtime_batch_cli.py",
        "tests/test_fractal_runtime_probe_cli.py",
        "tests/test_fractal_runtime_session.py",
        "tests/test_function_descriptor_cli.py",
        "tests/test_generic_probe_cli.py",
    ]


def test_active_runtime_exe_requires_metadata_file(tmp_path: Path, monkeypatch) -> None:
    runtime_dir = tmp_path / "runtime"
    runtime_dir.mkdir()
    monkeypatch.setattr(runtime_pytest_lane.sys, "platform", "win32")

    try:
        runtime_pytest_lane.active_runtime_exe(
            runtime_dir=runtime_dir,
            active_runtime_file=runtime_dir / "fractal_ui_active.txt",
        )
        assert False, "expected missing runtime metadata to fail"
    except RuntimeError as exc:
        assert "missing active runtime metadata" in str(exc)


def test_main_fails_when_zero_tests_pass(monkeypatch) -> None:
    monkeypatch.setattr(runtime_pytest_lane, "active_runtime_exe", lambda: Path("D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe"))
    monkeypatch.setattr(
        runtime_pytest_lane.subprocess,
        "run",
        lambda *args, **kwargs: type("_Proc", (), {"returncode": 0, "stdout": "ssss\n4 skipped in 0.10s\n"})(),
    )

    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = runtime_pytest_lane.main([])

    output = buf.getvalue()
    assert rc == 2
    assert "zero tests passed" in output
    assert "summary=passed=0" in output


def test_main_succeeds_when_runtime_lane_has_passing_tests(monkeypatch) -> None:
    monkeypatch.setattr(runtime_pytest_lane, "active_runtime_exe", lambda: Path("D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe"))
    monkeypatch.setattr(
        runtime_pytest_lane.subprocess,
        "run",
        lambda *args, **kwargs: type("_Proc", (), {"returncode": 0, "stdout": "............\n12 passed in 0.62s\n"})(),
    )

    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = runtime_pytest_lane.main([])

    output = buf.getvalue()
    assert rc == 0
    assert "summary=passed=12" in output


def test_main_forwards_additional_pytest_args(monkeypatch) -> None:
    monkeypatch.setattr(runtime_pytest_lane, "active_runtime_exe", lambda: Path("D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe"))

    commands: list[list[str]] = []

    def fake_run(command: list[str], **kwargs):
        commands.append(command)
        return type("_Proc", (), {"returncode": 0, "stdout": ".\n1 passed in 0.10s\n"})()

    monkeypatch.setattr(runtime_pytest_lane.subprocess, "run", fake_run)

    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = runtime_pytest_lane.main([
            "tests/test_fractal_runtime_runtime_walk_viewer.py",
            "-k",
            "test_runtime_walk_viewer_replays_and_space_pauses",
        ])

    assert rc == 0
    assert commands == [[
        sys.executable,
        "-m",
        "pytest",
        "tests/test_fractal_runtime_runtime_walk_viewer.py",
        "-k",
        "test_runtime_walk_viewer_replays_and_space_pauses",
        "-q",
    ]]