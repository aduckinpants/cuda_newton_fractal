from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import tools.viewer_host_apply_repo_patch as apply_repo_patch
from tools.viewer_host_checkpoint_slice import _build_validation_command_hints
from tools.viewer_host_contract_state import validate_slice_contract_payload


REPO_ROOT = Path(__file__).resolve().parents[1]
LOGGED_COMMAND_TOOL = REPO_ROOT / "tools" / "viewer_host_run_logged_command.py"


def _valid_contract_payload(command: str) -> dict[str, object]:
    return {
        "contract_id": "example_contract",
        "feature_id": "example_feature",
        "workflow_type": "workflow_only",
        "plan_path": "docs/notes/viewer_host_tooling_hardening_PHASED_PLAN.md",
        "allowed_mutation_scope": ["tools"],
        "required_operator_inputs": ["harden tooling"],
        "forbidden_operator_prompts": ["change product behavior"],
        "required_defaults": {"scope": "tooling"},
        "forbidden_defaults": {"runtime_publish": "not_required"},
        "required_validation_commands": [command],
        "required_acceptance_assertions": [
            {
                "assertion_id": "contract_schema_valid",
                "description": "Contract schema validates",
                "evidence_kind": "validator_json",
                "artifact_path": "artifacts/validation/example.json",
                "json_path": "ok",
                "equals": True,
            }
        ],
    }


def test_apply_repo_patch_extracts_quoted_windows_b_paths() -> None:
    patch_text = """diff --git \"a/ui_app\\\\src\\\\diagnostics_capture.cpp\" \"b/ui_app\\\\src\\\\diagnostics_capture.cpp\"
--- \"a/ui_app\\\\src\\\\diagnostics_capture.cpp\"
+++ \"b/ui_app\\\\src\\\\diagnostics_capture.cpp\"
@@ -1 +1 @@
-old
+new
"""

    assert apply_repo_patch._extract_patch_targets(patch_text) == ["ui_app/src/diagnostics_capture.cpp"]


def test_apply_repo_patch_normalizes_patch_text_to_lf_for_git_apply() -> None:
    patch_text = "line one\r\nline two\rline three\n"

    assert apply_repo_patch._patch_text_for_git_apply(patch_text) == "line one\nline two\nline three\n"


def test_contract_validation_rejects_control_characters_in_required_commands() -> None:
    bad_command = "ui_app" + "\b" + "uild_tests_vsdevcmd.cmd"
    payload = _valid_contract_payload(bad_command)

    result = validate_slice_contract_payload(payload, REPO_ROOT)

    assert result.ok is False
    assert any("control character" in error and "required_validation_commands[0]" in error for error in result.errors)


def test_logged_command_reports_timeout_configuration(tmp_path: Path) -> None:
    log_path = tmp_path / "logged.log"
    json_path = tmp_path / "logged.json"
    result = subprocess.run(
        [
            sys.executable,
            str(LOGGED_COMMAND_TOOL),
            "--label",
            "timeout-config-smoke",
            "--log",
            str(log_path),
            "--out-json",
            str(json_path),
            "--timeout-seconds",
            "5",
            "--",
            sys.executable,
            "-c",
            "print('ok')",
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        check=False,
    )

    assert result.returncode == 0
    assert "viewer_host_run_logged_command: timeout_seconds=5.000" in result.stdout
    payload = json.loads(json_path.read_text(encoding="utf-8"))
    assert payload["timeout_seconds"] == 5.0


def test_receipt_preflight_hints_near_matching_validation_commands() -> None:
    hints = _build_validation_command_hints(
        ["py -3.14 ui_app/build_vsdevcmd.cmd"],
        ["py -3.14 ui_app\\build_vsdevcmd.cmd"],
    )

    assert len(hints) == 1
    assert "slash/control-character normalization" in hints[0]
    assert "required=py -3.14 ui_app/build_vsdevcmd.cmd" in hints[0]
    assert "provided=py -3.14 ui_app\\build_vsdevcmd.cmd" in hints[0]
