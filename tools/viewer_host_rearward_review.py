from __future__ import annotations

import argparse
from datetime import datetime, timezone
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

try:
    from tools.viewer_host_contract_state import contract_proof_receipt_path
    from tools.viewer_host_validate_hostile_audit import validate_hostile_audit_plan
except ModuleNotFoundError:
    from viewer_host_contract_state import contract_proof_receipt_path
    from viewer_host_validate_hostile_audit import validate_hostile_audit_plan


REPO_ROOT = Path(__file__).resolve().parents[1]
REARWARD_REVIEW_VERSION = 1
REARWARD_REVIEW_STATES = {"ok", "needs_repair", "blocked_unproven"}
STALE_PLAN_PHRASES = (
    "ready for checkpoint",
    "ready for receipts",
    "remaining mechanical step",
    "checkpoint and receipt",
    "resume",
)


def _run_git(repo_root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *args],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )


def _git_output(repo_root: Path, *args: str) -> str:
    proc = _run_git(repo_root, *args)
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout or "git command failed").strip()
        raise RuntimeError(detail)
    return (proc.stdout or "").strip()


def _sanitize_head(head: str) -> str:
    sanitized = "".join(ch for ch in head if ch.isalnum())
    if not sanitized:
        raise RuntimeError("rearward review requires a non-empty HEAD id")
    return sanitized


def validation_receipt_path(head: str, repo_root: Path = REPO_ROOT) -> Path:
    return repo_root / "artifacts" / "hooks" / "viewer_host_validation_receipts" / f"{_sanitize_head(head)}.json"


def rearward_review_dir(repo_root: Path = REPO_ROOT) -> Path:
    return repo_root / "artifacts" / "hooks" / "viewer_host_rearward_review"


def rearward_review_artifact_path(head: str, repo_root: Path = REPO_ROOT) -> Path:
    return rearward_review_dir(repo_root) / f"{_sanitize_head(head)}.json"


def load_rearward_review_artifact(head: str, repo_root: Path = REPO_ROOT) -> dict[str, Any] | None:
    path = rearward_review_artifact_path(head, repo_root)
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def _current_branch(repo_root: Path) -> str:
    return _git_output(repo_root, "rev-parse", "--abbrev-ref", "HEAD")


def _current_head(repo_root: Path) -> str:
    return _git_output(repo_root, "rev-parse", "HEAD")


def _worktree_clean(repo_root: Path) -> bool:
    return not bool(_git_output(repo_root, "status", "--porcelain"))


def _changed_files_last_commit(repo_root: Path, head: str) -> list[str]:
    lines = _git_output(repo_root, "diff-tree", "--root", "--no-commit-id", "--name-only", "-r", head).splitlines()
    return sorted(Path(line.strip()).as_posix() for line in lines if line.strip())


def _ahead_behind(repo_root: Path) -> dict[str, Any]:
    upstream_proc = _run_git(repo_root, "rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}")
    if upstream_proc.returncode != 0:
        return {"state": "no_upstream", "upstream": "", "ahead": None, "behind": None}
    upstream = (upstream_proc.stdout or "").strip()
    counts = _git_output(repo_root, "rev-list", "--left-right", "--count", f"HEAD...{upstream}").split()
    ahead = int(counts[0]) if counts else 0
    behind = int(counts[1]) if len(counts) > 1 else 0
    return {"state": "tracked", "upstream": upstream, "ahead": ahead, "behind": behind}


def _latest_handoff(repo_root: Path) -> dict[str, Any] | None:
    handoff_path = repo_root / "HANDOFF_LOG.md"
    if not handoff_path.exists():
        return None
    lines = handoff_path.read_text(encoding="utf-8").splitlines()
    for index in range(len(lines) - 1, -1, -1):
        text = lines[index].strip()
        if text:
            return {"path": "HANDOFF_LOG.md", "line": index + 1, "text": text}
    return None


def _receipt_payload(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError:
        return {"_invalid_json": True}


def _receipt_status(path: Path, head: str) -> dict[str, Any]:
    payload = _receipt_payload(path)
    present = payload is not None
    head_matches = present and str(payload.get("head", "")).strip() == head
    return {
        "present": present,
        "path": path.as_posix(),
        "head_matches": head_matches,
        "invalid_json": bool(isinstance(payload, dict) and payload.get("_invalid_json")),
    }


def _scan_stale_plan_phrases(repo_root: Path, plan_paths: list[str]) -> list[dict[str, Any]]:
    hits: list[dict[str, Any]] = []
    for plan_text in plan_paths:
        plan_path = repo_root / plan_text
        if not plan_path.exists():
            continue
        for line_number, line in enumerate(plan_path.read_text(encoding="utf-8").splitlines(), start=1):
            lower = line.lower()
            for phrase in STALE_PLAN_PHRASES:
                if phrase in lower:
                    hits.append(
                        {
                            "path": plan_text,
                            "line": line_number,
                            "phrase": phrase,
                            "text": line.strip(),
                        }
                    )
    return hits


def _validate_touched_plan_hostile_audits(repo_root: Path, plan_paths: list[str]) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for plan_text in plan_paths:
        plan_path = repo_root / plan_text
        if not plan_path.exists():
            results.append({"path": plan_text, "ok": False, "blocked_reason": "touched plan is missing"})
            continue
        result = validate_hostile_audit_plan(plan_path)
        result["path"] = plan_text
        results.append(result)
    return results


def _append_receipt_findings(findings: list[dict[str, Any]], *, receipt: dict[str, Any], kind: str, label: str) -> None:
    if not receipt["present"]:
        findings.append({"kind": kind, "severity": "blocked_unproven", "message": f"missing {label} receipt"})
    elif receipt["invalid_json"]:
        findings.append({"kind": f"invalid_{kind}", "severity": "blocked_unproven", "message": f"invalid {label} receipt json"})
    elif not receipt["head_matches"]:
        findings.append(
            {"kind": f"{kind}_head_mismatch", "severity": "blocked_unproven", "message": f"{label} receipt head mismatch"}
        )


def _status_from_findings(findings: list[dict[str, Any]]) -> str:
    if any(finding.get("severity") == "blocked_unproven" for finding in findings):
        return "blocked_unproven"
    if findings:
        return "needs_repair"
    return "ok"


def build_rearward_review_payload(*, repo_root: Path = REPO_ROOT, head: str | None = None) -> dict[str, Any]:
    repo_root = repo_root.resolve()
    current_head = head or _current_head(repo_root)
    branch = _current_branch(repo_root)
    clean = _worktree_clean(repo_root)
    changed_files = _changed_files_last_commit(repo_root, current_head)
    touched_plans = [path for path in changed_files if path.endswith("_PHASED_PLAN.md")]
    validation = _receipt_status(validation_receipt_path(current_head, repo_root), current_head)
    contract_proof = _receipt_status(contract_proof_receipt_path(current_head, repo_root), current_head)
    hostile_results = _validate_touched_plan_hostile_audits(repo_root, touched_plans)
    stale_hits = _scan_stale_plan_phrases(repo_root, touched_plans)

    findings: list[dict[str, Any]] = []
    if not clean:
        findings.append({"kind": "dirty_worktree", "severity": "blocked_unproven", "message": "worktree is not clean"})
    _append_receipt_findings(
        findings,
        receipt=validation,
        kind="missing_validation_receipt",
        label="validation",
    )
    _append_receipt_findings(
        findings,
        receipt=contract_proof,
        kind="missing_contract_proof_receipt",
        label="contract proof",
    )
    for hostile in hostile_results:
        if not hostile.get("ok", False):
            findings.append(
                {
                    "kind": "hostile_audit_not_ok",
                    "severity": "needs_repair",
                    "path": hostile.get("path", ""),
                    "message": str(hostile.get("blocked_reason", "hostile audit is not ok")),
                }
            )
    for hit in stale_hits:
        findings.append(
            {
                "kind": "stale_plan_phrase",
                "severity": "needs_repair",
                "path": hit["path"],
                "line": hit["line"],
                "message": f"stale plan phrase: {hit['phrase']}",
            }
        )

    return {
        "version": REARWARD_REVIEW_VERSION,
        "reviewed_at_utc": datetime.now(timezone.utc).isoformat(),
        "status": _status_from_findings(findings),
        "repo_root": repo_root.as_posix(),
        "branch": branch,
        "head": current_head,
        "head_short": current_head[:12],
        "clean": clean,
        "ahead_behind": _ahead_behind(repo_root),
        "latest_handoff": _latest_handoff(repo_root),
        "changed_files_last_commit": changed_files,
        "touched_phased_plans": touched_plans,
        "receipts": {
            "validation": validation,
            "contract_proof": contract_proof,
        },
        "hostile_audit": hostile_results,
        "stale_plan_phrase_hits": stale_hits,
        "findings": findings,
    }


def write_rearward_review_artifact(payload: dict[str, Any], *, repo_root: Path = REPO_ROOT) -> Path:
    status = str(payload.get("status", "")).strip()
    if status not in REARWARD_REVIEW_STATES:
        raise RuntimeError(f"invalid rearward review status: {status}")
    head = str(payload.get("head", "")).strip()
    path = rearward_review_artifact_path(head, repo_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return path


def _write_optional_copy(payload: dict[str, Any], out_json: str | None, repo_root: Path) -> None:
    if not out_json:
        return
    out_path = Path(out_json)
    if not out_path.is_absolute():
        out_path = repo_root / out_path
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Write and report a rearward hostile-review artifact for the current HEAD")
    parser.add_argument("--repo-root", default=str(REPO_ROOT), help="Repo root to inspect")
    parser.add_argument("--head", help="Optional committed head to inspect; defaults to current HEAD")
    parser.add_argument("--out-json", help="Optional extra copy of the review JSON")
    args = parser.parse_args(argv)

    repo_root = Path(args.repo_root)
    if not repo_root.is_absolute():
        repo_root = (Path.cwd() / repo_root).resolve()
    try:
        payload = build_rearward_review_payload(repo_root=repo_root, head=args.head)
        artifact_path = write_rearward_review_artifact(payload, repo_root=repo_root)
        _write_optional_copy(payload, args.out_json, repo_root)
        print(json.dumps({"status": payload["status"], "artifact": artifact_path.as_posix()}, indent=2))
        if payload["status"] == "ok":
            return 0
        if payload["status"] == "needs_repair":
            return 1
        return 2
    except Exception as exc:
        sys.stderr.write(f"viewer_host_rearward_review: {exc}\n")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
