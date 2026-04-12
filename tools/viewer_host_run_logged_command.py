from __future__ import annotations

import argparse
import collections
import os
import shlex
import subprocess
import sys
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

    return_code = 0
    with log_path.open("w", encoding="utf-8", errors="replace") as handle:
        try:
            proc = subprocess.run(
                command,
                cwd=str(cwd),
                stdout=handle,
                stderr=subprocess.STDOUT,
                text=True,
                check=False,
            )
            return_code = int(proc.returncode)
        except OSError as exc:
            handle.write(f"viewer_host_run_logged_command: failed to launch command: {exc}\n")
            return_code = 127

    print(f"viewer_host_run_logged_command: label={args.label}")
    print(f"viewer_host_run_logged_command: cwd={_relative_to_repo(cwd)}")
    print(f"viewer_host_run_logged_command: command={_format_command(command)}")
    print(f"viewer_host_run_logged_command: log={_relative_to_repo(log_path)}")
    print(f"viewer_host_run_logged_command: exit={return_code}")

    for line in _tail_lines(log_path, args.tail):
        print(f"viewer_host_run_logged_command: tail: {line}")

    return return_code


if __name__ == "__main__":
    raise SystemExit(main())
