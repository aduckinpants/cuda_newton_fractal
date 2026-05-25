from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

import tools.viewer_host_rearward_review as rearward_review
from tools.viewer_host_contract_state import contract_proof_receipt_path


VALID_PLAN_TEXT = """# Test Slice

## Current Phase

Closed - test fixture.

## Phase Checklist

- [x] Phase 1 - complete

## Explicit User Asks

- [done] Prove the fixture.

## Hostile Audit

- Status: done

## Audit Passes

- [done] Pass 1 - found a real workflow defect and repaired it
- [done] Pass 2 - re-read the repaired state and confirmed the repair
- [done] Pass 3 - re-read the repaired state again and confirmed no additional real defect was found

## Audit Findings

- [done] Real defect found and repaired: the guard previously allowed unreviewed mutation.
- [done] No additional real defect found in the repaired state after the focused re-audit.
"""


def _run_git(repo_root: Path, *args: str) -> str:
    proc = subprocess.run(
        ["git", *args],
        cwd=repo_root,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    return proc.stdout.strip()


def _init_repo(repo_root: Path, *, plan_text: str = VALID_PLAN_TEXT) -> str:
    repo_root.mkdir(parents=True)
    _run_git(repo_root, "init")
    _run_git(repo_root, "config", "user.email", "agent@example.com")
    _run_git(repo_root, "config", "user.name", "Agent")
    (repo_root / ".gitignore").write_text("artifacts/\n", encoding="utf-8")
    plan_path = repo_root / "docs" / "notes" / "test_slice_PHASED_PLAN.md"
    plan_path.parent.mkdir(parents=True)
    plan_path.write_text(plan_text, encoding="utf-8")
    (repo_root / "HANDOFF_LOG.md").write_text("- commit: abc123 score=95 initial handoff\n", encoding="utf-8")
    _run_git(repo_root, "add", ".gitignore", "docs/notes/test_slice_PHASED_PLAN.md", "HANDOFF_LOG.md")
    _run_git(repo_root, "commit", "-m", "test slice closeout")
    return _run_git(repo_root, "rev-parse", "HEAD")


def _write_receipts(repo_root: Path, head: str) -> None:
    validation_path = rearward_review.validation_receipt_path(head, repo_root)
    validation_path.parent.mkdir(parents=True, exist_ok=True)
    validation_path.write_text(json.dumps({"head": head, "commands": ["pytest"]}), encoding="utf-8")
    contract_path = contract_proof_receipt_path(head, repo_root)
    contract_path.parent.mkdir(parents=True, exist_ok=True)
    contract_path.write_text(json.dumps({"head": head, "contract_id": "test"}), encoding="utf-8")


def test_rearward_review_writes_ok_artifact_for_clean_reviewed_head(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    head = _init_repo(repo_root)
    _write_receipts(repo_root, head)

    payload = rearward_review.build_rearward_review_payload(repo_root=repo_root)
    artifact_path = rearward_review.write_rearward_review_artifact(payload, repo_root=repo_root)

    assert payload["status"] == "ok"
    assert payload["head"] == head
    assert payload["clean"] is True
    assert payload["receipts"]["validation"]["present"] is True
    assert payload["receipts"]["contract_proof"]["present"] is True
    assert payload["changed_files_last_commit"] == [
        ".gitignore",
        "HANDOFF_LOG.md",
        "docs/notes/test_slice_PHASED_PLAN.md",
    ]
    assert payload["touched_phased_plans"] == ["docs/notes/test_slice_PHASED_PLAN.md"]
    assert artifact_path == rearward_review.rearward_review_artifact_path(head, repo_root)
    assert json.loads(artifact_path.read_text(encoding="utf-8"))["status"] == "ok"


def test_rearward_review_missing_validation_receipt_is_blocked_unproven(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    head = _init_repo(repo_root)
    contract_path = contract_proof_receipt_path(head, repo_root)
    contract_path.parent.mkdir(parents=True, exist_ok=True)
    contract_path.write_text(json.dumps({"head": head}), encoding="utf-8")

    payload = rearward_review.build_rearward_review_payload(repo_root=repo_root)

    assert payload["status"] == "blocked_unproven"
    assert any(finding["kind"] == "missing_validation_receipt" for finding in payload["findings"])


def test_rearward_review_missing_contract_proof_receipt_is_blocked_unproven(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    head = _init_repo(repo_root)
    validation_path = rearward_review.validation_receipt_path(head, repo_root)
    validation_path.parent.mkdir(parents=True, exist_ok=True)
    validation_path.write_text(json.dumps({"head": head}), encoding="utf-8")

    payload = rearward_review.build_rearward_review_payload(repo_root=repo_root)

    assert payload["status"] == "blocked_unproven"
    assert any(finding["kind"] == "missing_contract_proof_receipt" for finding in payload["findings"])


def test_rearward_review_stale_touched_plan_is_needs_repair(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    head = _init_repo(repo_root, plan_text=VALID_PLAN_TEXT + "\nThe remaining mechanical step is ready for checkpoint.\n")
    _write_receipts(repo_root, head)

    payload = rearward_review.build_rearward_review_payload(repo_root=repo_root)

    assert payload["status"] == "needs_repair"
    assert any(finding["kind"] == "stale_plan_phrase" for finding in payload["findings"])
    assert payload["stale_plan_phrase_hits"][0]["phrase"] == "ready for checkpoint"


def test_rearward_review_incomplete_hostile_audit_is_needs_repair(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    head = _init_repo(repo_root, plan_text=VALID_PLAN_TEXT.replace("- Status: done", "- Status: pending"))
    _write_receipts(repo_root, head)

    payload = rearward_review.build_rearward_review_payload(repo_root=repo_root)

    assert payload["status"] == "needs_repair"
    assert any(finding["kind"] == "hostile_audit_not_ok" for finding in payload["findings"])
