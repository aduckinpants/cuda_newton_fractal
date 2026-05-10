from __future__ import annotations

import contextlib
import io
import json
import subprocess
import sys
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))


TEST_PLAN_TEXT = (
    "# Plan\n\n"
    "## Current Phase\n\n"
    "Phase 1 in progress\n\n"
    "## Phase Checklist\n\n"
    "- [ ] Phase 1 - X\n"
)

import tools.viewer_host_checkpoint_guard as checkpoint_guard
import tools.viewer_host_checkpoint_dirty_prompt_guard as checkpoint_dirty_prompt_guard
import tools.viewer_host_contract_state as contract_state
import tools.viewer_host_hook_require_checkpoint_before_complete as completion_hook
import tools.viewer_host_hook_require_checkpoint_carryover as carryover_hook
import tools.viewer_host_hook_stop_if_dirty_worktree as stop_hook
from tools.viewer_host_validate_hostile_audit import validate_hostile_audit_plan

from tools.viewer_host_checkpoint_guard import (
    build_posttool_response,
    build_pretool_response,
    build_stop_response,
    compare_snapshots,
    discover_repo_root,
    evaluate_checkpoint_guard,
    evaluate_validation_receipt_guard,
    load_validation_receipt,
    summarize_changed_paths,
    validation_receipt_path,
    write_validation_receipt,
)
from tools.viewer_host_contract_state import contract_proof_receipt_path
from tools.viewer_host_checkpoint_dirty_prompt_guard import (
    build_dirty_prompt_message,
    build_userprompt_response,
    build_validation_receipt_prompt_message,
    load_carryover_state,
    write_carryover_state,
)


def _snapshot(*, unstaged: dict[str, str] | None = None, staged: dict[str, str] | None = None, untracked: dict[str, str] | None = None) -> dict[str, object]:
    return {
        "unstaged": unstaged or {},
        "staged": staged or {},
        "untracked": untracked or {},
        "clean": not any((unstaged, staged, untracked)),
    }


def _invoke_hook_main(module: Any, payload: dict[str, object]) -> tuple[int, dict[str, object]]:
    old_stdin = sys.stdin
    sys.stdin = io.StringIO(json.dumps(payload))
    try:
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = module.main()
        output = json.loads(buf.getvalue())
    finally:
        sys.stdin = old_stdin
    return rc, output


def _write_active_contract_with_plan(
    repo_root: Path,
    *,
    session_id: str = "session-1",
    contract_id: str = "slice",
    workflow_type: str = "workflow_only",
    plan_text: str,
    required_defaults: dict[str, str] | None = None,
) -> tuple[Path, Path]:
    contract_path = repo_root / "docs" / "contracts" / f"{contract_id}.contract.json"
    plan_path = repo_root / "docs" / "notes" / f"{contract_id}_PHASED_PLAN.md"
    contract_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text(plan_text, encoding="utf-8")
    contract_payload = {
        "version": 1,
        "contract_id": contract_id,
        "feature_id": contract_id,
        "workflow_type": workflow_type,
        "plan_path": str(plan_path.relative_to(repo_root).as_posix()),
        "allowed_mutation_scope": [
            str(contract_path.relative_to(repo_root).as_posix()),
            str(plan_path.relative_to(repo_root).as_posix()),
            "tools",
        ],
        "required_operator_inputs": ["respect explicit user asks"],
        "forbidden_operator_prompts": ["ignore explicit user asks"],
        "required_defaults": {
            "explicit_user_asks_enforced": "required",
            **(required_defaults or {}),
        },
        "forbidden_defaults": {"closure_while_open_asks": "forbidden"},
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
    state_path = contract_state.contract_state_path_for_session(session_id, repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": contract_id,
                "feature_id": contract_id,
                "workflow_type": workflow_type,
                "contract_path": str(contract_path.relative_to(repo_root).as_posix()),
                "plan_path": str(plan_path.relative_to(repo_root).as_posix()),
                "contract_hash": contract_state.hash_file(contract_path),
                "allowed_mutation_scope": contract_payload["allowed_mutation_scope"],
                "required_validation_commands": contract_payload["required_validation_commands"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    return contract_path, plan_path


SALT_NDEPEND_READY_PLAN_TEXT = (
    "# Plan\n\n"
    "## Current Phase\n\n"
    "Phase 1 in progress\n\n"
    "## Phase Checklist\n\n"
    "- [ ] Phase 1 - X\n\n"
    "## Explicit User Asks\n\n"
    "- [done] Start implementation\n\n"
    "## Hostile Audit\n\n"
    "- Status: done\n\n"
    "## Audit Passes\n\n"
    "- [done] Pass 1 - found a regression in the guard behavior and repaired it\n"
    "- [done] Pass 2 - reran the focused validations after the repair and re-read the repaired state\n"
    "- [done] Pass 3 - re-read the repaired state again and confirmed no additional real defect was found\n\n"
    "## Audit Findings\n\n"
    "- [done] Real defect found and repaired: the first hostile-audit implementation still allowed false closure after a finding.\n"
    "- [done] No additional real defect found in the repaired state after the focused re-audit.\n"
)


def _init_git_repo(repo_root: Path) -> None:
    repo_root.mkdir(parents=True, exist_ok=True)
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)


def test_compare_snapshots_detects_same_file_content_changes() -> None:
    baseline = _snapshot(unstaged={"AGENTS.md": "hash-a"})
    current = _snapshot(unstaged={"AGENTS.md": "hash-b"})

    assert compare_snapshots(baseline, current) == ["AGENTS.md"]


def test_evaluate_checkpoint_guard_blocks_when_state_differs_from_baseline() -> None:
    baseline = _snapshot()
    current = _snapshot(unstaged={"ui_app/src/main.cpp": "hash-main"})

    status = evaluate_checkpoint_guard(baseline, current)

    assert status.should_block is True
    assert "session baseline" in status.reason
    assert status.changed_paths == ["ui_app/src/main.cpp"]


def test_evaluate_checkpoint_guard_blocks_dirty_state_when_baseline_missing() -> None:
    current = _snapshot(untracked={"docs/notes/foo.md": "hash-doc"})

    status = evaluate_checkpoint_guard(None, current)

    assert status.should_block is True
    assert "baseline is missing" in status.reason
    assert status.changed_paths == ["docs/notes/foo.md"]


def test_build_pretool_response_denies_task_complete_when_state_changed() -> None:
    baseline = _snapshot()
    current = _snapshot(staged={"HANDOFF_LOG.md": "blob-1"})

    response = build_pretool_response("task_complete", baseline, current, "session-1", {"recipient_name": "functions.task_complete"})

    assert response is not None
    payload = response["hookSpecificOutput"]
    assert payload["permissionDecision"] == "deny"
    assert "checkpoint commit" in payload["permissionDecisionReason"] or "baseline" in payload["permissionDecisionReason"]
    assert "HANDOFF_LOG.md" in payload["additionalContext"]


def test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner() -> None:
    baseline = _snapshot()
    current = _snapshot(unstaged={"AGENTS.md": "hash-a"})

    response = build_pretool_response("run_in_terminal", baseline, current, "session-1", {"recipient_name": "functions.read_only"})

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "allow"
    assert "STRICT REPO RULE" in hook["permissionDecisionReason"]


def test_build_posttool_response_emits_checkpoint_debt_reminder() -> None:
    baseline = _snapshot()
    current = _snapshot(
        unstaged={"docs/notes/plan.md": "hash-plan"},
        staged={"HANDOFF_LOG.md": "blob-1"},
    )

    response = build_posttool_response("apply_patch", baseline, current, "session-1")

    assert response is not None
    assert "systemMessage" in response
    assert "STRICT REPO RULE" in response["systemMessage"]
    assert "apply_patch" in response["systemMessage"]
    assert "checkpoint commit" in response["systemMessage"] or "baseline" in response["systemMessage"]
    assert "docs/notes/plan.md" in response["systemMessage"]
    assert "HANDOFF_LOG.md" in response["systemMessage"]


def test_build_posttool_response_ignores_clean_state() -> None:
    baseline = _snapshot()
    current = _snapshot()

    response = build_posttool_response("read_tool", baseline, current, "session-1")
    assert response is not None
    assert "STRICT REPO RULE" in response["systemMessage"]


def test_build_pretool_response_blocks_clean_head_without_validation_receipt(tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    response = build_pretool_response("task_complete", baseline, current, "session-1", {"recipient_name": "functions.task_complete"}, tmp_path)

    assert response is not None
    payload = response["hookSpecificOutput"]
    assert payload["permissionDecision"] == "deny"
    assert "validation receipt" in payload["permissionDecisionReason"]
    assert "def456.json" in payload["additionalContext"]


def test_completion_hook_blocks_task_complete_via_recipient_name(monkeypatch) -> None:
    baseline = _snapshot()
    current = _snapshot(staged={"HANDOFF_LOG.md": "blob-1"})

    monkeypatch.setattr(completion_hook.checkpoint_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(completion_hook.checkpoint_guard, "_bootstrap_missing_baseline_if_clean", lambda session_id, current_snapshot, repo_root=checkpoint_guard.REPO_ROOT: baseline)
    monkeypatch.setattr(completion_hook.checkpoint_guard, "discover_repo_root", lambda start_path: REPO_ROOT)

    payload = {
        "hookEventName": "PreToolUse",
        "sessionId": "session-1",
        "cwd": str(REPO_ROOT),
        "recipient_name": "functions.task_complete",
        "parameters": {"summary": "done"},
    }

    rc, output = _invoke_hook_main(completion_hook, payload)

    assert rc == 0
    hook = output["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "session baseline" in hook["permissionDecisionReason"]
    assert "HANDOFF_LOG.md" in hook["additionalContext"]


def test_completion_hook_blocks_missing_validation_receipt_via_recipient_name(monkeypatch, tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    monkeypatch.setattr(completion_hook.checkpoint_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(completion_hook.checkpoint_guard, "_bootstrap_missing_baseline_if_clean", lambda session_id, current_snapshot, repo_root=checkpoint_guard.REPO_ROOT: baseline)
    monkeypatch.setattr(completion_hook.checkpoint_guard, "discover_repo_root", lambda start_path: tmp_path)

    payload = {
        "hookEventName": "PreToolUse",
        "sessionId": "session-1",
        "cwd": str(tmp_path),
        "recipient_name": "functions.task_complete",
        "parameters": {"summary": "done"},
    }

    rc, output = _invoke_hook_main(completion_hook, payload)

    assert rc == 0
    hook = output["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "validation receipt" in hook["permissionDecisionReason"]
    assert "def456.json" in hook["additionalContext"]


def test_build_pretool_response_blocks_validation_receipted_head_without_contract_proof(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        session_id="session-1",
        plan_text=TEST_PLAN_TEXT,
    )
    (tmp_path / "tools").mkdir()
    baseline = _snapshot()
    baseline["head"] = "def456"
    current = _snapshot()
    current["head"] = "def456"
    receipt_path = validation_receipt_path("def456", tmp_path)
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    receipt_path.write_text(json.dumps({"head": "def456"}, indent=2) + "\n", encoding="utf-8")

    response = build_pretool_response(
        "task_complete",
        baseline,
        current,
        "session-1",
        {"recipient_name": "functions.task_complete"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "contract proof receipt" in hook["permissionDecisionReason"]
    assert "Current clean validation-receipted HEAD lacks" in hook["permissionDecisionReason"]
    assert "session baseline" not in hook["permissionDecisionReason"]


def test_completion_hook_allows_non_task_complete(monkeypatch) -> None:
    monkeypatch.setattr(completion_hook.checkpoint_guard, "discover_repo_root", lambda start_path: REPO_ROOT)

    payload = {
        "hookEventName": "PreToolUse",
        "sessionId": "session-1",
        "cwd": str(REPO_ROOT),
        "recipient_name": "functions.read_file",
    }

    rc, output = _invoke_hook_main(completion_hook, payload)

    assert rc == 0
    assert output["hookSpecificOutput"]["permissionDecision"] == "allow"

def test_build_pretool_response_denies_task_complete_status_vocabulary_without_proof(tmp_path: Path) -> None:
    response = build_pretool_response(
        "task_complete",
        _snapshot(),
        _snapshot(),
        "session-1",
        {
            "recipient_name": "functions.task_complete",
            "parameters": {"summary": "done"},
        },
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "Restricted status vocabulary" in hook["permissionDecisionReason"]
    assert "fresh command" in hook["additionalContext"]
    assert "exit code" in hook["additionalContext"]


def test_build_pretool_response_allows_task_complete_status_vocabulary_with_same_summary_proof(tmp_path: Path) -> None:
    response = build_pretool_response(
        "task_complete",
        _snapshot(),
        _snapshot(),
        "session-1",
        {
            "recipient_name": "functions.task_complete",
            "parameters": {
                "summary": (
                    "Done. Fresh command: py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q. "
                    "Command label: checkpoint guard pytest. Exit code: 0. "
                    "Artifact path: artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml. "
                    "Checked result: junit tests=1 failures=0."
                )
            },
        },
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "allow"


def test_main_pretool_denies_raw_apply_patch_for_non_task_complete_tool(monkeypatch) -> None:
    baseline = _snapshot()
    current = _snapshot()

    monkeypatch.setattr(checkpoint_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(checkpoint_guard, "load_session_baseline", lambda session_id, repo_root=checkpoint_guard.REPO_ROOT: baseline)
    monkeypatch.setattr(checkpoint_guard, "discover_repo_root", lambda start_path: REPO_ROOT)

    payload = {
        "hookEventName": "PreToolUse",
        "sessionId": "session-1",
        "cwd": str(REPO_ROOT),
        "recipient_name": "functions.apply_patch",
        "parameters": {"patch": "*** Begin Patch\n*** End Patch\n"},
    }

    old_stdin = sys.stdin
    sys.stdin = io.StringIO(json.dumps(payload))
    try:
        buf = io.StringIO()
        with contextlib.redirect_stdout(buf):
            rc = checkpoint_guard.main()
        output = json.loads(buf.getvalue())
    finally:
        sys.stdin = old_stdin

    assert rc == 0
    hook = output["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "Raw apply_patch is forbidden" in hook["permissionDecisionReason"]


def test_build_posttool_response_reminds_when_head_advanced_without_validation_receipt(tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    response = build_posttool_response("run_in_terminal", baseline, current, "session-1", tmp_path)

    assert response is not None
    assert "Validation debt after run_in_terminal" in response["systemMessage"]
    assert "def456.json" in response["systemMessage"]


def test_stop_hook_blocks_dirty_stop(monkeypatch) -> None:
    baseline = _snapshot(unstaged={"AGENTS.md": "hash-a"})
    current = _snapshot(unstaged={"AGENTS.md": "hash-b"}, untracked={"artifacts/report.txt": "hash-r"})

    monkeypatch.setattr(stop_hook.checkpoint_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(stop_hook.checkpoint_guard, "_bootstrap_missing_baseline_if_clean", lambda session_id, current_snapshot, repo_root=checkpoint_guard.REPO_ROOT: baseline)
    monkeypatch.setattr(stop_hook.checkpoint_guard, "discover_repo_root", lambda start_path: REPO_ROOT)

    payload = {
        "hookEventName": "Stop",
        "sessionId": "session-1",
        "cwd": str(REPO_ROOT),
    }

    rc, output = _invoke_hook_main(stop_hook, payload)

    assert rc == 0
    hook = output["hookSpecificOutput"]
    assert hook["decision"] == "block"
    assert "AGENTS.md" in hook["reason"]
    assert "artifacts/report.txt" in hook["reason"]


def test_evaluate_validation_receipt_guard_allows_matching_receipt(tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"
    receipt_path = validation_receipt_path("def456", tmp_path)
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    receipt_path.write_text(json.dumps({"head": "def456", "summary": "validated"}), encoding="utf-8")

    should_block, reason = evaluate_validation_receipt_guard(baseline, current, tmp_path)

    assert should_block is False
    assert reason == ""


def test_write_validation_receipt_records_current_clean_head(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)

    path = write_validation_receipt(
        "native and runtime validation passed",
        repo_root=repo_root,
        commands=["ui_app/build_tests_vsdevcmd.cmd", "ui_app/build_vsdevcmd.cmd"],
    )
    receipt = load_validation_receipt(path.stem, repo_root)

    assert path.exists()
    assert receipt is not None
    assert receipt["head"] == path.stem
    assert receipt["clean"] is True
    assert receipt["commands"] == ["ui_app/build_tests_vsdevcmd.cmd", "ui_app/build_vsdevcmd.cmd"]
    assert receipt["git"]["head"] == path.stem
    assert receipt["git"]["branch"]
    assert receipt["git"]["status_short"][0].startswith("## ")
    assert all(not line.startswith("?? ") for line in receipt["git"]["status_short"])


def test_write_validation_receipt_rejects_viewer_first_missing_published_runtime_proof(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)

    contract_path = repo_root / "docs" / "contracts" / "slice.contract.json"
    contract_path.parent.mkdir(parents=True)
    contract_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "viewer_first",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["runtime proof"],
                "forbidden_operator_prompts": ["helper-only proof"],
                "required_defaults": {"runtime_publish": "required"},
                "forbidden_defaults": {"helper_only": "forbidden"},
                "required_validation_commands": ["ui_app/build_vsdevcmd.cmd"],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "contract_schema_valid",
                        "description": "contract schema valid",
                        "evidence_kind": "validator_json",
                        "artifact_path": "artifacts/validation/contract.json",
                        "json_path": "ok",
                        "equals": True
                    }
                ]
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    state_path = contract_state.contract_state_path_for_session(contract_state.GLOBAL_CONTRACT_SESSION_ID, repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "viewer_first",
                "contract_path": "docs/contracts/slice.contract.json",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "contract_hash": contract_state.hash_file(contract_path),
                "allowed_mutation_scope": ["tools"],
                "required_validation_commands": ["ui_app/build_vsdevcmd.cmd"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    subprocess.run(["git", "add", "."], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "lock viewer-first slice"], cwd=repo_root, check=True, capture_output=True, text=True)

    try:
        write_validation_receipt(
            "viewer-first publish only",
            repo_root=repo_root,
            commands=["ui_app/build_vsdevcmd.cmd"],
        )
        assert False, "Expected viewer-first validation receipt write to reject missing published-runtime proof"
    except RuntimeError as exc:
        assert "published-runtime proof" in str(exc)


def test_write_validation_receipt_accepts_viewer_first_publish_and_runtime_proof(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)

    contract_path = repo_root / "docs" / "contracts" / "slice.contract.json"
    contract_path.parent.mkdir(parents=True)
    contract_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "viewer_first",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["runtime proof"],
                "forbidden_operator_prompts": ["helper-only proof"],
                "required_defaults": {"runtime_publish": "required"},
                "forbidden_defaults": {"helper_only": "forbidden"},
                "required_validation_commands": ["ui_app/build_vsdevcmd.cmd"],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "contract_schema_valid",
                        "description": "contract schema valid",
                        "evidence_kind": "validator_json",
                        "artifact_path": "artifacts/validation/contract.json",
                        "json_path": "ok",
                        "equals": True
                    }
                ]
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    state_path = contract_state.contract_state_path_for_session(contract_state.GLOBAL_CONTRACT_SESSION_ID, repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "viewer_first",
                "contract_path": "docs/contracts/slice.contract.json",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "contract_hash": contract_state.hash_file(contract_path),
                "allowed_mutation_scope": ["tools"],
                "required_validation_commands": ["ui_app/build_vsdevcmd.cmd"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    subprocess.run(["git", "add", "."], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "lock viewer-first slice"], cwd=repo_root, check=True, capture_output=True, text=True)

    path = write_validation_receipt(
        "viewer-first publish and runtime proof",
        repo_root=repo_root,
        commands=[
            "ui_app/build_vsdevcmd.cmd",
            "py -3.14 -m pytest tests/test_explaino_runtime_walk_tool.py -q",
        ],
    )

    assert path.exists()


def test_write_validation_receipt_rejects_dirty_repo(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("dirty\n", encoding="utf-8")

    try:
        write_validation_receipt("should fail", repo_root=repo_root)
        assert False, "Expected dirty repo validation receipt write to fail"
    except RuntimeError as exc:
        assert "clean repository state" in str(exc)


def test_build_stop_response_blocks_dirty_stop() -> None:
    baseline = _snapshot(unstaged={"AGENTS.md": "hash-a"})
    current = _snapshot(unstaged={"AGENTS.md": "hash-b"}, untracked={"artifacts/report.txt": "hash-r"})

    response = build_stop_response(baseline, current, "session-1")

    assert response is not None
    payload = response["hookSpecificOutput"]
    assert payload["decision"] == "block"
    assert "AGENTS.md" in payload["reason"]
    assert "artifacts/report.txt" in payload["reason"]


def test_summarize_changed_paths_truncates_long_lists() -> None:
    text = summarize_changed_paths([f"file_{index}.txt" for index in range(8)], limit=3)
    assert text == "file_0.txt, file_1.txt, file_2.txt, ... (+5 more)"


def test_build_userprompt_response_warns_when_state_differs_from_baseline() -> None:
    baseline = _snapshot()
    current = _snapshot(unstaged={"ui_app/src/main.cpp": "hash-main"})

    response = build_userprompt_response(baseline, current, "Please do the next task", "session-1")

    assert response is not None
    assert response["continue"] is False
    assert "session baseline" in response["systemMessage"]
    assert "ui_app/src/main.cpp" in response["systemMessage"]
    assert "Please do the next task" in response["systemMessage"]


def test_build_userprompt_response_ignores_clean_state() -> None:
    baseline = _snapshot()
    current = _snapshot()

    response = build_userprompt_response(baseline, current, "fresh prompt", "session-1")
    assert response is not None
    assert "STRICT REPO RULE" in response["systemMessage"]


def test_build_userprompt_response_warns_when_head_advanced_without_receipt(tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    response = build_userprompt_response(baseline, current, "do another task", "session-1", tmp_path)

    assert response is not None
    assert response["continue"] is False
    assert "validation receipt" in response["systemMessage"]
    assert "def456.json" in response["systemMessage"]


def test_build_userprompt_response_requires_explicit_crash_recovery_when_dirty_baseline_missing() -> None:
    current = _snapshot(unstaged={"ui_app/src/main.cpp": "hash-main"})
    resolution = checkpoint_guard.SessionBaselineResolution(
        baseline=None,
        status="adoption_required",
        changed_paths=["ui_app/src/main.cpp"],
    )

    response = build_userprompt_response(
        None,
        current,
        "Please keep going",
        "session-1",
        baseline_resolution=resolution,
    )

    assert response is not None
    assert response["continue"] is False
    assert "Crash recovery required" in response["systemMessage"]
    assert "viewer_host_recover_crash_state.py" in response["systemMessage"]
    assert "ui_app/src/main.cpp" in response["systemMessage"]


def test_build_userprompt_response_requires_rerun_when_recovery_adoption_is_stale() -> None:
    current = _snapshot(unstaged={"ui_app/src/main.cpp": "hash-main"})
    resolution = checkpoint_guard.SessionBaselineResolution(
        baseline=None,
        status="adoption_stale",
        changed_paths=["ui_app/src/main.cpp"],
        recovery_report_path="artifacts/hooks/viewer_host_checkpoint_guard/recovery/recovery_20260508T210000Z_deadbeef.json",
        recovery_adoption_path="artifacts/hooks/viewer_host_checkpoint_guard/recovery/active_recovery_adoption.json",
    )

    response = build_userprompt_response(
        None,
        current,
        "Please keep going",
        "session-1",
        baseline_resolution=resolution,
    )

    assert response is not None
    assert response["continue"] is False
    assert "no longer matches the current dirty snapshot" in response["systemMessage"]
    assert "viewer_host_recover_crash_state.py" in response["systemMessage"]


def test_build_userprompt_response_resumes_after_dirty_recovery_adoption() -> None:
    current = _snapshot(unstaged={"ui_app/src/main.cpp": "hash-main"})
    resolution = checkpoint_guard.SessionBaselineResolution(
        baseline=current,
        status="adopted_dirty",
        changed_paths=["ui_app/src/main.cpp"],
        recovery_report_path="artifacts/hooks/viewer_host_checkpoint_guard/recovery/recovery_20260508T210000Z_deadbeef.json",
    )

    response = build_userprompt_response(
        current,
        current,
        "Continue this slice",
        "session-1",
        baseline_resolution=resolution,
    )

    assert response is not None
    assert response["continue"] is True
    assert "Crash recovery resumed" in response["systemMessage"]
    assert "Continue this slice" in response["systemMessage"]
    assert "recovery_20260508T210000Z_deadbeef.json" in response["systemMessage"]


def test_dirty_prompt_guard_main_writes_carryover_state_for_dirty_prompt(monkeypatch, tmp_path: Path) -> None:
    current = _snapshot(unstaged={"ui_app/src/main.cpp": "hash-main"})
    resolution = checkpoint_guard.SessionBaselineResolution(
        baseline=_snapshot(),
        status="existing",
        changed_paths=["ui_app/src/main.cpp"],
    )

    monkeypatch.setattr(checkpoint_dirty_prompt_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(checkpoint_dirty_prompt_guard, "resolve_session_baseline", lambda session_id, current_snapshot, repo_root=checkpoint_guard.REPO_ROOT: resolution)
    monkeypatch.setattr(checkpoint_dirty_prompt_guard, "discover_repo_root", lambda start_path: tmp_path)

    payload = {
        "hookEventName": "UserPromptSubmit",
        "sessionId": "session-1",
        "cwd": str(tmp_path),
        "prompt": "Start implementation",
    }

    rc, output = _invoke_hook_main(checkpoint_dirty_prompt_guard, payload)

    assert rc == 0
    assert output["continue"] is False
    state = load_carryover_state(tmp_path)
    assert state["active"] is True
    assert state["changed_paths"] == ["ui_app/src/main.cpp"]
    assert "Start implementation" in state["prompt_excerpt"]


def test_dirty_prompt_guard_main_clears_carryover_state_when_prompt_is_clean(monkeypatch, tmp_path: Path) -> None:
    write_carryover_state(["ui_app/src/main.cpp"], "Start implementation", reason="existing", repo_root=tmp_path)
    current = _snapshot()
    resolution = checkpoint_guard.SessionBaselineResolution(
        baseline=current,
        status="existing",
        changed_paths=[],
    )

    monkeypatch.setattr(checkpoint_dirty_prompt_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(checkpoint_dirty_prompt_guard, "resolve_session_baseline", lambda session_id, current_snapshot, repo_root=checkpoint_guard.REPO_ROOT: resolution)
    monkeypatch.setattr(checkpoint_dirty_prompt_guard, "discover_repo_root", lambda start_path: tmp_path)

    payload = {
        "hookEventName": "UserPromptSubmit",
        "sessionId": "session-1",
        "cwd": str(tmp_path),
        "prompt": "Fresh prompt",
    }

    rc, output = _invoke_hook_main(checkpoint_dirty_prompt_guard, payload)

    assert rc == 0
    assert output["continue"] is True
    assert load_carryover_state(tmp_path) == {}


def test_carryover_hook_blocks_unrelated_tool_when_dirty_state_active(monkeypatch, tmp_path: Path) -> None:
    write_carryover_state(["ui_app/src/main.cpp"], "Start implementation", reason="existing", repo_root=tmp_path)
    current = _snapshot(unstaged={"ui_app/src/main.cpp": "hash-main"})

    monkeypatch.setattr(carryover_hook.checkpoint_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(carryover_hook.checkpoint_guard, "discover_repo_root", lambda start_path: tmp_path)

    payload = {
        "hookEventName": "PreToolUse",
        "sessionId": "session-1",
        "cwd": str(tmp_path),
        "recipient_name": "functions.apply_patch",
    }

    rc, output = _invoke_hook_main(carryover_hook, payload)

    assert rc == 2
    hook = output["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "carryover dirty-worktree guard" in hook["permissionDecisionReason"].lower()
    assert "ui_app/src/main.cpp" in hook["permissionDecisionReason"]


def test_carryover_hook_allows_read_only_tool_when_dirty_state_active(monkeypatch, tmp_path: Path) -> None:
    write_carryover_state(["ui_app/src/main.cpp"], "Start implementation", reason="existing", repo_root=tmp_path)
    current = _snapshot(unstaged={"ui_app/src/main.cpp": "hash-main"})

    monkeypatch.setattr(carryover_hook.checkpoint_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(carryover_hook.checkpoint_guard, "discover_repo_root", lambda start_path: tmp_path)

    payload = {
        "hookEventName": "PreToolUse",
        "sessionId": "session-1",
        "cwd": str(tmp_path),
        "recipient_name": "functions.read_file",
    }

    rc, output = _invoke_hook_main(carryover_hook, payload)

    assert rc == 0
    assert output["hookSpecificOutput"]["permissionDecision"] == "allow"


def test_build_pretool_response_denies_task_complete_when_explicit_user_asks_remain_open(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        plan_text=(
            "# Plan\n\n"
            "## Current Phase\n\n"
            "Phase 1 in progress\n\n"
            "## Phase Checklist\n\n"
            "- [ ] Phase 1 - X\n\n"
            "## Explicit User Asks\n\n"
            "- [open] Fix the real bug\n"
            "- [done] Keep scope bounded\n"
        ),
    )

    response = build_pretool_response(
        "task_complete",
        _snapshot(),
        _snapshot(),
        "session-1",
        {"recipient_name": "functions.task_complete"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "explicit user asks" in hook["permissionDecisionReason"].lower()
    assert "Fix the real bug" in hook["additionalContext"]


def test_build_stop_response_blocks_when_explicit_user_asks_remain_open(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        plan_text=(
            "# Plan\n\n"
            "## Current Phase\n\n"
            "Phase 1 in progress\n\n"
            "## Phase Checklist\n\n"
            "- [ ] Phase 1 - X\n\n"
            "## Explicit User Asks\n\n"
            "- [open] Land the runtime proof\n"
        ),
    )

    response = build_stop_response(_snapshot(), _snapshot(), "session-1", tmp_path)

    assert response is not None
    payload = response["hookSpecificOutput"]
    assert payload["decision"] == "block"
    assert "explicit user asks" in payload["reason"].lower()
    assert "Land the runtime proof" in payload["reason"]


def test_build_pretool_response_denies_task_complete_when_hostile_audit_is_pending(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        plan_text=(
            "# Plan\n\n"
            "## Current Phase\n\n"
            "Phase 1 in progress\n\n"
            "## Phase Checklist\n\n"
            "- [ ] Phase 1 - X\n\n"
            "## Explicit User Asks\n\n"
            "- [done] Start implementation\n\n"
            "## Hostile Audit\n\n"
            "- Status: pending\n\n"
            "## Audit Passes\n\n"
            "- [open] Pass 1 - hostile review still pending\n"
        ),
    )

    response = build_pretool_response(
        "task_complete",
        _snapshot(),
        _snapshot(),
        "session-1",
        {"recipient_name": "functions.task_complete"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "hostile" in hook["permissionDecisionReason"].lower()


def test_build_stop_response_blocks_when_hostile_audit_is_pending(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        plan_text=(
            "# Plan\n\n"
            "## Current Phase\n\n"
            "Phase 1 in progress\n\n"
            "## Phase Checklist\n\n"
            "- [ ] Phase 1 - X\n\n"
            "## Explicit User Asks\n\n"
            "- [done] Start implementation\n\n"
            "## Hostile Audit\n\n"
            "- Status: pending\n\n"
            "## Audit Passes\n\n"
            "- [open] Pass 1 - hostile review still pending\n"
        ),
    )

    response = build_stop_response(_snapshot(), _snapshot(), "session-1", tmp_path)

    assert response is not None
    payload = response["hookSpecificOutput"]
    assert payload["decision"] == "block"
    assert "hostile" in payload["reason"].lower()


def test_validate_hostile_audit_plan_blocks_when_repaired_state_is_not_proven(tmp_path: Path) -> None:
    plan_path = tmp_path / "plan_PHASED_PLAN.md"
    plan_path.write_text(
        "# Plan\n\n"
        "## Hostile Audit\n\n"
        "- Status: done\n\n"
        "## Audit Passes\n\n"
        "- [done] Pass 1 - found a regression in the guard behavior and repaired it\n"
        "- [done] Pass 2 - reran the focused validations after the repair\n"
        "- [done] Pass 3 - reread the touched seams after validation\n\n"
        "## Audit Findings\n\n"
        "- [done] Real defect found and repaired: the first hostile-audit implementation still allowed false closure after a finding.\n",
        encoding="utf-8",
    )

    payload = validate_hostile_audit_plan(plan_path)

    assert payload["ok"] is False
    assert payload["audit_passes_completed"] == 3
    assert payload["real_finding_recorded"] is True
    assert "prove the repaired state" in str(payload["blocked_reason"])


def test_validate_hostile_audit_plan_allows_repaired_state_once_clean_reaudit_is_recorded(tmp_path: Path) -> None:
    plan_path = tmp_path / "plan_PHASED_PLAN.md"
    plan_path.write_text(
        "# Plan\n\n"
        "## Hostile Audit\n\n"
        "- Status: done\n\n"
        "## Audit Passes\n\n"
        "- [done] Pass 1 - found a regression in the guard behavior and repaired it\n"
        "- [done] Pass 2 - reran the focused validations after the repair and re-read the repaired state\n"
        "- [done] Pass 3 - re-read the repaired state again and confirmed no additional real defect was found\n\n"
        "## Audit Findings\n\n"
        "- [done] Real defect found and repaired: the first hostile-audit implementation still allowed false closure after a finding.\n"
        "- [done] No additional real defect found in the repaired state after the focused re-audit.\n",
        encoding="utf-8",
    )

    payload = validate_hostile_audit_plan(plan_path)

    assert payload["ok"] is True
    assert payload["audit_passes_completed"] == 3
    assert payload["real_finding_recorded"] is True


def test_build_pretool_response_denies_task_complete_when_repaired_state_is_not_proven(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        plan_text=(
            "# Plan\n\n"
            "## Current Phase\n\n"
            "Phase 1 in progress\n\n"
            "## Phase Checklist\n\n"
            "- [ ] Phase 1 - X\n\n"
            "## Explicit User Asks\n\n"
            "- [done] Start implementation\n\n"
            "## Hostile Audit\n\n"
            "- Status: done\n\n"
            "## Audit Passes\n\n"
            "- [done] Pass 1 - found a regression in the guard behavior and repaired it\n"
            "- [done] Pass 2 - reran the focused validations after the repair\n"
            "- [done] Pass 3 - reread the touched seams after validation\n\n"
            "## Audit Findings\n\n"
            "- [done] Real defect found and repaired: the first hostile-audit implementation still allowed false closure after a finding.\n"
        ),
    )

    response = build_pretool_response(
        "task_complete",
        _snapshot(),
        _snapshot(),
        "session-1",
        {"recipient_name": "functions.task_complete"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "hostile review is incomplete" in hook["permissionDecisionReason"].lower()
    assert "prove the repaired state" in hook["permissionDecisionReason"].lower()


def test_build_pretool_response_denies_task_complete_when_salt_ndepend_gate_is_open(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        contract_id="ui_integration_harness_completion",
        workflow_type="workflow_only",
        plan_text=SALT_NDEPEND_READY_PLAN_TEXT,
        required_defaults={
            "salt_ndepend_gate_is_explicit": "required",
            "deterministic_scoped_coverage_is_required": "required",
        },
    )
    doctor_path = tmp_path / "artifacts" / "salt_ndepend" / "latest" / "doctor.json"
    doctor_path.parent.mkdir(parents=True, exist_ok=True)
    doctor_path.write_text(
        json.dumps(
            {
                "freeze_ready": False,
                "findings": [
                    {
                        "code": "required_blocker_contract_not_green:runtime_ui_harness_contract.v1",
                        "reason": "family-parity reports runtime_ui_harness_contract.v1 status=missing_required_producers.",
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    response = build_pretool_response(
        "task_complete",
        _snapshot(),
        _snapshot(),
        "session-1",
        {"recipient_name": "functions.task_complete"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "salt_ndepend" in hook["permissionDecisionReason"].lower()
    assert "packet gate" in hook["permissionDecisionReason"].lower()
    assert "runtime_ui_harness_contract.v1" in hook["additionalContext"]


def test_build_stop_response_blocks_when_salt_ndepend_gate_is_open(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        contract_id="ui_integration_harness_completion",
        workflow_type="workflow_only",
        plan_text=SALT_NDEPEND_READY_PLAN_TEXT,
        required_defaults={
            "salt_ndepend_gate_is_explicit": "required",
            "deterministic_scoped_coverage_is_required": "required",
        },
    )
    doctor_path = tmp_path / "artifacts" / "salt_ndepend" / "latest" / "doctor.json"
    doctor_path.parent.mkdir(parents=True, exist_ok=True)
    doctor_path.write_text(
        json.dumps(
            {
                "freeze_ready": False,
                "findings": [
                    {
                        "code": "required_blocker_contract_not_green:runtime_ui_harness_contract.v1",
                        "reason": "family-parity reports runtime_ui_harness_contract.v1 status=missing_required_producers.",
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    response = build_stop_response(_snapshot(), _snapshot(), "session-1", tmp_path)

    assert response is not None
    payload = response["hookSpecificOutput"]
    assert payload["decision"] == "block"
    assert "salt_ndepend" in payload["reason"].lower()
    assert "packet gate" in payload["reason"].lower()


def test_build_pretool_response_allows_task_complete_when_salt_ndepend_gate_is_ready(tmp_path: Path) -> None:
    _write_active_contract_with_plan(
        tmp_path,
        contract_id="ui_integration_harness_completion",
        workflow_type="workflow_only",
        plan_text=SALT_NDEPEND_READY_PLAN_TEXT,
        required_defaults={
            "salt_ndepend_gate_is_explicit": "required",
            "deterministic_scoped_coverage_is_required": "required",
        },
    )
    doctor_path = tmp_path / "artifacts" / "salt_ndepend" / "latest" / "doctor.json"
    doctor_path.parent.mkdir(parents=True, exist_ok=True)
    doctor_path.write_text(
        json.dumps(
            {
                "freeze_ready": True,
                "findings": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    response = build_pretool_response(
        "task_complete",
        _snapshot(),
        _snapshot(),
        "session-1",
        {"recipient_name": "functions.task_complete"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "allow"


def test_build_dirty_prompt_message_mentions_closure_flow() -> None:
    text = build_dirty_prompt_message(["HANDOFF_LOG.md", "ui_app/src/main.cpp"], "Start implementation")

    assert "HANDOFF_LOG.md" in text
    assert "workflow context only" in text
    assert "Start implementation" in text


def test_session_start_does_not_auto_capture_dirty_baseline_without_recovery_adoption(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    (repo_root / "README.md").write_text("changed\n", encoding="utf-8")

    response = checkpoint_guard._session_start_response("session-1", repo_root)

    payload = response["hookSpecificOutput"]
    assert "did not capture a dirty-session baseline automatically" in payload["additionalContext"]
    assert "viewer_host_recover_crash_state.py" in payload["additionalContext"]
    assert not checkpoint_guard.state_path_for_session("session-1", repo_root).exists()


def test_resolve_session_baseline_adopts_dirty_snapshot_and_consumes_recovery_artifact(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    (repo_root / "README.md").write_text("changed\n", encoding="utf-8")
    snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    report_path = checkpoint_guard.recovery_report_path_for_snapshot(snapshot, repo_root)
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text('{"ok": true}\n', encoding="utf-8")
    checkpoint_guard.write_active_recovery_adoption(
        snapshot,
        report_path=report_path,
        summary="host crashed",
        reason="dirty baseline missing",
        repo_root=repo_root,
    )

    resolution = checkpoint_guard.resolve_session_baseline("session-1", snapshot, repo_root)

    assert resolution.status == "adopted_dirty"
    assert resolution.baseline == snapshot
    assert checkpoint_guard.state_path_for_session("session-1", repo_root).exists()
    assert report_path.exists()
    assert not checkpoint_guard.recovery_adoption_path(repo_root).exists()


def test_resolve_session_baseline_replaces_stale_existing_baseline_when_recovery_adoption_matches(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    readme = repo_root / "README.md"
    readme.write_text("changed once\n", encoding="utf-8")
    first_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    checkpoint_guard.write_session_baseline("unknown_session", first_snapshot, repo_root)

    readme.write_text("changed twice\n", encoding="utf-8")
    current_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    report_path = checkpoint_guard.recovery_report_path_for_snapshot(current_snapshot, repo_root)
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text('{"ok": true}\n', encoding="utf-8")
    checkpoint_guard.write_active_recovery_adoption(
        current_snapshot,
        report_path=report_path,
        summary="host crashed again",
        reason="stale unknown_session baseline masked recovery adoption",
        repo_root=repo_root,
    )
    snapshot_with_recovery_artifacts = checkpoint_guard.capture_repo_snapshot(repo_root)

    resolution = checkpoint_guard.resolve_session_baseline("unknown_session", snapshot_with_recovery_artifacts, repo_root)

    assert resolution.status == "adopted_dirty"
    assert resolution.baseline == current_snapshot
    assert checkpoint_guard.load_session_baseline("unknown_session", repo_root) == current_snapshot
    assert not checkpoint_guard.recovery_adoption_path(repo_root).exists()


def test_resolve_session_baseline_refreshes_stale_clean_baseline_when_current_head_is_receipted(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    (repo_root / ".gitignore").write_text("artifacts/\n", encoding="utf-8")
    subprocess.run(["git", "add", ".gitignore"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "ignore artifacts"], cwd=repo_root, check=True, capture_output=True, text=True)
    contract_file_path, _ = _write_active_contract_with_plan(
        repo_root,
        session_id=contract_state.GLOBAL_CONTRACT_SESSION_ID,
        plan_text=TEST_PLAN_TEXT,
    )
    subprocess.run(["git", "add", "docs"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "lock slice contract"], cwd=repo_root, check=True, capture_output=True, text=True)
    first_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    checkpoint_guard.write_session_baseline("unknown_session", first_snapshot, repo_root)

    readme = repo_root / "README.md"
    readme.write_text("closed slice\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "closed slice"], cwd=repo_root, check=True, capture_output=True, text=True)
    current_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    current_head = str(current_snapshot["head"])

    validation_path = validation_receipt_path(current_head, repo_root)
    validation_path.parent.mkdir(parents=True, exist_ok=True)
    validation_path.write_text(json.dumps({"head": current_head}, indent=2) + "\n", encoding="utf-8")

    contract_receipt_path = contract_proof_receipt_path(current_head, repo_root)
    contract_receipt_path.parent.mkdir(parents=True, exist_ok=True)
    contract_receipt_path.write_text(
        json.dumps(
            {
                "head": current_head,
                "contract_id": "slice",
                "contract_hash": contract_state.hash_file(contract_file_path),
            },
            indent=2,
        ) + "\n",
        encoding="utf-8",
    )

    resolution = checkpoint_guard.resolve_session_baseline("unknown_session", current_snapshot, repo_root)

    assert resolution.status == "refreshed_clean"
    assert resolution.baseline == current_snapshot
    assert checkpoint_guard.load_session_baseline("unknown_session", repo_root) == current_snapshot


def test_resolve_session_baseline_refreshes_stale_clean_baseline_when_current_head_has_validation_receipt(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    (repo_root / ".gitignore").write_text("artifacts/\n", encoding="utf-8")
    subprocess.run(["git", "add", ".gitignore"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "ignore artifacts"], cwd=repo_root, check=True, capture_output=True, text=True)
    _write_active_contract_with_plan(
        repo_root,
        session_id=contract_state.GLOBAL_CONTRACT_SESSION_ID,
        plan_text=TEST_PLAN_TEXT,
    )
    subprocess.run(["git", "add", "docs"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "lock slice contract"], cwd=repo_root, check=True, capture_output=True, text=True)
    first_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    checkpoint_guard.write_session_baseline("unknown_session", first_snapshot, repo_root)

    readme = repo_root / "README.md"
    readme.write_text("validation receipt only\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "validation receipt only"], cwd=repo_root, check=True, capture_output=True, text=True)
    current_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    current_head = str(current_snapshot["head"])

    validation_path = validation_receipt_path(current_head, repo_root)
    validation_path.parent.mkdir(parents=True, exist_ok=True)
    validation_path.write_text(json.dumps({"head": current_head}, indent=2) + "\n", encoding="utf-8")

    resolution = checkpoint_guard.resolve_session_baseline("unknown_session", current_snapshot, repo_root)

    assert resolution.status == "refreshed_clean_validation_receipted"
    assert resolution.baseline == current_snapshot
    assert checkpoint_guard.load_session_baseline("unknown_session", repo_root) == current_snapshot


def test_resolve_session_baseline_keeps_stale_clean_baseline_when_current_head_lacks_receipts(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    (repo_root / ".gitignore").write_text("artifacts/\n", encoding="utf-8")
    subprocess.run(["git", "add", ".gitignore"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "ignore artifacts"], cwd=repo_root, check=True, capture_output=True, text=True)
    _write_active_contract_with_plan(
        repo_root,
        session_id=contract_state.GLOBAL_CONTRACT_SESSION_ID,
        plan_text=TEST_PLAN_TEXT,
    )
    subprocess.run(["git", "add", "docs"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "lock slice contract"], cwd=repo_root, check=True, capture_output=True, text=True)
    first_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    checkpoint_guard.write_session_baseline("unknown_session", first_snapshot, repo_root)

    readme = repo_root / "README.md"
    readme.write_text("unreceipted commit\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "unreceipted commit"], cwd=repo_root, check=True, capture_output=True, text=True)
    current_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)

    resolution = checkpoint_guard.resolve_session_baseline("unknown_session", current_snapshot, repo_root)

    assert resolution.status == "existing"
    assert resolution.baseline == first_snapshot
    assert checkpoint_guard.load_session_baseline("unknown_session", repo_root) == first_snapshot


def test_resolve_session_baseline_keeps_stale_clean_baseline_when_receipt_metadata_is_malformed(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    _init_git_repo(repo_root)
    (repo_root / ".gitignore").write_text("artifacts/\n", encoding="utf-8")
    subprocess.run(["git", "add", ".gitignore"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "ignore artifacts"], cwd=repo_root, check=True, capture_output=True, text=True)
    _write_active_contract_with_plan(
        repo_root,
        session_id=contract_state.GLOBAL_CONTRACT_SESSION_ID,
        plan_text=TEST_PLAN_TEXT,
    )
    subprocess.run(["git", "add", "docs"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "lock slice contract"], cwd=repo_root, check=True, capture_output=True, text=True)
    first_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    checkpoint_guard.write_session_baseline("unknown_session", first_snapshot, repo_root)

    readme = repo_root / "README.md"
    readme.write_text("bad receipt metadata\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "bad receipt metadata"], cwd=repo_root, check=True, capture_output=True, text=True)
    current_snapshot = checkpoint_guard.capture_repo_snapshot(repo_root)
    current_head = str(current_snapshot["head"])

    validation_path = validation_receipt_path(current_head, repo_root)
    validation_path.parent.mkdir(parents=True, exist_ok=True)
    validation_path.write_text(json.dumps({"head": "wrong-head"}, indent=2) + "\n", encoding="utf-8")

    contract_path = contract_proof_receipt_path(current_head, repo_root)
    contract_path.parent.mkdir(parents=True, exist_ok=True)
    contract_path.write_text(
        json.dumps(
            {
                "head": current_head,
                "contract_id": "wrong-slice",
                "contract_hash": "wrong-hash",
            },
            indent=2,
        ) + "\n",
        encoding="utf-8",
    )

    resolution = checkpoint_guard.resolve_session_baseline("unknown_session", current_snapshot, repo_root)

    assert resolution.status == "existing"
    assert resolution.baseline == first_snapshot
    assert checkpoint_guard.load_session_baseline("unknown_session", repo_root) == first_snapshot


def test_build_validation_receipt_prompt_message_mentions_expected_receipt_path(tmp_path: Path) -> None:
    text = build_validation_receipt_prompt_message("Start implementation", tmp_path, "abc123")

    assert "validation receipt" in text
    assert "does not override closure discipline" in text
    assert "Start implementation" in text
    assert "tool-generated prompts" in text
    assert "abc123.json" in text


def test_hook_config_wires_checkpoint_guard_events() -> None:
    payload = json.loads((REPO_ROOT / ".github" / "hooks" / "checkpoint_guard.json").read_text(encoding="utf-8"))

    assert set(payload["hooks"]) == {"UserPromptSubmit", "SessionStart", "PreToolUse", "PostToolUse", "Stop"}
    prompt_command = payload["hooks"]["UserPromptSubmit"][0]["windows"]
    assert prompt_command == "py -3.14 tools\\viewer_host_checkpoint_dirty_prompt_guard.py"
    assert payload["hooks"]["SessionStart"][0]["windows"] == "py -3.14 tools\\viewer_host_checkpoint_guard.py"
    pretool_commands = [entry["windows"] for entry in payload["hooks"]["PreToolUse"]]
    assert pretool_commands == [
        "py -3.14 tools\\viewer_host_hook_require_checkpoint_carryover.py",
        "py -3.14 tools\\viewer_host_checkpoint_guard.py",
        "py -3.14 tools\\viewer_host_hook_require_checkpoint_before_complete.py",
    ]
    assert payload["hooks"]["PostToolUse"][0]["windows"] == "py -3.14 tools\\viewer_host_checkpoint_guard.py"
    assert payload["hooks"]["Stop"][0]["windows"] == "py -3.14 tools\\viewer_host_hook_stop_if_dirty_worktree.py"


def test_discover_repo_root_normalizes_subdirectory_paths() -> None:
    assert discover_repo_root(REPO_ROOT / "ui_app") == REPO_ROOT


def test_build_pretool_response_denies_raw_apply_patch_even_when_repo_is_clean(tmp_path: Path) -> None:
    baseline = _snapshot()
    current = _snapshot()

    response = build_pretool_response(
        "unknown_tool",
        baseline,
        current,
        "session-1",
        {"recipient_name": "functions.apply_patch"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "Raw apply_patch is forbidden" in hook["permissionDecisionReason"]


def test_build_pretool_response_denies_raw_mutating_shell_command_even_when_repo_is_clean(tmp_path: Path) -> None:
    baseline = _snapshot()
    current = _snapshot()

    response = build_pretool_response(
        "unknown_tool",
        baseline,
        current,
        "session-1",
        {"recipient_name": "functions.shell_command", "command": "git commit -m test"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "Direct handoff/receipt/git mutation is forbidden" in hook["permissionDecisionReason"]


def test_build_pretool_response_denies_chained_shell_after_allowed_prefix(tmp_path: Path) -> None:
    baseline = _snapshot()
    current = _snapshot()

    response = build_pretool_response(
        "unknown_tool",
        baseline,
        current,
        "session-1",
        {
            "recipient_name": "functions.shell_command",
            "command": "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q && git commit -m nope",
        },
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "forbidden" in hook["permissionDecisionReason"].lower()


def test_build_pretool_response_allows_prepare_slice_without_active_contract(tmp_path: Path) -> None:
    baseline = _snapshot()
    current = _snapshot()

    response = build_pretool_response(
        "unknown_tool",
        baseline,
        current,
        "session-1",
        {"recipient_name": "functions.shell_command", "command": "py -3.14 tools\\viewer_host_prepare_slice.py --session-id session-1 --plan foo --contract bar"},
        tmp_path,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "allow"
    assert "STRICT REPO RULE" in hook["permissionDecisionReason"]


def test_build_pretool_response_denies_mutation_when_locked_contract_hash_drifted(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    contract_dir = repo_root / "docs" / "contracts"
    contract_dir.mkdir(parents=True)
    contract_path = contract_dir / "slice.contract.json"
    contract_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["fits"],
                "forbidden_operator_prompts": ["state.json"],
                "required_defaults": {"base_fractal_type": "explaino"},
                "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
                "required_validation_commands": ["pytest"],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "assertion",
                        "description": "placeholder",
                        "evidence_kind": "pytest_junit_case",
                        "artifact_path": "artifacts/pytest/banner.junit.xml",
                        "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner",
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    plan_path = repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md"
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text("# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n", encoding="utf-8")
    state_path = contract_state.contract_state_path_for_session("session-1", repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "contract_path": "docs/contracts/slice.contract.json",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "contract_hash": "stale",
                "allowed_mutation_scope": ["tools"],
                "required_validation_commands": ["pytest"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    response = build_pretool_response(
        "unknown_tool",
        _snapshot(),
        _snapshot(),
        "session-1",
        {"recipient_name": "functions.shell_command", "command": "py -3.14 tools\\viewer_host_run_repo_mutation.py --session-id session-1 -- cmd /c echo mutate"},
        repo_root,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "Active slice contract changed after it was locked" in hook["permissionDecisionReason"]


def test_build_general_pretool_response_denies_mutation_wrapper_without_action_hostile_review(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    _write_active_contract_with_plan(repo_root, plan_text=TEST_PLAN_TEXT)

    response = checkpoint_guard.build_general_pretool_response(
        "session-1",
        {
            "recipient_name": "functions.shell_command",
            "command": "py -3.14 tools\\viewer_host_run_repo_mutation.py --session-id session-1 -- cmd /c echo mutate",
        },
        repo_root,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "Action hostile review is missing" in hook["permissionDecisionReason"]

def test_build_general_pretool_response_denies_legacy_owner_seam_action_hostile_review(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    _write_active_contract_with_plan(
        repo_root,
        plan_text=(
            TEST_PLAN_TEXT
            + "\n## Action Hostile Review\n\n"
            + "- Action ID: action-legacy\n"
            + "- Suspected Failure Mode: legacy hostile review fields can pass without the incident-required blocked action\n"
            + "- Owner Seam: tools/viewer_host_checkpoint_guard.py\n"
            + "- Proof Surface: tests/test_viewer_host_checkpoint_guard.py\n"
        ),
    )

    response = checkpoint_guard.build_general_pretool_response(
        "session-1",
        {
            "recipient_name": "functions.shell_command",
            "command": "py -3.14 tools\\viewer_host_run_repo_mutation.py --session-id session-1 -- cmd /c echo mutate",
        },
        repo_root,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "correct owner/action" in hook["permissionDecisionReason"].lower()


def test_build_general_pretool_response_allows_mutation_wrapper_with_ready_action_hostile_review(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    _write_active_contract_with_plan(
        repo_root,
        plan_text=(
            TEST_PLAN_TEXT
            + "\n## Action Hostile Review\n\n"
            + "- Action ID: action-1\n"
            + "- Suspected Failure Mode: per-action hostile review is not enforced before mutation\n"
            + "- Correct Owner/Action: repair the checkpoint guard before mutation proceeds\n"
            + "- Proof Surface: tests/test_viewer_host_checkpoint_guard.py\n"
            + "- Blocked Action: wrapper mutation without exact per-action hostile review fields\n"
        ),
    )

    response = checkpoint_guard.build_general_pretool_response(
        "session-1",
        {
            "recipient_name": "functions.shell_command",
            "command": "py -3.14 tools\\viewer_host_run_repo_mutation.py --session-id session-1 -- cmd /c echo mutate",
        },
        repo_root,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "allow"


def test_build_general_pretool_response_denies_reused_action_hostile_review_after_consumption(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    _write_active_contract_with_plan(
        repo_root,
        plan_text=(
            TEST_PLAN_TEXT
            + "\n## Action Hostile Review\n\n"
            + "- Action ID: action-1\n"
            + "- Suspected Failure Mode: stale hostile review can be reused across multiple mutations\n"
            + "- Correct Owner/Action: require a fresh action review token for each mutation\n"
            + "- Proof Surface: tests/test_viewer_host_checkpoint_guard.py\n"
            + "- Blocked Action: reusing one action review across multiple mutations\n"
        ),
    )
    checkpoint_guard.write_consumed_action_hostile_review(
        "session-1",
        action_id="action-1",
        plan_path="docs/notes/slice_PHASED_PLAN.md",
        command_text="py -3.14 tools\\viewer_host_run_repo_mutation.py --session-id session-1 -- cmd /c echo mutate",
        repo_root=repo_root,
    )

    response = checkpoint_guard.build_general_pretool_response(
        "session-1",
        {
            "recipient_name": "functions.shell_command",
            "command": "py -3.14 tools\\viewer_host_run_repo_mutation.py --session-id session-1 -- cmd /c echo mutate",
        },
        repo_root,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "deny"
    assert "already been consumed" in hook["permissionDecisionReason"]


def test_build_general_pretool_response_allows_plan_only_patch_wrapper_to_bootstrap_action_hostile_review(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    _write_active_contract_with_plan(repo_root, plan_text=TEST_PLAN_TEXT)
    patch_path = repo_root / "artifacts" / "plan_only.patch"
    patch_path.parent.mkdir(parents=True, exist_ok=True)
    patch_path.write_text(
        "diff --git a/docs/notes/slice_PHASED_PLAN.md b/docs/notes/slice_PHASED_PLAN.md\n"
        "--- a/docs/notes/slice_PHASED_PLAN.md\n"
        "+++ b/docs/notes/slice_PHASED_PLAN.md\n"
        "@@ -1,1 +1,1 @@\n"
        "-# Plan\n"
        "+# Plan\n",
        encoding="utf-8",
    )

    response = checkpoint_guard.build_general_pretool_response(
        "session-1",
        {
            "recipient_name": "functions.shell_command",
            "command": "py -3.14 tools\\viewer_host_apply_repo_patch.py --session-id session-1 --patch-file artifacts\\plan_only.patch",
        },
        repo_root,
    )

    assert response is not None
    hook = response["hookSpecificOutput"]
    assert hook["permissionDecision"] == "allow"


def test_maybe_consume_action_hostile_review_skips_plan_only_bootstrap_patch(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    _write_active_contract_with_plan(
        repo_root,
        plan_text=(
            TEST_PLAN_TEXT
            + "\n## Action Hostile Review\n\n"
            + "- Action ID: action-1\n"
            + "- Suspected Failure Mode: plan bootstrap patches consume the action review token immediately\n"
            + "- Correct Owner/Action: skip consuming action review tokens for plan-only bootstrap patches\n"
            + "- Proof Surface: tests/test_viewer_host_checkpoint_guard.py\n"
            + "- Blocked Action: consuming a review token before the first non-plan mutation\n"
        ),
    )
    patch_path = repo_root / "artifacts" / "plan_only.patch"
    patch_path.parent.mkdir(parents=True, exist_ok=True)
    patch_path.write_text(
        "diff --git a/docs/notes/slice_PHASED_PLAN.md b/docs/notes/slice_PHASED_PLAN.md\n"
        "--- a/docs/notes/slice_PHASED_PLAN.md\n"
        "+++ b/docs/notes/slice_PHASED_PLAN.md\n"
        "@@ -1,1 +1,1 @@\n"
        "-# Plan\n"
        "+# Plan\n",
        encoding="utf-8",
    )

    checkpoint_guard.maybe_consume_action_hostile_review(
        {
            "recipient_name": "functions.shell_command",
            "command": "py -3.14 tools\\viewer_host_apply_repo_patch.py --session-id session-1 --patch-file artifacts\\plan_only.patch",
        },
        "session-1",
        _snapshot(),
        _snapshot(unstaged={"docs/notes/slice_PHASED_PLAN.md": "hash-a"}),
        repo_root,
    )

    assert checkpoint_guard.load_consumed_action_hostile_review("session-1", repo_root) is None


def test_evaluate_contract_proof_receipt_guard_blocks_failed_assertion_results(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    (repo_root / "docs" / "contracts").mkdir(parents=True)
    (repo_root / "docs" / "notes").mkdir(parents=True)
    (repo_root / "tools").mkdir()
    contract_path = repo_root / "docs" / "contracts" / "slice.contract.json"
    contract_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["fits"],
                "forbidden_operator_prompts": ["state.json"],
                "required_defaults": {"base_fractal_type": "explaino"},
                "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
                "required_validation_commands": ["pytest"],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "strict_banner_emitted",
                        "description": "Strict banner emitted",
                        "evidence_kind": "pytest_junit_case",
                        "artifact_path": "artifacts/pytest/banner.junit.xml",
                        "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner",
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    (repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md").write_text(
        "# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )
    state_path = contract_state.contract_state_path_for_session("session-1", repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "contract_path": "docs/contracts/slice.contract.json",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "contract_hash": contract_state.hash_file(contract_path),
                "allowed_mutation_scope": ["tools"],
                "required_validation_commands": ["pytest"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    receipt_path = contract_proof_receipt_path("def456", repo_root)
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    receipt_path.write_text(
        json.dumps(
            {
                "head": "def456",
                "contract_id": "slice",
                "contract_hash": contract_state.hash_file(contract_path),
                "assertion_results": [
                    {
                        "assertion_id": "strict_banner_emitted",
                        "ok": False,
                        "failure_detail": "missing case",
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    should_block, reason = checkpoint_guard.evaluate_contract_proof_receipt_guard(
        baseline,
        current,
        "session-1",
        repo_root,
    )

    assert should_block is True
    assert "assertion" in reason.lower()


def test_evaluate_contract_proof_receipt_guard_blocks_missing_required_assertion_result(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    (repo_root / "docs" / "contracts").mkdir(parents=True)
    (repo_root / "docs" / "notes").mkdir(parents=True)
    (repo_root / "tools").mkdir()
    contract_path = repo_root / "docs" / "contracts" / "slice.contract.json"
    contract_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["fits"],
                "forbidden_operator_prompts": ["state.json"],
                "required_defaults": {"base_fractal_type": "explaino"},
                "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
                "required_validation_commands": ["pytest"],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "strict_banner_emitted",
                        "description": "Strict banner emitted",
                        "evidence_kind": "pytest_junit_case",
                        "artifact_path": "artifacts/pytest/banner.junit.xml",
                        "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner",
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    (repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md").write_text(
        "# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )
    state_path = contract_state.contract_state_path_for_session("session-1", repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "contract_path": "docs/contracts/slice.contract.json",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "contract_hash": contract_state.hash_file(contract_path),
                "allowed_mutation_scope": ["tools"],
                "required_validation_commands": ["pytest"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    receipt_path = contract_proof_receipt_path("def456", repo_root)
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    receipt_path.write_text(
        json.dumps(
            {
                "head": "def456",
                "contract_id": "slice",
                "contract_hash": contract_state.hash_file(contract_path),
                "assertion_results": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    should_block, reason = checkpoint_guard.evaluate_contract_proof_receipt_guard(
        baseline,
        current,
        "session-1",
        repo_root,
    )

    assert should_block is True
    assert "missing" in reason.lower()


def test_evaluate_contract_proof_receipt_guard_blocks_when_locked_contract_has_drifted(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    (repo_root / "docs" / "contracts").mkdir(parents=True)
    (repo_root / "docs" / "notes").mkdir(parents=True)
    (repo_root / "tools").mkdir()
    contract_path = repo_root / "docs" / "contracts" / "slice.contract.json"
    contract_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["fits"],
                "forbidden_operator_prompts": ["state.json"],
                "required_defaults": {"base_fractal_type": "explaino"},
                "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
                "required_validation_commands": ["pytest"],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "strict_banner_emitted",
                        "description": "Strict banner emitted",
                        "evidence_kind": "pytest_junit_case",
                        "artifact_path": "artifacts/pytest/banner.junit.xml",
                        "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner",
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    (repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md").write_text(
        "# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )
    state_path = contract_state.contract_state_path_for_session("session-1", repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "contract_path": "docs/contracts/slice.contract.json",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "contract_hash": "stale",
                "allowed_mutation_scope": ["tools"],
                "required_validation_commands": ["pytest"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    should_block, reason = checkpoint_guard.evaluate_contract_proof_receipt_guard(
        baseline,
        current,
        "session-1",
        repo_root,
    )

    assert should_block is True
    assert "changed after it was locked" in reason


def test_evaluate_contract_proof_receipt_guard_blocks_viewer_first_missing_published_runtime_proof(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    (repo_root / "docs" / "contracts").mkdir(parents=True)
    (repo_root / "docs" / "notes").mkdir(parents=True)
    (repo_root / "tools").mkdir()
    contract_path = repo_root / "docs" / "contracts" / "slice.contract.json"
    contract_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "viewer_first",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["runtime proof"],
                "forbidden_operator_prompts": ["helper-only proof"],
                "required_defaults": {"runtime_publish": "required"},
                "forbidden_defaults": {"helper_only": "forbidden"},
                "required_validation_commands": ["ui_app/build_vsdevcmd.cmd"],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "strict_banner_emitted",
                        "description": "Strict banner emitted",
                        "evidence_kind": "pytest_junit_case",
                        "artifact_path": "artifacts/pytest/banner.junit.xml",
                        "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner"
                    }
                ]
            },
            indent=2,
        ),
        encoding="utf-8",
    )
    (repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md").write_text(
        "# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )
    state_path = contract_state.contract_state_path_for_session("session-1", repo_root)
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(
            {
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "viewer_first",
                "contract_path": "docs/contracts/slice.contract.json",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "contract_hash": contract_state.hash_file(contract_path),
                "allowed_mutation_scope": ["tools"],
                "required_validation_commands": ["ui_app/build_vsdevcmd.cmd"],
                "required_validators": [],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    receipt_path = validation_receipt_path("def456", repo_root)
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    receipt_path.write_text(
        json.dumps(
            {
                "head": "def456",
                "summary": "publish only",
                "commands": ["ui_app/build_vsdevcmd.cmd"],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    should_block, reason = checkpoint_guard.evaluate_contract_proof_receipt_guard(
        baseline,
        current,
        "session-1",
        repo_root,
    )

    assert should_block is True
    assert "published-runtime proof" in reason
