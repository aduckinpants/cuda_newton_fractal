from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

try:
    from tools.viewer_host_append_handoff import build_handoff_append_commands
    from tools.viewer_host_checkpoint_guard import discover_repo_root
    from tools.viewer_host_contract_state import validate_locked_contract_state
    from tools.viewer_host_validate_hostile_audit import plan_declares_hostile_audit, validate_hostile_audit_plan
except ModuleNotFoundError:
    from viewer_host_append_handoff import build_handoff_append_commands
    from viewer_host_checkpoint_guard import discover_repo_root
    from viewer_host_contract_state import validate_locked_contract_state
    from viewer_host_validate_hostile_audit import plan_declares_hostile_audit, validate_hostile_audit_plan


def _contract_requires_hostile_audit(contract_state: dict[str, object] | None) -> bool:
    if not isinstance(contract_state, dict):
        return False
    commands = contract_state.get("required_validation_commands", [])
    if not isinstance(commands, list):
        return False
    return any(
        isinstance(command, str) and "tools/viewer_host_validate_hostile_audit.py" in command.replace("\\", "/")
        for command in commands
    )


def _validate_hostile_audit_before_commit(contract_state: dict[str, object], repo_root: Path) -> str:
    plan_path_text = str(contract_state.get("plan_path", "")).strip()
    if not plan_path_text:
        return ""
    plan_path = repo_root / plan_path_text
    if not plan_path.exists():
        return f"hostile-audit validation plan is missing: {plan_path_text}"
    if not plan_declares_hostile_audit(plan_path) and not _contract_requires_hostile_audit(contract_state):
        return ""
    payload = validate_hostile_audit_plan(plan_path)
    if bool(payload.get("ok", False)):
        return ""
    return str(payload.get("blocked_reason", "")).strip() or "hostile review is incomplete"


def _run_all(commands: list[list[str]], cwd: Path) -> int:
    for command in commands:
        proc = subprocess.run(command, cwd=str(cwd), check=False)
        if proc.returncode != 0:
            return int(proc.returncode)
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Repo-enforced checkpoint wrapper for handoff append and commit")
    subparsers = parser.add_subparsers(dest="mode", required=True)

    commit_parser = subparsers.add_parser("commit")
    commit_parser.add_argument("--session-id", required=True)
    commit_parser.add_argument("--cwd", default=".")
    commit_parser.add_argument("--checkpoint-id", required=True)
    commit_parser.add_argument("--score", required=True, type=int)
    commit_parser.add_argument("--handoff-message", required=True)
    commit_parser.add_argument("--commit-message", required=True)

    receipt_parser = subparsers.add_parser("write-receipts")
    receipt_parser.add_argument("--session-id", required=True)
    receipt_parser.add_argument("--cwd", default=".")
    receipt_parser.add_argument("--validation-summary", required=True)
    receipt_parser.add_argument("--validation-command", action="append", default=[])

    args = parser.parse_args(argv)
    repo_root = discover_repo_root(Path(args.cwd))
    _contract_state, contract_error = validate_locked_contract_state(args.session_id, repo_root)
    if contract_error:
        sys.stderr.write(f"viewer_host_checkpoint_slice: {contract_error}\n")
        return 2

    if args.mode == "commit":
        hostile_audit_error = _validate_hostile_audit_before_commit(_contract_state or {}, repo_root)
        if hostile_audit_error:
            sys.stderr.write("viewer_host_checkpoint_slice: hostile review is incomplete: " + hostile_audit_error + "\n")
            return 2
        commands = build_handoff_append_commands(
            py=sys.executable,
            message=args.handoff_message,
            commit=args.checkpoint_id,
            resolve_last_pending=False,
            score=args.score,
            repo_root=repo_root,
        )
        rc = _run_all(commands, repo_root)
        if rc != 0:
            return rc
        stage_rc = subprocess.run(["git", "add", "-A"], cwd=str(repo_root), check=False).returncode
        if stage_rc != 0:
            return int(stage_rc)
        commit_rc = subprocess.run(["git", "commit", "-m", args.commit_message], cwd=str(repo_root), check=False).returncode
        return int(commit_rc)

    hostile_audit_error = _validate_hostile_audit_before_commit(_contract_state or {}, repo_root)
    if hostile_audit_error:
        sys.stderr.write("viewer_host_checkpoint_slice: hostile review is incomplete: " + hostile_audit_error + "\n")
        return 2

    validation_command_args: list[str] = []
    for command in args.validation_command:
        validation_command_args.extend(["--command", command])
    validation_rc = subprocess.run(
        [
            sys.executable,
            "tools/viewer_host_write_validation_receipt.py",
            "--summary",
            args.validation_summary,
            *validation_command_args,
        ],
        cwd=str(repo_root),
        check=False,
    ).returncode
    if validation_rc != 0:
        return int(validation_rc)
    proof_rc = subprocess.run(
        [
            sys.executable,
            "tools/viewer_host_write_contract_proof_receipt.py",
            "--session-id",
            args.session_id,
            "--cwd",
            str(repo_root),
        ],
        cwd=str(repo_root),
        check=False,
    ).returncode
    return int(proof_rc)


if __name__ == "__main__":
    raise SystemExit(main())
