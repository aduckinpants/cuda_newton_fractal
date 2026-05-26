from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

try:
    from tools.viewer_host_append_handoff import build_handoff_append_commands
    from tools.viewer_host_checkpoint_guard import discover_repo_root
    from tools.viewer_host_contract_proof import validation_evidence_spec_for_command
    from tools.viewer_host_contract_state import file_path_is_in_contract_scope, load_and_validate_slice_contract, validate_locked_contract_state
    from tools.viewer_host_validate_hostile_audit import plan_declares_hostile_audit, validate_hostile_audit_plan
except ModuleNotFoundError:
    from viewer_host_append_handoff import build_handoff_append_commands
    from viewer_host_checkpoint_guard import discover_repo_root
    from viewer_host_contract_proof import validation_evidence_spec_for_command
    from viewer_host_contract_state import file_path_is_in_contract_scope, load_and_validate_slice_contract, validate_locked_contract_state
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


def _scoped_commit_paths(paths: list[str]) -> list[str]:
    if not paths:
        return []
    scoped: list[str] = []
    for path in [*paths, "HANDOFF_LOG.md"]:
        candidate = str(path).strip()
        if candidate and candidate not in scoped:
            scoped.append(candidate)
    return scoped


def _validate_scoped_commit_paths(contract_state: dict[str, object] | None, repo_root: Path, paths: list[str]) -> str:
    for path in paths:
        if not file_path_is_in_contract_scope(path, contract_state, repo_root):
            return f"scoped checkpoint path outside contract scope: {path}"
    return ""


def _append_error_section(lines: list[str], title: str, items: list[str]) -> None:
    if not items:
        return
    lines.append(f"- {title}:")
    for item in items:
        lines.append(f"  - {item}")


def _collect_write_receipts_preflight_errors(
    contract_state: dict[str, object],
    repo_root: Path,
    validation_commands: list[str],
) -> list[str]:
    contract_path_text = str(contract_state.get("contract_path", "")).strip()
    if not contract_path_text:
        return ["- active contract path is missing from locked contract state"]

    contract_path = repo_root / contract_path_text
    contract_payload, contract_result = load_and_validate_slice_contract(contract_path, repo_root)
    if contract_payload is None or not bool(getattr(contract_result, "ok", False)):
        errors = ["- active contract failed validation"]
        for error in list(getattr(contract_result, "errors", []) or []):
            errors.append(f"  - {error}")
        return errors

    required_commands = [
        str(command)
        for command in list(contract_payload.get("required_validation_commands", []) or [])
        if str(command).strip()
    ]
    provided_commands = [str(command) for command in validation_commands if str(command).strip()]
    provided_set = set(provided_commands)

    missing_required = [command for command in required_commands if command not in provided_set]
    missing_parseable_evidence = [
        command
        for command in required_commands
        if command in provided_set and validation_evidence_spec_for_command(command) is None
    ]

    missing_artifacts: list[str] = []
    for command in provided_commands:
        spec = validation_evidence_spec_for_command(command)
        if spec is None:
            continue
        artifact_path = repo_root / spec.artifact_path
        if not artifact_path.exists():
            missing_artifacts.append(f"{command} | expected {spec.artifact_path}")

    lines: list[str] = []
    _append_error_section(lines, "missing required validation commands", missing_required)
    _append_error_section(lines, "missing parseable evidence for required validation commands", missing_parseable_evidence)
    _append_error_section(lines, "missing validation artifacts for provided commands", missing_artifacts)
    return lines


def _write_receipts_preflight_error_text(errors: list[str]) -> str:
    if not errors:
        return ""
    return (
        "viewer_host_checkpoint_slice: write-receipts preflight failed; "
        "no validation receipt was written\n"
        + "\n".join(errors)
        + "\n"
    )


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
    commit_parser.add_argument("--path", action="append", default=[], help="Optional repo-relative path to include in the checkpoint commit; repeat for multiple paths")

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
        commit_paths = _scoped_commit_paths(args.path)
        scope_error = _validate_scoped_commit_paths(_contract_state or {}, repo_root, commit_paths) if commit_paths else ""
        if scope_error:
            sys.stderr.write("viewer_host_checkpoint_slice: " + scope_error + "\n")
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
        if commit_paths:
            stage_command = ["git", "add", "--", *commit_paths]
            commit_command = ["git", "commit", "-m", args.commit_message, "--", *commit_paths]
        else:
            stage_command = ["git", "add", "-A"]
            commit_command = ["git", "commit", "-m", args.commit_message]
        stage_rc = subprocess.run(stage_command, cwd=str(repo_root), check=False).returncode
        if stage_rc != 0:
            return int(stage_rc)
        commit_rc = subprocess.run(commit_command, cwd=str(repo_root), check=False).returncode
        return int(commit_rc)

    hostile_audit_error = _validate_hostile_audit_before_commit(_contract_state or {}, repo_root)
    if hostile_audit_error:
        sys.stderr.write("viewer_host_checkpoint_slice: hostile review is incomplete: " + hostile_audit_error + "\n")
        return 2

    preflight_errors = _collect_write_receipts_preflight_errors(
        _contract_state or {},
        repo_root,
        list(args.validation_command),
    )
    if preflight_errors:
        sys.stderr.write(_write_receipts_preflight_error_text(preflight_errors))
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
