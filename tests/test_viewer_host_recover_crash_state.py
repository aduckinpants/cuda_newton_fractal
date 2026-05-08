from __future__ import annotations

import contextlib
import io
import json
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

import tools.viewer_host_checkpoint_guard as checkpoint_guard
import tools.viewer_host_contract_state as contract_state
import tools.viewer_host_recover_crash_state as recover_crash_state


def _init_git_repo(repo_root: Path) -> None:
    repo_root.mkdir(parents=True, exist_ok=True)
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)


def _write_active_contract(repo_root: Path) -> None:
    contract_path = repo_root / "docs" / "contracts" / "slice.contract.json"
    plan_path = repo_root / "docs" / "notes" / "slice_PHASED_PLAN.md"
    contract_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text("# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n", encoding="utf-8")
    contract_payload = {
        "version": 1,
        "contract_id": "slice",
        "feature_id": "slice",
        "workflow_type": "workflow_only",
        "plan_path": "docs/notes/slice_PHASED_PLAN.md",
        "allowed_mutation_scope": [
            "docs/contracts/slice.contract.json",
            "docs/notes/slice_PHASED_PLAN.md",
            "tools",
        ],
        "required_operator_inputs": ["respect workflow"],
        "forbidden_operator_prompts": ["ignore workflow"],
        "required_defaults": {"workflow": "required"},
        "forbidden_defaults": {"workflow": "forbidden"},
        "required_validation_commands": ["pytest"],
        "required_acceptance_assertions": [
            {
                "assertion_id": "contract_schema_valid",
                "description": "contract schema valid",
                "evidence_kind": "validator_json",
                "artifact_path": "artifacts/validation/contract.json",
                "json_path": "ok",
                "equals": True,
            }
        ],
    }
    contract_path.write_text(json.dumps(contract_payload, indent=2), encoding="utf-8")
    state_path = contract_state.contract_state_path_for_session(contract_state.GLOBAL_CONTRACT_SESSION_ID, repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "slice",
                "workflow_type": "workflow_only",
                "contract_path": "docs/contracts/slice.contract.json",
                "plan_path": "docs/notes/slice_PHASED_PLAN.md",
                "contract_hash": contract_state.hash_file(contract_path),
                "allowed_mutation_scope": contract_payload["allowed_mutation_scope"],
                "required_validation_commands": ["pytest"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    subprocess.run(["git", "add", "."], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "lock slice"], cwd=repo_root, check=True, capture_output=True, text=True)


def test_build_recovery_report_splits_in_scope_and_out_of_scope_paths(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    _write_active_contract(repo_root)
    (repo_root / "README.md").write_text("changed\n", encoding="utf-8")
    (repo_root / "tools").mkdir(exist_ok=True)
    (repo_root / "tools" / "new_helper.py").write_text("print('x')\n", encoding="utf-8")
    snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)

    report = recover_crash_state.build_recovery_report(
        repo_root=repo_root,
        summary="host crashed during slice",
        snapshot=snapshot,
    )

    assert "tools/new_helper.py" in report["changed_paths"]["in_contract_scope"]
    assert "README.md" in report["changed_paths"]["out_of_contract_scope"]
    assert report["reason"].startswith("Checkpoint guard baseline is missing")


def test_main_writes_report_and_adoption_artifact(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    (repo_root / "README.md").write_text("changed\n", encoding="utf-8")

    buf = io.StringIO()
    with contextlib.redirect_stdout(buf):
        rc = recover_crash_state.main([
            "--repo-root",
            str(repo_root),
            "--summary",
            "host crashed and rollback reset the chat",
            "--adopt-current-state",
        ])

    output = buf.getvalue()
    assert rc == 0
    assert "report=" in output
    assert "adoption=" in output

    recovery_root = checkpoint_guard.recovery_dir(repo_root)
    reports = sorted(recovery_root.glob("recovery_*.json"))
    assert reports
    adoption_path = checkpoint_guard.recovery_adoption_path(repo_root)
    assert adoption_path.exists()

    payload = json.loads(adoption_path.read_text(encoding="utf-8"))
    assert payload["summary"] == "host crashed and rollback reset the chat"
    assert payload["report_path"] == reports[-1].relative_to(repo_root).as_posix()
