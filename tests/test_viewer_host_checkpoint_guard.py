from __future__ import annotations

import json
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_checkpoint_guard import (
    build_pretool_response,
    build_stop_response,
    compare_snapshots,
    discover_repo_root,
    evaluate_checkpoint_guard,
    summarize_changed_paths,
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


def test_hook_config_wires_checkpoint_guard_events() -> None:
    payload = json.loads((REPO_ROOT / ".github" / "hooks" / "checkpoint_guard.json").read_text(encoding="utf-8"))

    assert set(payload["hooks"]) == {"SessionStart", "PreToolUse", "Stop"}
    for event_name in ("SessionStart", "PreToolUse", "Stop"):
        command = payload["hooks"][event_name][0]["windows"]
        assert command == "py -3.14 tools\\viewer_host_checkpoint_guard.py"


def test_discover_repo_root_normalizes_subdirectory_paths() -> None:
    assert discover_repo_root(REPO_ROOT / "ui_app") == REPO_ROOT