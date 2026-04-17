from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

try:
    from tools.viewer_host_checkpoint_guard import discover_repo_root
    from tools.viewer_host_contract_state import validate_locked_contract_state
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import discover_repo_root
    from viewer_host_contract_state import validate_locked_contract_state


def _normalize(text: str) -> str:
    return " ".join(text.replace("/", "\\").split()).lower()


def _is_allowed_delegate(command: list[str]) -> bool:
    normalized = _normalize(" ".join(command))
    allowed_wrappers = (
        "tools\\viewer_host_apply_repo_patch.py",
        "tools\\viewer_host_checkpoint_slice.py",
        "tools\\viewer_host_prepare_slice.py",
        "tools\\viewer_host_revise_contract.py",
    )
    return any(wrapper in normalized for wrapper in allowed_wrappers)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Run a repo mutation command only when an active contract is locked")
    parser.add_argument("--session-id", required=True, help="Current host session id")
    parser.add_argument("--cwd", default=".", help="Repo cwd")
    parser.add_argument("command", nargs=argparse.REMAINDER, help="Command to execute after --")
    args = parser.parse_args(argv)

    repo_root = discover_repo_root(Path(args.cwd))
    _contract_state, contract_error = validate_locked_contract_state(args.session_id, repo_root)
    if contract_error:
        sys.stderr.write(f"viewer_host_run_repo_mutation: {contract_error}\n")
        return 2

    command = list(args.command)
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        sys.stderr.write("viewer_host_run_repo_mutation: missing command\n")
        return 2
    if not _is_allowed_delegate(command):
        sys.stderr.write(
            "viewer_host_run_repo_mutation: only approved viewer_host_* wrapper delegates are allowed\n"
        )
        return 2

    proc = subprocess.run(command, cwd=str(repo_root), check=False)
    return int(proc.returncode)


if __name__ == "__main__":
    raise SystemExit(main())
