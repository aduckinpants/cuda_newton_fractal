from __future__ import annotations

import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_begin_work_slice import build_breadcrumb_append_plan, build_breadcrumb_message, main as begin_work_slice_main
from tools.viewer_host_session_bootstrap import build_bootstrap_state, build_validation_profiles, legacy_pending_handoff_entries, tail_handoff_entries
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


def test_build_breadcrumb_append_plan_uses_generated_checkpoint_id(monkeypatch) -> None:
    monkeypatch.setattr("tools.viewer_host_append_handoff.uuid.uuid4", lambda: type("_Uuid", (), {"hex": "flow1234abcdef"})())

    plan = build_breadcrumb_append_plan(
        py="py",
        message="session-start | branch=feature/test | head=abc123 | status=clean | profile=checkpoint | intent=workflow closeout",
    )

    assert plan.checkpoint_id == "ck:flow1234"
    assert plan.generated_checkpoint_id is True
    assert plan.command == [
        "py",
        "C:\\code\\salticid-cuda\\tools\\handoff_append.py",
        "--repo-root",
        str(REPO_ROOT),
        "--commit",
        "ck:flow1234",
        "session-start | branch=feature/test | head=abc123 | status=clean | profile=checkpoint | intent=workflow closeout",
    ]


def test_begin_work_slice_dry_run_surfaces_generated_checkpoint_id(monkeypatch, capsys) -> None:
    def fake_capture_git(*args: str) -> str:
        if args == ("rev-parse", "--abbrev-ref", "HEAD"):
            return "feature/test"
        if args == ("rev-parse", "--short", "HEAD"):
            return "abc123"
        raise AssertionError(args)

    monkeypatch.setattr("tools.viewer_host_begin_work_slice._capture_git", fake_capture_git)
    monkeypatch.setattr("tools.viewer_host_begin_work_slice.repo_is_dirty", lambda _repo_root: False)
    monkeypatch.setattr("tools.viewer_host_append_handoff.uuid.uuid4", lambda: type("_Uuid", (), {"hex": "flow1234abcdef"})())

    assert begin_work_slice_main([
        "--intent",
        "workflow closeout",
        "--profile",
        "checkpoint",
        "--dry-run",
    ]) == 0

    out = capsys.readouterr().out
    assert "session-start | branch=feature/test | head=abc123 | status=clean | profile=checkpoint | intent=workflow closeout" in out
    assert "viewer_host_begin_work_slice: checkpoint_id=ck:flow1234" in out
    assert "--commit ck:flow1234" in out
    assert "--commit pending" not in out


def test_tail_handoff_entries_returns_latest_checkpoint_lines() -> None:
    text = """
- `ck:00000001` older
- noise entry
- `ck:00000002` newer
- `abc12345` committed
- `pending` active
- `ck:00000003` newest
"""
    assert tail_handoff_entries(text, 2) == [
        "`pending` active",
        "`ck:00000003` newest",
    ]


def test_legacy_pending_handoff_entries_finds_unresolvable_lines() -> None:
    text = """
- `ck:old11111` 2026-04-10 10:00 UTC — pending: older legacy entry
- `pending` 2026-04-10 10:05 UTC — session-start | branch=feature/test | head=abc123 | status=dirty | profile=native | intent=example
- `abc12345` 2026-04-10 10:10 UTC — committed note
"""
    assert legacy_pending_handoff_entries(text) == [
        "`ck:old11111` 2026-04-10 10:00 UTC — pending: older legacy entry"
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


def test_repo_workflow_surface_does_not_reintroduce_removed_core_tool_names() -> None:
    old_names = (
        "tools/agent_session_bootstrap.py",
        "tools/agent_begin_work_slice.py",
        "tools/assert_phased_plan_sync.py",
        "tools/run_test_profile.py",
        "tools/test_profile_toolchain.py",
    )
    files = (
        REPO_ROOT / "AGENTS.md",
        REPO_ROOT / "AGENT_WORKING_PROTOCOL.md",
        REPO_ROOT / ".github" / "copilot-instructions.md",
        REPO_ROOT / "docs" / "PHASED_PLAN_CONTINUITY_PROTOCOL.md",
        REPO_ROOT / ".vscode" / "tasks.json",
    )
    for path in files:
        text = path.read_text(encoding="utf-8")
        for old_name in old_names:
            assert old_name not in text, f"{path} unexpectedly references removed tool {old_name}"


def test_bootstrap_surface_advertises_checkpoint_id_handoff_flow() -> None:
    state = build_bootstrap_state(run_audit=False, tail_handoff=1)

    assert state["next_commands"]["append_handoff"] == (
        'py -3.14 tools/viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"'
    )
    assert state["next_commands"]["append_handoff_legacy_pending"] == (
        'py -3.14 tools/viewer_host_append_handoff.py --resolve-last-pending --score <n> "<message>"'
    )


def test_bootstrap_surface_describes_checkpoint_profile_steps_and_outputs() -> None:
    state = build_bootstrap_state(run_audit=False, tail_handoff=1)

    checkpoint = state["validation_profiles"]["checkpoint"]

    assert checkpoint["task_label"] == "verify: profile checkpoint"
    assert checkpoint["steps"][0] == {
        "label": "verify: code quality audit",
        "outputs": ["artifacts/code_quality_report.json"],
    }
    assert checkpoint["steps"][1] == {
        "label": "verify: native helper tests",
        "outputs": ["artifacts/verify_native_helper_tests.log"],
    }
    assert checkpoint["steps"][3] == {
        "label": "verify: runtime probe/session pytest",
        "outputs": ["artifacts/verify_runtime_probe_session_pytest.log"],
    }


def test_bootstrap_surface_describes_catalog_smoke_log_and_output_dir() -> None:
    state = build_bootstrap_state(run_audit=False, tail_handoff=1)

    catalog = state["validation_profiles"]["catalog"]
    smoke_step = catalog["steps"][2]

    assert smoke_step == {
        "label": "verify: catalog smoke",
        "outputs": [
            "artifacts/verify_catalog_smoke.log",
            "artifacts/fractal_catalog_smoke_profile",
        ],
    }


def test_runtime_pytest_task_uses_runtime_lane_helper() -> None:
    tasks_json = (REPO_ROOT / ".vscode" / "tasks.json").read_text(encoding="utf-8")

    assert '"label": "verify: runtime probe/session pytest"' in tasks_json
    assert '"tools/viewer_host_runtime_pytest_lane.py"' in tasks_json


def test_core_workflow_docs_advertise_session_start_checkpoint_flow() -> None:
    expected = {
        REPO_ROOT / "AGENTS.md": [
            "append a session-start breadcrumb first",
            'py -3.14 tools\\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"',
        ],
        REPO_ROOT / "AGENT_WORKING_PROTOCOL.md": [
            "session-start breadcrumb",
            'py -3.14 tools\\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"',
        ],
        REPO_ROOT / ".github" / "copilot-instructions.md": [
            "append a session-start breadcrumb first",
        ],
        REPO_ROOT / "docs" / "PHASED_PLAN_CONTINUITY_PROTOCOL.md": [
            "session-start slice breadcrumb",
        ],
    }

    unexpected = {
        REPO_ROOT / "AGENTS.md": ["append a pending breadcrumb first"],
        REPO_ROOT / "AGENT_WORKING_PROTOCOL.md": [
            'py -3.14 tools\\viewer_host_append_handoff.py --resolve-last-pending --score <n> "<message>"',
            "omit `--commit` for the normal checkpoint-id flow",
        ],
        REPO_ROOT / ".github" / "copilot-instructions.md": ["append a pending breadcrumb first"],
        REPO_ROOT / "docs" / "PHASED_PLAN_CONTINUITY_PROTOCOL.md": ["Append a pending slice breadcrumb"],
    }

    for path, snippets in expected.items():
        text = path.read_text(encoding="utf-8")
        for snippet in snippets:
            assert snippet in text, f"{path} missing expected workflow-doc snippet: {snippet}"
        for snippet in unexpected.get(path, []):
            assert snippet not in text, f"{path} still contains stale workflow-doc snippet: {snippet}"


def test_workflow_cli_friction_plan_tracks_remaining_review_turns() -> None:
    plan_path = REPO_ROOT / "docs" / "notes" / "workflow_cli_friction_closure_PHASED_PLAN.md"
    text = plan_path.read_text(encoding="utf-8")

    assert "Phase 7 - dedicated hostile documentation review" in text
    assert "Phase 8 - final overall hostile workflow review" in text
    assert "## Current Phase\n\nComplete" not in text


def test_build_validation_profiles_uses_explicit_repo_root(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    tasks_dir = repo_root / ".vscode"
    tasks_dir.mkdir(parents=True)
    (tasks_dir / "tasks.json").write_text(
        """
{
    "tasks": [
        {
            "label": "verify: sample step",
            "type": "shell",
            "command": "py",
            "args": ["-3.14", "tool.py", "--log", "artifacts/sample.log", "--out-dir", "artifacts/sample_out"]
        },
        {
            "label": "verify: profile sample",
            "type": "shell",
            "command": "py",
            "dependsOn": ["verify: sample step"]
        }
    ]
}
""".strip(),
        encoding="utf-8",
    )

    profiles = build_validation_profiles(repo_root)

    assert profiles == {
        "sample": {
            "task_label": "verify: profile sample",
            "steps": [
                {
                    "label": "verify: sample step",
                    "outputs": ["artifacts/sample.log", "artifacts/sample_out"],
                }
            ],
        }
    }