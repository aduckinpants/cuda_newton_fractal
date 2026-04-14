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
from tools.viewer_host_checkpoint_dirty_prompt_guard import (
    build_dirty_prompt_message,
    build_userprompt_response,
    build_validation_receipt_prompt_message,
)


def _snapshot(*, unstaged: dict[str, str] | None = None, staged: dict[str, str] | None = None, untracked: dict[str, str] | None = None) -> dict[str, object]:
    return {
        "unstaged": unstaged or {},
        "staged": staged or {},
        "untracked": untracked or {},
        "clean": not any((unstaged, staged, untracked)),
    }


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

    response = build_pretool_response("task_complete", baseline, current)

    assert response is not None
    payload = response["hookSpecificOutput"]
    assert payload["permissionDecision"] == "deny"
    assert "checkpoint commit" in payload["permissionDecisionReason"] or "baseline" in payload["permissionDecisionReason"]
    assert "HANDOFF_LOG.md" in payload["additionalContext"]


def test_build_pretool_response_ignores_other_tools() -> None:
    baseline = _snapshot()
    current = _snapshot(unstaged={"AGENTS.md": "hash-a"})

    assert build_pretool_response("run_in_terminal", baseline, current) is None


def test_build_posttool_response_emits_checkpoint_debt_reminder() -> None:
    baseline = _snapshot()
    current = _snapshot(
        unstaged={"docs/notes/plan.md": "hash-plan"},
        staged={"HANDOFF_LOG.md": "blob-1"},
    )

    response = build_posttool_response("apply_patch", baseline, current)

    assert response is not None
    assert "systemMessage" in response
    assert "apply_patch" in response["systemMessage"]
    assert "checkpoint commit" in response["systemMessage"] or "baseline" in response["systemMessage"]
    assert "docs/notes/plan.md" in response["systemMessage"]
    assert "HANDOFF_LOG.md" in response["systemMessage"]


def test_build_posttool_response_ignores_clean_state() -> None:
    baseline = _snapshot()
    current = _snapshot()

    assert build_posttool_response("apply_patch", baseline, current) is None


def test_build_pretool_response_blocks_clean_head_without_validation_receipt(tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    response = build_pretool_response("task_complete", baseline, current, tmp_path)

    assert response is not None
    payload = response["hookSpecificOutput"]
    assert payload["permissionDecision"] == "deny"
    assert "validation receipt" in payload["permissionDecisionReason"]
    assert "def456.json" in payload["additionalContext"]


def test_main_pretool_blocks_task_complete_via_recipient_name(monkeypatch) -> None:
    baseline = _snapshot()
    current = _snapshot(staged={"HANDOFF_LOG.md": "blob-1"})

    monkeypatch.setattr(checkpoint_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(checkpoint_guard, "load_session_baseline", lambda session_id, repo_root=checkpoint_guard.REPO_ROOT: baseline)
    monkeypatch.setattr(checkpoint_guard, "discover_repo_root", lambda start_path: REPO_ROOT)

    payload = {
        "hookEventName": "PreToolUse",
        "sessionId": "session-1",
        "cwd": str(REPO_ROOT),
        "recipient_name": "functions.task_complete",
        "parameters": {"summary": "done"},
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
    assert "session baseline" in hook["permissionDecisionReason"]
    assert "HANDOFF_LOG.md" in hook["additionalContext"]


def test_main_pretool_blocks_missing_validation_receipt_via_recipient_name(monkeypatch, tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    monkeypatch.setattr(checkpoint_guard, "capture_repo_snapshot", lambda repo_root=checkpoint_guard.REPO_ROOT: current)
    monkeypatch.setattr(checkpoint_guard, "load_session_baseline", lambda session_id, repo_root=checkpoint_guard.REPO_ROOT: baseline)
    monkeypatch.setattr(checkpoint_guard, "discover_repo_root", lambda start_path: tmp_path)

    payload = {
        "hookEventName": "PreToolUse",
        "sessionId": "session-1",
        "cwd": str(tmp_path),
        "recipient_name": "functions.task_complete",
        "parameters": {"summary": "done"},
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
    assert "validation receipt" in hook["permissionDecisionReason"]
    assert "def456.json" in hook["additionalContext"]


def test_build_posttool_response_reminds_when_head_advanced_without_validation_receipt(tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    response = build_posttool_response("run_in_terminal", baseline, current, tmp_path)

    assert response is not None
    assert "Validation debt after run_in_terminal" in response["systemMessage"]
    assert "def456.json" in response["systemMessage"]


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

    response = build_stop_response(baseline, current)

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

    response = build_userprompt_response(baseline, current, "Please do the next task")

    assert response is not None
    assert response["continue"] is True
    assert "session baseline" in response["systemMessage"]
    assert "ui_app/src/main.cpp" in response["systemMessage"]
    assert "Please do the next task" in response["systemMessage"]


def test_build_userprompt_response_ignores_clean_state() -> None:
    baseline = _snapshot()
    current = _snapshot()

    assert build_userprompt_response(baseline, current, "fresh prompt") is None


def test_build_userprompt_response_warns_when_head_advanced_without_receipt(tmp_path: Path) -> None:
    baseline = _snapshot()
    baseline["head"] = "abc123"
    current = _snapshot()
    current["head"] = "def456"

    response = build_userprompt_response(baseline, current, "do another task", tmp_path)

    assert response is not None
    assert response["continue"] is True
    assert "validation receipt" in response["systemMessage"]
    assert "def456.json" in response["systemMessage"]


def test_build_dirty_prompt_message_mentions_closure_flow() -> None:
    text = build_dirty_prompt_message(["HANDOFF_LOG.md", "ui_app/src/main.cpp"], "Start implementation")

    assert "HANDOFF_LOG.md" in text
    assert "workflow context only" in text
    assert "Start implementation" in text
    assert "tool-generated prompts" in text
    assert "validation receipt" in text


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
    for event_name in ("SessionStart", "PreToolUse", "PostToolUse", "Stop"):
        command = payload["hooks"][event_name][0]["windows"]
        assert command == "py -3.14 tools\\viewer_host_checkpoint_guard.py"


def test_discover_repo_root_normalizes_subdirectory_paths() -> None:
    assert discover_repo_root(REPO_ROOT / "ui_app") == REPO_ROOT