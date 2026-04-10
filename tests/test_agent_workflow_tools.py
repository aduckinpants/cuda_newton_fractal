from __future__ import annotations

import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_begin_work_slice import build_breadcrumb_message
from tools.viewer_host_session_bootstrap import tail_handoff_entries
from tools.viewer_host_assert_phased_plan_sync import validate_plan_text


def test_build_handoff_message_normalizes_fields() -> None:
    message = build_breadcrumb_message(
        branch=" feature/test ",
        head=" abc123 ",
        dirty=True,
        intent=" tighten  session | bootstrap ",
        profile="runtime",
    )
    assert message == (
        "session-start | branch=feature/test | head=abc123 | status=dirty | "
        "profile=runtime | intent=tighten session / bootstrap"
    )


def test_tail_handoff_entries_returns_latest_checkpoint_lines() -> None:
    text = """
- `ck:00000001` older
- noise entry
- `ck:00000002` newer
- `ck:00000003` newest
"""
    assert tail_handoff_entries(text, 2) == [
        "`ck:00000002` newer",
        "`ck:00000003` newest",
    ]


def test_validate_plan_text_accepts_first_unchecked_current_phase() -> None:
    text = """
# Example

## Current Phase

Phase 2 - Runtime lock-down

## Phase Checklist

- [x] Phase 1 - Bootstrap
- [ ] Phase 2 - Runtime lock-down
- [ ] Phase 3 - Cleanup
"""
    assert validate_plan_text(text, display_path="docs/notes/example_PHASED_PLAN.md") is None


def test_validate_plan_text_rejects_current_phase_drift() -> None:
    text = """
# Example

## Current Phase

Phase 3 - Cleanup

## Phase Checklist

- [x] Phase 1 - Bootstrap
- [ ] Phase 2 - Runtime lock-down
- [ ] Phase 3 - Cleanup
"""
    message = validate_plan_text(text, display_path="docs/notes/example_PHASED_PLAN.md")
    assert message is not None
    assert "unsynced phased checklist" in message


def test_validate_plan_text_requires_complete_when_all_done() -> None:
    text = """
# Example

## Current Phase

Phase 2 - Cleanup

## Phase Checklist

- [x] Phase 1 - Bootstrap
- [x] Phase 2 - Cleanup
"""
    assert validate_plan_text(text, display_path="docs/notes/example_PHASED_PLAN.md") is None


def test_tasks_surface_exposes_profile_tasks() -> None:
    tasks_json = (REPO_ROOT / ".vscode" / "tasks.json").read_text(encoding="utf-8")
    assert '"label": "verify: profile native"' in tasks_json
    assert '"label": "verify: profile runtime"' in tasks_json
    assert '"label": "verify: profile catalog"' in tasks_json
    assert '"label": "verify: profile checkpoint"' in tasks_json