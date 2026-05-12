from __future__ import annotations

import argparse
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

try:
    from tools.viewer_host_checkpoint_guard import validate_validation_receipt_evidence_freshness, validation_receipt_path
    from tools.viewer_host_contract_state import contract_proof_receipt_path
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import validate_validation_receipt_evidence_freshness, validation_receipt_path
    from viewer_host_contract_state import contract_proof_receipt_path


REPO_ROOT = Path(__file__).resolve().parents[1]


def _git(repo_root: Path, *args: str) -> str:
    proc = subprocess.run(["git", *args], cwd=str(repo_root), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)
    if proc.returncode != 0:
        raise RuntimeError((proc.stderr or proc.stdout or "git command failed").strip())
    return (proc.stdout or "").strip()


def _status_short(repo_root: Path) -> list[str]:
    output = _git(repo_root, "status", "--short")
    return [line.rstrip() for line in output.splitlines() if line.strip()]


def _load_json(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def build_truth_report(repo_root: Path = REPO_ROOT) -> dict[str, Any]:
    repo_root = repo_root.resolve()
    head = _git(repo_root, "rev-parse", "HEAD")
    branch = _git(repo_root, "rev-parse", "--abbrev-ref", "HEAD")
    status_lines = _status_short(repo_root)
    validation_path = validation_receipt_path(head, repo_root)
    contract_path = contract_proof_receipt_path(head, repo_root)
    validation_receipt = _load_json(validation_path)
    contract_receipt = _load_json(contract_path)
    evidence_ok, evidence_reason = validate_validation_receipt_evidence_freshness(validation_receipt, repo_root)
    checks = {
        "live_git_clean": not status_lines,
        "validation_evidence_fresh": evidence_ok,
        "validation_receipt_head_matches": validation_receipt is None or str(validation_receipt.get("head", "")).strip() == head,
        "contract_proof_receipt_head_matches": contract_receipt is None or str(contract_receipt.get("head", "")).strip() == head,
    }
    ok = all(bool(value) for value in checks.values())
    return {
        "ok": ok,
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "repo_root": repo_root.as_posix(),
        "git": {
            "branch": branch,
            "head": head,
            "status_short": status_lines,
        },
        "receipts": {
            "validation_receipt": validation_path.relative_to(repo_root).as_posix(),
            "validation_receipt_exists": validation_receipt is not None,
            "contract_proof_receipt": contract_path.relative_to(repo_root).as_posix(),
            "contract_proof_receipt_exists": contract_receipt is not None,
        },
        "checks": checks,
        "failure_reasons": [] if ok else [
            reason for reason in [
                "live git status is dirty" if status_lines else "",
                evidence_reason,
                "validation receipt head does not match current HEAD" if not checks["validation_receipt_head_matches"] else "",
                "contract proof receipt head does not match current HEAD" if not checks["contract_proof_receipt_head_matches"] else "",
            ] if reason
        ],
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Write a machine-readable current truth report for status claims")
    parser.add_argument("--out-json", required=True, help="Output JSON artifact path")
    parser.add_argument("--repo-root", default=str(REPO_ROOT), help="Repository root")
    args = parser.parse_args(argv)
    repo_root = Path(args.repo_root).resolve()
    payload = build_truth_report(repo_root)
    out_path = Path(args.out_json)
    if not out_path.is_absolute():
        out_path = repo_root / out_path
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"viewer_host_truth_report: ok={payload['ok']} out={out_path}")
    return 0 if payload["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
