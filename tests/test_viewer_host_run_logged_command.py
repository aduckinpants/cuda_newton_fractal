from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
TOOL = REPO_ROOT / "tools" / "viewer_host_run_logged_command.py"


def test_logged_command_success_summary(tmp_path: Path) -> None:
    log_path = tmp_path / "success.log"
    result = subprocess.run(
        [
            sys.executable,
            str(TOOL),
            "--label",
            "success-smoke",
            "--log",
            str(log_path),
            "--tail",
            "2",
            "--",
            sys.executable,
            "-c",
            "print('alpha')\nprint('omega')",
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=False,
    )

    assert result.returncode == 0
    assert log_path.read_text(encoding="utf-8") == "alpha\nomega\n"
    assert "viewer_host_run_logged_command: label=success-smoke" in result.stdout
    assert "viewer_host_run_logged_command: exit=0" in result.stdout
    assert "viewer_host_run_logged_command: tail: alpha" in result.stdout
    assert "viewer_host_run_logged_command: tail: omega" in result.stdout


def test_logged_command_failure_preserves_exit_and_tail(tmp_path: Path) -> None:
    log_path = tmp_path / "failure.log"
    result = subprocess.run(
        [
            sys.executable,
            str(TOOL),
            "--label",
            "failure-smoke",
            "--log",
            str(log_path),
            "--tail",
            "1",
            "--",
            sys.executable,
            "-c",
            "import sys; print('before-fail'); sys.exit(7)",
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=False,
    )

    assert result.returncode == 7
    assert "before-fail" in log_path.read_text(encoding="utf-8")
    assert "viewer_host_run_logged_command: result=failure" in result.stdout
    assert "viewer_host_run_logged_command: exit=7" in result.stdout
    assert "viewer_host_run_logged_command: tail: before-fail" in result.stdout


def test_logged_command_missing_executable_still_summarizes(tmp_path: Path) -> None:
    log_path = tmp_path / "missing.log"
    result = subprocess.run(
        [
            sys.executable,
            str(TOOL),
            "--label",
            "missing-smoke",
            "--log",
            str(log_path),
            "--tail",
            "2",
            "--",
            "definitely_missing_executable_for_viewer_host_wrapper_test",
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=False,
    )

    assert result.returncode == 127
    log_text = log_path.read_text(encoding="utf-8")
    assert "failed to launch command" in log_text
    assert "viewer_host_run_logged_command: result=launch-failure" in result.stdout
    assert "viewer_host_run_logged_command: exit=127" in result.stdout
    assert "viewer_host_run_logged_command: tail: viewer_host_run_logged_command: failed to launch command:" in result.stdout


def test_logged_command_runs_relative_batch_script_without_dot_slash(tmp_path: Path) -> None:
    if os.name != "nt":
        return

    work_dir = tmp_path / "work"
    script_dir = work_dir / "nested"
    script_dir.mkdir(parents=True)
    script_path = script_dir / "sample.cmd"
    script_path.write_text("@echo off\r\necho batch-ok\r\n", encoding="utf-8")
    log_path = tmp_path / "batch.log"

    result = subprocess.run(
        [
            sys.executable,
            str(TOOL),
            "--label",
            "batch-path-smoke",
            "--cwd",
            str(work_dir),
            "--log",
            str(log_path),
            "--tail",
            "1",
            "--",
            "nested/sample.cmd",
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=False,
    )

    assert result.returncode == 0
    assert "batch-ok" in log_path.read_text(encoding="utf-8")
    assert "viewer_host_run_logged_command: exit=0" in result.stdout