from __future__ import annotations

import argparse
import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

try:
    from tools.viewer_host_checkpoint_guard import (
        REPO_ROOT,
        capture_repo_snapshot,
        compare_snapshots,
        evaluate_checkpoint_guard,
        recovery_dir,
        recovery_helper_command,
        recovery_report_path_for_snapshot,
        snapshot_digest,
        snapshot_is_clean,
        summarize_changed_paths,
        validation_receipt_path,
        write_active_recovery_adoption,
    )
    from tools.viewer_host_contract_state import (
        GLOBAL_CONTRACT_SESSION_ID,
        contract_proof_receipt_path,
        file_path_is_in_contract_scope,
        load_active_contract_state,
        validate_locked_contract_state,
    )
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import (
        REPO_ROOT,
        capture_repo_snapshot,
        compare_snapshots,
        evaluate_checkpoint_guard,
        recovery_dir,
        recovery_helper_command,
        recovery_report_path_for_snapshot,
        snapshot_digest,
        snapshot_is_clean,
        summarize_changed_paths,
        validation_receipt_path,
        write_active_recovery_adoption,
    )
    from viewer_host_contract_state import (
        GLOBAL_CONTRACT_SESSION_ID,
        contract_proof_receipt_path,
        file_path_is_in_contract_scope,
        load_active_contract_state,
        validate_locked_contract_state,
    )


def _run_git(repo_root: Path, *args: str) -> str:
    import subprocess

    proc = subprocess.run(
        ["git", *args],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout or "git command failed").strip()
        raise RuntimeError(detail)
    return (proc.stdout or "").strip()


def _latest_handoff_entries(repo_root: Path, *, limit: int = 4) -> list[str]:
    handoff_path = repo_root / "HANDOFF_LOG.md"
    if not handoff_path.exists():
        return []
    entries = [line.strip()[2:] for line in handoff_path.read_text(encoding="utf-8").splitlines() if line.strip().startswith("- `")]
    return entries[-limit:]


def _classify_paths(
    paths: list[str],
    contract_state: dict[str, Any] | None,
    repo_root: Path,
) -> tuple[list[str], list[str]]:
    in_scope: list[str] = []
    out_of_scope: list[str] = []
    for path in paths:
        if file_path_is_in_contract_scope(path, contract_state, repo_root):
            in_scope.append(path)
        else:
            out_of_scope.append(path)
    return in_scope, out_of_scope


def build_recovery_report(
    *,
    repo_root: Path,
    summary: str,
    snapshot: dict[str, Any],
) -> dict[str, Any]:
    changed_paths = compare_snapshots({"unstaged": {}, "staged": {}, "untracked": {}}, snapshot)
    guard_status = evaluate_checkpoint_guard(None, snapshot)
    active_contract = load_active_contract_state(GLOBAL_CONTRACT_SESSION_ID, repo_root)
    locked_contract_state, contract_error = validate_locked_contract_state(GLOBAL_CONTRACT_SESSION_ID, repo_root)
    in_scope, out_of_scope = _classify_paths(changed_paths, active_contract, repo_root)
    head = str(snapshot.get("head", "")).strip()
    validation_path = validation_receipt_path(head, repo_root)
    contract_proof_path = contract_proof_receipt_path(head, repo_root)

    return {
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "summary": summary,
        "reason": guard_status.reason or "Recovery helper invoked manually.",
        "repo_root": repo_root.as_posix(),
        "recovery_command": recovery_helper_command(),
        "git": {
            "branch": _run_git(repo_root, "rev-parse", "--abbrev-ref", "HEAD"),
            "head": head,
            "clean": bool(snapshot.get("clean", False)),
            "snapshot_digest": snapshot_digest(snapshot),
        },
        "active_contract": active_contract,
        "locked_contract_validation": {
            "ok": not bool(contract_error),
            "error": contract_error,
            "validated_contract_id": None if locked_contract_state is None else locked_contract_state.get("contract_id"),
            "validated_plan_path": None if locked_contract_state is None else locked_contract_state.get("plan_path"),
        },
        "changed_paths": {
            "all": changed_paths,
            "summary": summarize_changed_paths(changed_paths),
            "in_contract_scope": in_scope,
            "out_of_contract_scope": out_of_scope,
        },
        "latest_handoff_context": _latest_handoff_entries(repo_root),
        "receipts_for_current_head": {
            "validation_receipt_path": validation_path.relative_to(repo_root).as_posix(),
            "validation_receipt_exists": validation_path.exists(),
            "contract_proof_receipt_path": contract_proof_path.relative_to(repo_root).as_posix(),
            "contract_proof_receipt_exists": contract_proof_path.exists(),
        },
        "snapshot": snapshot,
    }


def write_recovery_report(report: dict[str, Any], *, snapshot: dict[str, Any], repo_root: Path) -> Path:
    path = recovery_report_path_for_snapshot(snapshot, repo_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return path


def _snapshot_without_paths(snapshot: dict[str, Any], *, repo_root: Path, ignored_paths: list[Path]) -> dict[str, Any]:
    normalized = json.loads(json.dumps(snapshot))
    ignored = {
        path.relative_to(repo_root).as_posix()
        for path in ignored_paths
        if path.exists() and path.is_relative_to(repo_root)
    }
    for section in ("unstaged", "staged", "untracked"):
        entries = normalized.get(section, {}) or {}
        normalized[section] = {path: value for path, value in entries.items() if path not in ignored}
    normalized["clean"] = snapshot_is_clean(normalized)
    return normalized


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description=(
            "Diagnose checkpoint-guard crash recovery state, write a durable recovery report, "
            "and optionally adopt the current dirty snapshot so the next session can resume it."
        ),
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--summary", required=True, help="Short operator note describing the crash, rollback, or recovery context")
    parser.add_argument("--adopt-current-state", action="store_true", help="Write an explicit one-shot recovery adoption artifact for the current dirty snapshot")
    parser.add_argument("--repo-root", default=str(REPO_ROOT), help="Repository root to diagnose")
    args = parser.parse_args(argv)

    repo_root = Path(args.repo_root)
    if not repo_root.is_absolute():
        repo_root = (REPO_ROOT / repo_root).resolve()

    snapshot = capture_repo_snapshot(repo_root)
    report = build_recovery_report(repo_root=repo_root, summary=args.summary, snapshot=snapshot)
    report_path = write_recovery_report(report, snapshot=snapshot, repo_root=repo_root)

    print(f"viewer_host_recover_crash_state: report={report_path.relative_to(repo_root).as_posix()}")
    print("viewer_host_recover_crash_state: reason=" + str(report["reason"]))
    print(
        "viewer_host_recover_crash_state: changed_paths="
        + str(report["changed_paths"]["summary"])
    )

    if not args.adopt_current_state:
        print(
            "viewer_host_recover_crash_state: next="
            + recovery_helper_command()
        )
        return 0

    final_snapshot = capture_repo_snapshot(repo_root)
    comparable_final_snapshot = _snapshot_without_paths(
        final_snapshot,
        repo_root=repo_root,
        ignored_paths=[report_path],
    )
    if snapshot_digest(comparable_final_snapshot) != snapshot_digest(snapshot):
        sys.stderr.write(
            "viewer_host_recover_crash_state: current snapshot changed during diagnosis; rerun the recovery helper before adopting.\n"
        )
        return 2

    adoption_path = write_active_recovery_adoption(
        comparable_final_snapshot,
        report_path=report_path,
        summary=args.summary,
        reason=str(report["reason"]),
        repo_root=repo_root,
    )
    print(
        "viewer_host_recover_crash_state: adoption="
        + adoption_path.relative_to(repo_root).as_posix()
    )
    print(
        "viewer_host_recover_crash_state: retry the prompt in a fresh session to resume the stranded slice."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
