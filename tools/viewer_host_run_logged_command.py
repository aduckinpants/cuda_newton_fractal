from __future__ import annotations

import argparse
import collections
import json
import os
import shlex
import signal
import subprocess
import sys
import time
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
WINDOWS_BATCH_SUFFIXES = {".bat", ".cmd"}


def _relative_to_repo(path: Path) -> str:
    try:
        return path.resolve().relative_to(REPO_ROOT.resolve()).as_posix()
    except ValueError:
        return path.resolve().as_posix()


def _tail_lines(path: Path, line_count: int) -> list[str]:
    if line_count <= 0 or not path.exists():
        return []
    with path.open("r", encoding="utf-8", errors="replace") as handle:
        tail = collections.deque(handle, maxlen=line_count)
    return [line.rstrip("\n") for line in tail]


def _format_command(command: list[str]) -> str:
    return shlex.join(command)


def _emit_summary_line(message: str) -> None:
    print(f"viewer_host_run_logged_command: {message}", flush=True)


def _popen_process_group_kwargs() -> dict[str, object]:
    if os.name == "nt":
        return {}
    return {"start_new_session": True}


def _terminate_process_tree(proc: subprocess.Popen[str]) -> dict[str, object]:
    cleanup: dict[str, object] = {
        "attempted": True,
        "method": "taskkill" if os.name == "nt" else "killpg",
        "taskkill_exit_code": None,
        "wait_completed": False,
        "forced_kill_after_wait": False,
    }
    if proc.poll() is not None:
        cleanup["already_exited"] = True
        cleanup["wait_completed"] = True
        return cleanup

    if os.name == "nt":
        result = subprocess.run(
            ["taskkill", "/PID", str(proc.pid), "/T", "/F"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        cleanup["taskkill_exit_code"] = int(result.returncode)
    else:
        try:
            os.killpg(proc.pid, signal.SIGKILL)
        except ProcessLookupError:
            cleanup["already_exited"] = True
        except OSError:
            cleanup["method"] = "kill"
            proc.kill()

    try:
        proc.wait(timeout=5)
        cleanup["wait_completed"] = True
    except subprocess.TimeoutExpired:
        cleanup["forced_kill_after_wait"] = True
        proc.kill()
    return cleanup


def _normalize_command_for_launch(command: list[str], cwd: Path) -> list[str]:
    normalized = list(command)
    if os.name != "nt" or not normalized:
        return normalized

    entry = normalized[0].strip()
    if not entry:
        return normalized

    entry_path = Path(entry)
    if entry_path.suffix.lower() not in WINDOWS_BATCH_SUFFIXES:
        return normalized

    if not entry_path.is_absolute():
        entry_path = (cwd / entry_path).resolve()
    else:
        entry_path = entry_path.resolve()
    normalized[0] = str(entry_path)
    return normalized


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Run a command, write full stdout/stderr to a log file, and print a deterministic short summary.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--label", required=True, help="Short human-readable label for the command")
    parser.add_argument("--log", required=True, help="Path to the combined stdout/stderr log file")
    parser.add_argument("--cwd", default=str(REPO_ROOT), help="Working directory for the child command")
    parser.add_argument("--tail", type=int, default=12, help="Number of log tail lines to print in the summary")
    parser.add_argument("--heartbeat-seconds", type=float, default=30.0, help="Print a short running marker at this interval; use 0 to suppress")
    parser.add_argument("--timeout-seconds", type=float, default=0.0, help="Terminate the child process tree after this many seconds; use 0 for no timeout")
    parser.add_argument("--out-json", help="Optional structured summary JSON path")
    parser.add_argument("command", nargs=argparse.REMAINDER, help="Command to run, prefixed with --")
    args = parser.parse_args(argv)

    command = list(args.command)
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        parser.error("missing command after --")

    cwd = Path(args.cwd)
    if not cwd.is_absolute():
        cwd = (REPO_ROOT / cwd).resolve()
    command = _normalize_command_for_launch(command, cwd)

    log_path = Path(args.log)
    if not log_path.is_absolute():
        log_path = (REPO_ROOT / log_path).resolve()
    log_path.parent.mkdir(parents=True, exist_ok=True)

    _emit_summary_line(f"label={args.label}")
    _emit_summary_line(f"cwd={_relative_to_repo(cwd)}")
    _emit_summary_line(f"command={_format_command(command)}")
    _emit_summary_line(f"log={_relative_to_repo(log_path)}")
    _emit_summary_line("result=started")

    return_code = 0
    launch_failed = False
    timed_out = False
    cleanup_result: dict[str, object] | None = None
    started_at = time.monotonic()
    heartbeat_seconds = max(0.0, float(args.heartbeat_seconds))
    timeout_seconds = max(0.0, float(args.timeout_seconds))
    with log_path.open("w", encoding="utf-8", errors="replace") as handle:
        try:
            proc = subprocess.Popen(
                command,
                cwd=str(cwd),
                stdout=handle,
                stderr=subprocess.STDOUT,
                text=True,
                **_popen_process_group_kwargs(),
            )
            while True:
                elapsed = time.monotonic() - started_at
                if timeout_seconds and elapsed >= timeout_seconds:
                    timed_out = True
                    handle.write(f"viewer_host_run_logged_command: timeout after {timeout_seconds:.1f} seconds\n")
                    handle.flush()
                    cleanup_result = _terminate_process_tree(proc)
                    return_code = 124
                    break
                wait_timeout = heartbeat_seconds if heartbeat_seconds else None
                if timeout_seconds:
                    remaining = max(0.0, timeout_seconds - elapsed)
                    wait_timeout = remaining if wait_timeout is None else min(wait_timeout, remaining)
                try:
                    return_code = int(proc.wait(timeout=wait_timeout))
                    break
                except subprocess.TimeoutExpired:
                    elapsed_seconds = int(time.monotonic() - started_at)
                    _emit_summary_line(f"running elapsed_seconds={elapsed_seconds} log={_relative_to_repo(log_path)}")
        except OSError as exc:
            handle.write(f"viewer_host_run_logged_command: failed to launch command: {exc}\n")
            return_code = 127
            launch_failed = True

    elapsed_seconds = time.monotonic() - started_at

    if launch_failed:
        result_label = "launch-failure"
    elif timed_out:
        result_label = "timeout"
    elif return_code == 0:
        result_label = "success"
    else:
        result_label = "failure"

    _emit_summary_line(f"result={result_label}")
    _emit_summary_line(f"exit={return_code}")
    _emit_summary_line(f"elapsed_seconds={elapsed_seconds:.3f}")

    if args.out_json:
        out_json_path = Path(args.out_json)
        if not out_json_path.is_absolute():
            out_json_path = (REPO_ROOT / out_json_path).resolve()
        out_json_path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "ok": return_code == 0,
            "label": str(args.label),
            "cwd": _relative_to_repo(cwd),
            "command": command,
            "command_text": _format_command(command),
            "log": _relative_to_repo(log_path),
            "result": result_label,
            "exit_code": return_code,
            "elapsed_seconds": elapsed_seconds,
            "timeout_seconds": timeout_seconds,
            "timed_out": timed_out,
            "cleanup": cleanup_result,
        }
        out_json_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
        _emit_summary_line(f"json={_relative_to_repo(out_json_path)}")

    for line in _tail_lines(log_path, args.tail):
        _emit_summary_line(f"tail: {line}")

    return return_code


if __name__ == "__main__":
    raise SystemExit(main())
