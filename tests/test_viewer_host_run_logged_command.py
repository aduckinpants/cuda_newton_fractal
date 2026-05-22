from __future__ import annotations

import json
import os
import subprocess
import sys
import time
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
TOOL = REPO_ROOT / "tools" / "viewer_host_run_logged_command.py"


def _pid_is_running(pid: int) -> bool:
    if os.name == "nt":
        result = subprocess.run(
            [
                "powershell",
                "-NoProfile",
                "-Command",
                f"if (Get-Process -Id {pid} -ErrorAction SilentlyContinue) {{ exit 0 }} else {{ exit 1 }}",
            ],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        return result.returncode == 0
    try:
        os.kill(pid, 0)
    except ProcessLookupError:
        return False
    except PermissionError:
        return True
    return True


def _wait_until_pid_exits(pid: int, timeout_seconds: float = 5.0) -> bool:
    deadline = time.monotonic() + timeout_seconds
    while time.monotonic() < deadline:
        if not _pid_is_running(pid):
            return True
        time.sleep(0.05)
    return not _pid_is_running(pid)


def _cleanup_pid(pid: int) -> None:
    if not _pid_is_running(pid):
        return
    if os.name == "nt":
        subprocess.run(
            ["taskkill", "/PID", str(pid), "/T", "/F"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
    else:
        try:
            os.kill(pid, 9)
        except ProcessLookupError:
            pass


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
    assert "viewer_host_run_logged_command: result=started" in result.stdout
    assert result.stdout.index("viewer_host_run_logged_command: result=started") < result.stdout.index("viewer_host_run_logged_command: exit=0")
    assert "viewer_host_run_logged_command: exit=0" in result.stdout
    assert "viewer_host_run_logged_command: tail: alpha" in result.stdout
    assert "viewer_host_run_logged_command: tail: omega" in result.stdout


def test_logged_command_emits_running_heartbeat_for_long_child(tmp_path: Path) -> None:
    log_path = tmp_path / "heartbeat.log"
    result = subprocess.run(
        [
            sys.executable,
            str(TOOL),
            "--label",
            "heartbeat-smoke",
            "--log",
            str(log_path),
            "--tail",
            "1",
            "--heartbeat-seconds",
            "0.05",
            "--",
            sys.executable,
            "-c",
            "import time; print('begin'); time.sleep(0.2); print('end')",
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=False,
    )

    assert result.returncode == 0
    assert "viewer_host_run_logged_command: result=started" in result.stdout
    assert "viewer_host_run_logged_command: running elapsed_seconds=" in result.stdout
    assert "viewer_host_run_logged_command: tail: end" in result.stdout


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


def test_timeout_kills_child_process_tree(tmp_path: Path) -> None:
    log_path = tmp_path / "timeout.log"
    json_path = tmp_path / "timeout.json"
    pid_path = tmp_path / "child.pid"
    script_path = tmp_path / "spawn_child.py"
    script_path.write_text(
        "from pathlib import Path\n"
        "import subprocess\n"
        "import sys\n"
        "import time\n"
        "pid_path = Path(sys.argv[1])\n"
        "child = subprocess.Popen([sys.executable, '-c', 'import time; time.sleep(30)'])\n"
        "pid_path.write_text(str(child.pid), encoding='utf-8')\n"
        "time.sleep(30)\n",
        encoding="utf-8",
    )

    result = subprocess.run(
        [
            sys.executable,
            str(TOOL),
            "--label",
            "timeout-tree-smoke",
            "--log",
            str(log_path),
            "--out-json",
            str(json_path),
            "--tail",
            "3",
            "--heartbeat-seconds",
            "0",
            "--timeout-seconds",
            "1",
            "--",
            sys.executable,
            str(script_path),
            str(pid_path),
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=False,
    )

    assert result.returncode == 124
    assert "viewer_host_run_logged_command: result=timeout" in result.stdout
    assert "viewer_host_run_logged_command: elapsed_seconds=" in result.stdout
    payload = json.loads(json_path.read_text(encoding="utf-8"))
    assert payload["result"] == "timeout"
    assert payload["timed_out"] is True
    assert payload["exit_code"] == 124
    assert payload["cleanup"]["attempted"] is True
    assert pid_path.exists(), log_path.read_text(encoding="utf-8")
    child_pid = int(pid_path.read_text(encoding="utf-8"))
    try:
        assert _wait_until_pid_exits(child_pid), f"child process survived timeout cleanup: {child_pid}"
    finally:
        _cleanup_pid(child_pid)


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