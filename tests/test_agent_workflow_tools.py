from __future__ import annotations

import json
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_begin_work_slice import build_breadcrumb_append_plan, build_breadcrumb_message, main as begin_work_slice_main
from tools.viewer_host_checkpoint_slice import main as checkpoint_slice_main
from tools.viewer_host_contract_proof import build_validation_evidence_entries, validation_evidence_spec_for_command
from tools.viewer_host_session_bootstrap import BOOTSTRAP_AUDIT_PATH, _run_audit, build_bootstrap_state, build_validation_profiles, legacy_pending_handoff_entries, tail_handoff_entries
from tools.viewer_host_assert_phased_plan_sync import PLAN_SYNC_VALIDATION_ARTIFACT, main as assert_plan_sync_main, validate_plan_text
from tools.viewer_host_validate_slice_contract import main as validate_slice_contract_main


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
        "--plan",
        "docs/notes/hard_denial_workflow_lock_and_fits_reclosure_PHASED_PLAN.md",
        "--contract",
        "docs/contracts/hard_denial_workflow_lock_and_fits_reclosure.contract.json",
        "--dry-run",
    ]) == 0

    out = capsys.readouterr().out
    assert "session-start | branch=feature/test | head=abc123 | status=clean | profile=checkpoint | intent=workflow closeout" in out
    assert "viewer_host_begin_work_slice: checkpoint_id=ck:flow1234" in out
    assert "viewer_host_begin_work_slice: plan=docs/notes/hard_denial_workflow_lock_and_fits_reclosure_PHASED_PLAN.md" in out
    assert "viewer_host_begin_work_slice: contract=docs/contracts/hard_denial_workflow_lock_and_fits_reclosure.contract.json" in out
    assert "--commit ck:flow1234" in out
    assert "--commit pending" not in out


def test_begin_work_slice_requires_plan_and_contract(capsys) -> None:
    try:
        begin_work_slice_main([
            "--intent",
            "workflow closeout",
            "--profile",
            "checkpoint",
            "--dry-run",
        ])
        assert False, "expected begin_work_slice_main to reject missing plan/contract"
    except SystemExit as exc:
        assert exc.code == 2
    err = capsys.readouterr().err
    assert "the following arguments are required: --plan, --contract" in err


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


def test_validate_plan_text_requires_hostile_audit_sections_when_explicit_user_asks_exist() -> None:
    text = """
# Example

## Current Phase

Phase 1 - Runtime lock-down

## Phase Checklist

- [ ] Phase 1 - Runtime lock-down

## Explicit User Asks

- [open] Start implementation
"""

    message = validate_plan_text(text, display_path="docs/notes/example_PHASED_PLAN.md")

    assert message is not None
    assert "Hostile Audit" in message


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


def test_plan_sync_command_writes_validation_artifact() -> None:
    artifact_path = REPO_ROOT / PLAN_SYNC_VALIDATION_ARTIFACT
    if artifact_path.exists():
        artifact_path.unlink()

    assert assert_plan_sync_main(["docs/notes/ui_polish_sprint_phase0_foundation_PHASED_PLAN.md"]) == 0

    payload = json.loads(artifact_path.read_text(encoding="utf-8"))
    assert payload["ok"] is True
    assert payload["plan_count"] == 1
    assert payload["checked_plans"] == ["docs/notes/ui_polish_sprint_phase0_foundation_PHASED_PLAN.md"]


def test_validation_evidence_entries_cover_phase0_contract_validator_and_plan_sync() -> None:
    contract_command = (
        "py -3.14 tools/viewer_host_validate_slice_contract.py --contract "
        "docs/contracts/ui_polish_sprint_phase0_foundation.contract.json --out-json "
        "artifacts/validation/ui_polish_sprint_phase0_foundation_contract.json"
    )
    plan_sync_command = "py -3.14 tools/viewer_host_assert_phased_plan_sync.py"

    assert validate_slice_contract_main([
        "--contract",
        "docs/contracts/ui_polish_sprint_phase0_foundation.contract.json",
        "--out-json",
        "artifacts/validation/ui_polish_sprint_phase0_foundation_contract.json",
    ]) == 0
    assert assert_plan_sync_main(["docs/notes/ui_polish_sprint_phase0_foundation_PHASED_PLAN.md"]) == 0

    assert validation_evidence_spec_for_command(contract_command) is not None
    assert validation_evidence_spec_for_command(plan_sync_command) is not None

    entries = build_validation_evidence_entries([contract_command, plan_sync_command], REPO_ROOT)
    assert [entry["artifact_path"] for entry in entries] == [
        "artifacts/validation/ui_polish_sprint_phase0_foundation_contract.json",
        "artifacts/validation/viewer_host_assert_phased_plan_sync.json",
    ]


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
    assert state["next_commands"]["begin_work_slice"] == (
        'py -3.14 tools/viewer_host_begin_work_slice.py --intent "<slice>" --profile <native|runtime|catalog|checkpoint|unspecified> --plan <plan> --contract <contract>'
    )
    assert state["next_commands"]["prepare_slice"] == (
        'py -3.14 tools/viewer_host_prepare_slice.py --session-id <session_id> --plan <plan> --contract <contract>'
    )
    assert state["next_commands"]["crash_recovery"] == (
        'py -3.14 tools/viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state'
    )
    assert state["next_commands"]["append_handoff_legacy_pending"] == (
        'py -3.14 tools/viewer_host_append_handoff.py --resolve-last-pending --score <n> "<message>"'
    )


def test_bootstrap_audit_writes_to_non_authoritative_path(monkeypatch) -> None:
    class _Proc:
        returncode = 0
        stdout = "ok\n"

    captured: dict[str, object] = {}

    def fake_run(cmd, **kwargs):
        captured["cmd"] = list(cmd)
        return _Proc()

    monkeypatch.setattr("tools.viewer_host_session_bootstrap.subprocess.run", fake_run)

    audit = _run_audit("py")

    assert audit.command == ["py", "tools/code_quality_audit.py", "--check-baseline", "--out", BOOTSTRAP_AUDIT_PATH]
    assert captured["cmd"] == audit.command


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


def test_wrapped_build_tasks_do_not_use_cmd_c_batch_indirection() -> None:
    tasks = json.loads((REPO_ROOT / ".vscode" / "tasks.json").read_text(encoding="utf-8"))["tasks"]
    tasks_by_label = {task["label"]: task for task in tasks}

    native_command = tasks_by_label["verify: native helper tests"]["args"]
    native_command = native_command[native_command.index("--") + 1 :]
    runtime_command = tasks_by_label["verify: runtime publish"]["args"]
    runtime_command = runtime_command[runtime_command.index("--") + 1 :]

    assert native_command == ["ui_app\\build_tests_vsdevcmd.cmd"]
    assert runtime_command == ["ui_app\\build_vsdevcmd.cmd"]


def test_runtime_pytest_task_uses_runtime_lane_helper() -> None:
    tasks_json = (REPO_ROOT / ".vscode" / "tasks.json").read_text(encoding="utf-8")

    assert '"label": "verify: runtime probe/session pytest"' in tasks_json
    assert '"tools/viewer_host_runtime_pytest_lane.py"' in tasks_json
    assert '"tests/test_fractal_runtime_batch_cli.py"' in tasks_json
    assert '"tests/test_fractal_runtime_probe_cli.py"' in tasks_json
    assert '"tests/test_fractal_runtime_session.py"' in tasks_json
    assert '"tests/test_function_descriptor_cli.py"' in tasks_json
    assert '"tests/test_generic_probe_cli.py"' in tasks_json


def test_build_tests_script_exposes_advanced_color_grading_focus_modes() -> None:
    build_script = (REPO_ROOT / "ui_app" / "build_tests_vsdevcmd.cmd").read_text(encoding="utf-8")

    for snippet in (
        'if /I "%FOCUSED_TEST%"=="advanced_color_grading_red" goto focused_advanced_color_grading_red',
        'if /I "%FOCUSED_TEST%"=="advanced_color_grading_owner" goto focused_advanced_color_grading_owner',
        ":focused_advanced_color_grading_red",
        ":focused_advanced_color_grading_owner",
        'test_color_pipeline_core.exe',
        'test_color_pipeline_window.exe',
        'test_schema_binding.exe',
        'test_escape_time_coloring.exe',
        'test_fractal_family_rules.exe',
        'test_diagnostics_state_io.exe',
        'test_finding_archive_actions.exe',
        'test_runtime_reset.exe',
    ):
        assert snippet in build_script


def test_advanced_color_phase8c_plan_documents_fast_medium_and_closure_ladders() -> None:
    plan_text = (
        REPO_ROOT / "docs" / "notes" / "advanced_color_library_foundation_phase8c_test_lane_acceleration_PHASED_PLAN.md"
    ).read_text(encoding="utf-8")

    for snippet in (
        "test-only RED",
        "owner-seam implementation rerun",
        "seam/integration rerun",
        "final viewer-first closure proof",
        "advanced_color_grading_red",
        "advanced_color_grading_owner",
        "ui_app\\build_tests_vsdevcmd.cmd",
        "ui_app\\build_vsdevcmd.cmd",
        "tools/viewer_host_runtime_pytest_lane.py",
    ):
        assert snippet in plan_text


def test_runtime_ui_harness_task_targets_shared_runtime_scenarios() -> None:
    tasks_json = (REPO_ROOT / ".vscode" / "tasks.json").read_text(encoding="utf-8")

    assert '"label": "verify: runtime ui harness"' in tasks_json
    assert '"artifacts/verify_runtime_ui_harness.log"' in tasks_json
    assert '"tests/test_fractal_runtime_explaino_dual.py"' in tasks_json
    assert '"tests/test_fractal_runtime_explaino_escape_variants.py"' in tasks_json
    assert '"tests/test_fractal_runtime_explaino_sidecar_live.py"' in tasks_json
    assert '"tests/test_fractal_runtime_runtime_walk_viewer.py"' in tasks_json
    assert '"tests/test_fractal_runtime_shutdown.py"' in tasks_json
    assert '"tests/test_fractal_runtime_sweep_pause.py"' in tasks_json
    assert 'not test_runtime_walk_viewer_tolerates_missing_companion_fits' in tasks_json
    assert 'not test_runtime_walk_viewer_can_boot_from_fits_only_cli' in tasks_json


def test_runtime_walk_fits_witness_task_surfaces_non_mandatory_pair() -> None:
    tasks_json = (REPO_ROOT / ".vscode" / "tasks.json").read_text(encoding="utf-8")

    assert '"label": "verify: runtime walk FITS witnesses"' in tasks_json
    assert '"artifacts/verify_runtime_walk_fits_witnesses.log"' in tasks_json
    assert '"tests/test_fractal_runtime_runtime_walk_viewer.py"' in tasks_json
    assert 'test_runtime_walk_viewer_tolerates_missing_companion_fits or test_runtime_walk_viewer_can_boot_from_fits_only_cli' in tasks_json


def test_runtime_walk_fits_witness_task_stays_outside_mandatory_profiles() -> None:
    state = build_bootstrap_state(run_audit=False, tail_handoff=1)

    runtime_labels = [step["label"] for step in state["validation_profiles"]["runtime"]["steps"]]
    checkpoint_labels = [step["label"] for step in state["validation_profiles"]["checkpoint"]["steps"]]

    assert "verify: runtime walk FITS witnesses" not in runtime_labels
    assert "verify: runtime walk FITS witnesses" not in checkpoint_labels


def test_runtime_artifact_tools_task_surfaces_explicit_headless_bundle() -> None:
    tasks_json = (REPO_ROOT / ".vscode" / "tasks.json").read_text(encoding="utf-8")

    assert '"label": "verify: runtime artifact tools"' in tasks_json
    assert '"artifacts/verify_runtime_artifact_tools.log"' in tasks_json
    assert '"tests/test_explaino_runtime_walk_tool.py"' in tasks_json
    assert '"tests/test_flashlight_bridge_runner.py"' in tasks_json
    assert '"tests/test_fractal_runtime_flashlight_probe.py"' in tasks_json
    assert '"tests/test_fractal_runtime_flashlight_bridge.py"' in tasks_json
    assert '"tests/test_fractal_runtime_explaino_runtime_walk.py"' in tasks_json


def test_runtime_profile_includes_runtime_artifact_tools_task() -> None:
    state = build_bootstrap_state(run_audit=False, tail_handoff=1)

    runtime = state["validation_profiles"]["runtime"]

    assert [step["label"] for step in runtime["steps"]] == [
        "verify: code quality audit",
        "verify: runtime publish",
        "verify: runtime probe/session pytest",
        "verify: runtime artifact tools",
        "verify: runtime ui harness",
    ]


def test_checkpoint_profile_includes_runtime_artifact_tools_task() -> None:
    state = build_bootstrap_state(run_audit=False, tail_handoff=1)

    checkpoint = state["validation_profiles"]["checkpoint"]

    assert [step["label"] for step in checkpoint["steps"]] == [
        "verify: code quality audit",
        "verify: native helper tests",
        "verify: runtime publish",
        "verify: runtime probe/session pytest",
        "verify: runtime artifact tools",
        "verify: runtime ui harness",
    ]


def test_runtime_profile_includes_runtime_ui_harness_task() -> None:
    state = build_bootstrap_state(run_audit=False, tail_handoff=1)

    runtime = state["validation_profiles"]["runtime"]

    assert [step["label"] for step in runtime["steps"]] == [
        "verify: code quality audit",
        "verify: runtime publish",
        "verify: runtime probe/session pytest",
        "verify: runtime artifact tools",
        "verify: runtime ui harness",
    ]


def test_checkpoint_profile_includes_runtime_ui_harness_task() -> None:
    state = build_bootstrap_state(run_audit=False, tail_handoff=1)

    checkpoint = state["validation_profiles"]["checkpoint"]

    assert [step["label"] for step in checkpoint["steps"]] == [
        "verify: code quality audit",
        "verify: native helper tests",
        "verify: runtime publish",
        "verify: runtime probe/session pytest",
        "verify: runtime artifact tools",
        "verify: runtime ui harness",
    ]


def test_core_workflow_docs_advertise_session_start_checkpoint_flow() -> None:
    expected = {
        REPO_ROOT / "AGENTS.md": [
            "append a session-start breadcrumb first",
            'py -3.14 tools\\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"',
            'py -3.14 tools\\viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state',
        ],
        REPO_ROOT / "AGENT_WORKING_PROTOCOL.md": [
            "session-start breadcrumb",
            'py -3.14 tools\\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"',
            'py -3.14 tools\\viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state',
        ],
        REPO_ROOT / ".github" / "copilot-instructions.md": [
            "append a session-start breadcrumb first",
            'py -3.14 tools\\viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state',
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


def test_phase0_workflow_docs_require_mainline_style_plan_sections() -> None:
    expected = {
        REPO_ROOT / "AGENTS.md": [
            "Explicit User Asks",
            "Proof Ledger",
            "Hostile Audit",
        ],
        REPO_ROOT / "AGENT_WORKING_PROTOCOL.md": [
            "Explicit User Asks",
            "Proof Ledger",
            "Hostile Audit",
        ],
        REPO_ROOT / ".github" / "copilot-instructions.md": [
            "Explicit User Asks",
            "Proof Ledger",
            "Hostile Audit",
        ],
        REPO_ROOT / "docs" / "PHASED_PLAN_CONTINUITY_PROTOCOL.md": [
            "## Explicit User Asks",
            "## Proof Ledger",
            "## Hostile Audit",
            "## Audit Passes",
            "## Audit Findings",
        ],
    }

    for path, snippets in expected.items():
        text = path.read_text(encoding="utf-8")
        for snippet in snippets:
            assert snippet in text, f"{path} missing expected phased-plan section guidance: {snippet}"


def test_ui_polish_phase0_plan_resume_point_waits_for_checkpoint() -> None:
    plan_path = REPO_ROOT / "docs" / "notes" / "ui_polish_sprint_phase0_foundation_PHASED_PLAN.md"
    text = plan_path.read_text(encoding="utf-8")

    assert "after this slice is checkpointed" in text.lower()
    assert "current clean `feature/explaino-joy` state" not in text


def test_workflow_cli_friction_plan_tracks_remaining_review_turns() -> None:
    plan_path = REPO_ROOT / "docs" / "notes" / "workflow_cli_friction_closure_PHASED_PLAN.md"
    text = plan_path.read_text(encoding="utf-8")

    assert "Phase 7 - dedicated hostile documentation review" in text
    assert "Phase 8 - final overall hostile workflow review" in text
    assert "## Current Phase\n\nComplete" in text
    assert "Phase 8 completion snapshot:" in text
    assert "this initiative is now complete" in text.lower()


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


def test_tasks_surface_exposes_plan_and_contract_inputs_for_begin_work_slice() -> None:
    tasks_json = (REPO_ROOT / ".vscode" / "tasks.json").read_text(encoding="utf-8")

    assert '"id": "agentSlicePlan"' in tasks_json
    assert '"id": "agentSliceContract"' in tasks_json
    assert '"--plan"' in tasks_json
    assert '"--contract"' in tasks_json


def test_checkpoint_slice_commit_refuses_pending_hostile_audit(monkeypatch, tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    plan_path = repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md"
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text(
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
        "- [open] Pass 1 - hostile review still pending\n",
        encoding="utf-8",
    )

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.discover_repo_root", lambda _path: repo_root)
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.validate_locked_contract_state",
        lambda _session_id, _repo_root: ({"plan_path": "docs/notes/plan_PHASED_PLAN.md"}, ""),
    )
    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.build_handoff_append_commands", lambda **_kwargs: [])
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.file_path_is_in_contract_scope",
        lambda path, _contract_state, _repo_root: path in {
            "tools/viewer_host_recover_crash_state.py",
            "docs/notes/checkpoint_guard_crash_recovery_playbook.md",
            "HANDOFF_LOG.md",
        },
    )

    commands: list[list[str]] = []

    class _Proc:
        returncode = 0

    def fake_run(command: list[str], cwd: str, check: bool = False):
        commands.append(command)
        return _Proc()

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.subprocess.run", fake_run)

    rc = checkpoint_slice_main([
        "commit",
        "--session-id",
        "session-1",
        "--cwd",
        str(repo_root),
        "--checkpoint-id",
        "ck:test1234",
        "--score",
        "95",
        "--handoff-message",
        "test handoff",
        "--commit-message",
        "ck:test1234 test",
    ])

    assert rc != 0
    assert not any(command[:2] == ["git", "commit"] for command in commands)


def test_checkpoint_slice_commit_scopes_git_add_and_commit_to_selected_paths(monkeypatch, tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    plan_path = repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md"
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text(
        "# Plan\n\n"
        "## Current Phase\n\n"
        "Phase 1 in progress\n\n"
        "## Phase Checklist\n\n"
        "- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.discover_repo_root", lambda _path: repo_root)
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.validate_locked_contract_state",
        lambda _session_id, _repo_root: ({"plan_path": "docs/notes/plan_PHASED_PLAN.md"}, ""),
    )
    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.build_handoff_append_commands", lambda **_kwargs: [])
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.file_path_is_in_contract_scope",
        lambda path, _contract_state, _repo_root: path in {
            "tools/viewer_host_recover_crash_state.py",
            "docs/notes/checkpoint_guard_crash_recovery_playbook.md",
            "HANDOFF_LOG.md",
        },
    )

    commands: list[list[str]] = []

    class _Proc:
        returncode = 0

    def fake_run(command: list[str], cwd: str, check: bool = False):
        commands.append(command)
        return _Proc()

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.subprocess.run", fake_run)

    rc = checkpoint_slice_main([
        "commit",
        "--session-id",
        "session-1",
        "--cwd",
        str(repo_root),
        "--checkpoint-id",
        "ck:test1234",
        "--score",
        "95",
        "--handoff-message",
        "test handoff",
        "--commit-message",
        "ck:test1234 test",
        "--path",
        "tools/viewer_host_recover_crash_state.py",
        "--path",
        "docs/notes/checkpoint_guard_crash_recovery_playbook.md",
    ])

    assert rc == 0
    assert [command for command in commands if command[:2] == ["git", "add"]] == [
        [
            "git",
            "add",
            "--",
            "tools/viewer_host_recover_crash_state.py",
            "docs/notes/checkpoint_guard_crash_recovery_playbook.md",
            "HANDOFF_LOG.md",
        ]
    ]
    assert [command for command in commands if command[:2] == ["git", "commit"]] == [
        [
            "git",
            "commit",
            "-m",
            "ck:test1234 test",
            "--",
            "tools/viewer_host_recover_crash_state.py",
            "docs/notes/checkpoint_guard_crash_recovery_playbook.md",
            "HANDOFF_LOG.md",
        ]
    ]
    assert not any(command == ["git", "add", "-A"] for command in commands)


def test_checkpoint_slice_commit_auto_includes_handoff_log_in_scoped_paths(monkeypatch, tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    plan_path = repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md"
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text(
        "# Plan\n\n"
        "## Current Phase\n\n"
        "Phase 1 in progress\n\n"
        "## Phase Checklist\n\n"
        "- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.discover_repo_root", lambda _path: repo_root)
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.validate_locked_contract_state",
        lambda _session_id, _repo_root: ({"plan_path": "docs/notes/plan_PHASED_PLAN.md"}, ""),
    )
    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.build_handoff_append_commands", lambda **_kwargs: [])
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.file_path_is_in_contract_scope",
        lambda path, _contract_state, _repo_root: path in {
            "tools/viewer_host_recover_crash_state.py",
            "HANDOFF_LOG.md",
        },
    )

    commands: list[list[str]] = []

    class _Proc:
        returncode = 0

    def fake_run(command: list[str], cwd: str, check: bool = False):
        commands.append(command)
        return _Proc()

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.subprocess.run", fake_run)

    rc = checkpoint_slice_main([
        "commit",
        "--session-id",
        "session-1",
        "--cwd",
        str(repo_root),
        "--checkpoint-id",
        "ck:test1234",
        "--score",
        "95",
        "--handoff-message",
        "test handoff",
        "--commit-message",
        "ck:test1234 test",
        "--path",
        "tools/viewer_host_recover_crash_state.py",
    ])

    assert rc == 0
    assert [command for command in commands if command[:2] == ["git", "add"]] == [
        [
            "git",
            "add",
            "--",
            "tools/viewer_host_recover_crash_state.py",
            "HANDOFF_LOG.md",
        ]
    ]
    assert [command for command in commands if command[:2] == ["git", "commit"]] == [
        [
            "git",
            "commit",
            "-m",
            "ck:test1234 test",
            "--",
            "tools/viewer_host_recover_crash_state.py",
            "HANDOFF_LOG.md",
        ]
    ]


def test_checkpoint_slice_commit_without_paths_stages_all_changes(monkeypatch, tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    plan_path = repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md"
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text(
        "# Plan\n\n"
        "## Current Phase\n\n"
        "Phase 1 in progress\n\n"
        "## Phase Checklist\n\n"
        "- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.discover_repo_root", lambda _path: repo_root)
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.validate_locked_contract_state",
        lambda _session_id, _repo_root: ({"plan_path": "docs/notes/plan_PHASED_PLAN.md"}, ""),
    )
    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.build_handoff_append_commands", lambda **_kwargs: [])

    commands: list[list[str]] = []

    class _Proc:
        returncode = 0

    def fake_run(command: list[str], cwd: str, check: bool = False):
        commands.append(command)
        return _Proc()

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.subprocess.run", fake_run)

    rc = checkpoint_slice_main([
        "commit",
        "--session-id",
        "session-1",
        "--cwd",
        str(repo_root),
        "--checkpoint-id",
        "ck:test1234",
        "--score",
        "95",
        "--handoff-message",
        "test handoff",
        "--commit-message",
        "ck:test1234 test",
    ])

    assert rc == 0
    assert [command for command in commands if command[:2] == ["git", "add"]] == [["git", "add", "-A"]]
    assert [command for command in commands if command[:2] == ["git", "commit"]] == [["git", "commit", "-m", "ck:test1234 test"]]


def test_checkpoint_slice_commit_refuses_scoped_path_outside_contract(monkeypatch, tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    plan_path = repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md"
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text(
        "# Plan\n\n"
        "## Current Phase\n\n"
        "Phase 1 in progress\n\n"
        "## Phase Checklist\n\n"
        "- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.discover_repo_root", lambda _path: repo_root)
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.validate_locked_contract_state",
        lambda _session_id, _repo_root: ({"plan_path": "docs/notes/plan_PHASED_PLAN.md"}, ""),
    )
    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.build_handoff_append_commands", lambda **_kwargs: [])
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.file_path_is_in_contract_scope",
        lambda path, _contract_state, _repo_root: path == "HANDOFF_LOG.md",
    )

    commands: list[list[str]] = []

    class _Proc:
        returncode = 0

    def fake_run(command: list[str], cwd: str, check: bool = False):
        commands.append(command)
        return _Proc()

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.subprocess.run", fake_run)

    rc = checkpoint_slice_main([
        "commit",
        "--session-id",
        "session-1",
        "--cwd",
        str(repo_root),
        "--checkpoint-id",
        "ck:test1234",
        "--score",
        "95",
        "--handoff-message",
        "test handoff",
        "--commit-message",
        "ck:test1234 test",
        "--path",
        "tools/viewer_host_recover_crash_state.py",
    ])

    assert rc != 0
    assert not commands


def test_checkpoint_slice_write_receipts_refuses_pending_hostile_audit(monkeypatch, tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    plan_path = repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md"
    plan_path.parent.mkdir(parents=True, exist_ok=True)
    plan_path.write_text(
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
        "- [open] Pass 1 - hostile review still pending\n",
        encoding="utf-8",
    )

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.discover_repo_root", lambda _path: repo_root)
    monkeypatch.setattr(
        "tools.viewer_host_checkpoint_slice.validate_locked_contract_state",
        lambda _session_id, _repo_root: ({"plan_path": "docs/notes/plan_PHASED_PLAN.md"}, ""),
    )

    commands: list[list[str]] = []

    class _Proc:
        returncode = 0

    def fake_run(command: list[str], cwd: str, check: bool = False):
        commands.append(command)
        return _Proc()

    monkeypatch.setattr("tools.viewer_host_checkpoint_slice.subprocess.run", fake_run)

    rc = checkpoint_slice_main([
        "write-receipts",
        "--session-id",
        "session-1",
        "--cwd",
        str(repo_root),
        "--validation-summary",
        "tests passed",
        "--validation-command",
        "py -3.14 -m pytest tests/test_agent_workflow_tools.py -q",
    ])

    assert rc != 0
    assert commands == []
