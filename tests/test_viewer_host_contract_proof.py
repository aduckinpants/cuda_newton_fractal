from __future__ import annotations

import json
import hashlib
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_write_contract_proof_receipt import main as write_contract_proof_main
from tools.viewer_host_contract_proof import (
    build_validation_evidence_entries,
    evaluate_assertion,
    load_artifact_evidence,
    validation_evidence_spec_for_command,
)


def test_load_artifact_evidence_reads_junit_case_results(tmp_path: Path) -> None:
    artifact = tmp_path / "sample.junit.xml"
    artifact.write_text(
        """
<testsuite tests="1" failures="0">
  <testcase classname="tests.test_viewer_host_checkpoint_guard" name="test_build_pretool_response_denies_raw_apply_patch_even_when_repo_is_clean" time="0.01" />
</testsuite>
""".strip(),
        encoding="utf-8",
    )

    payload = load_artifact_evidence("pytest_junit_case", artifact)

    node_id = "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_denies_raw_apply_patch_even_when_repo_is_clean"
    assert payload[node_id]["ok"] is True


def test_evaluate_assertion_matches_validator_json_exact_path(tmp_path: Path) -> None:
    artifact = tmp_path / "validator.json"
    artifact.write_text(
        json.dumps(
            {
                "ok": True,
                "checks": {
                    "default_mapping_has_no_warp_binding": True,
                },
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    result = evaluate_assertion(
        {
            "assertion_id": "default_mapping_has_no_warp_binding",
            "description": "default mapping has no warp binding",
            "evidence_kind": "validator_json",
            "artifact_path": str(artifact),
            "json_path": "checks.default_mapping_has_no_warp_binding",
            "equals": True,
        },
        tmp_path,
    )

    assert result["ok"] is True
    assert result["matched_key"] == "checks.default_mapping_has_no_warp_binding"


def test_evaluate_assertion_rejects_validation_artifact_hash_drift(tmp_path: Path) -> None:
    artifact = tmp_path / "validator.json"
    artifact.write_text(
        json.dumps(
            {
                "ok": True,
                "checks": {
                    "default_mapping_has_no_warp_binding": True,
                },
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    result = evaluate_assertion(
        {
            "assertion_id": "default_mapping_has_no_warp_binding",
            "description": "default mapping has no warp binding",
            "evidence_kind": "validator_json",
            "artifact_path": str(artifact.relative_to(tmp_path)),
            "json_path": "checks.default_mapping_has_no_warp_binding",
            "equals": True,
        },
        tmp_path,
        {
            "evidence": [
                {
                    "evidence_id": "validator_fits_contract",
                    "artifact_kind": "validator_json",
                    "artifact_path": str(artifact.relative_to(tmp_path)).replace("\\", "/"),
                    "artifact_sha256": "stale",
                }
            ]
        },
    )

    assert result["ok"] is False
    assert "hash mismatch" in result["failure_detail"]


def test_evaluate_assertion_rejects_validation_artifact_metadata_drift(tmp_path: Path) -> None:
    artifact = tmp_path / "validator.json"
    artifact.write_text(
        json.dumps({"ok": True}, indent=2),
        encoding="utf-8",
    )
    artifact_path = str(artifact.relative_to(tmp_path)).replace("\\", "/")
    artifact_hash = hashlib.sha256(artifact.read_bytes()).hexdigest()

    result = evaluate_assertion(
        {
            "assertion_id": "validator_ok",
            "description": "validator ok",
            "evidence_kind": "validator_json",
            "artifact_path": artifact_path,
            "json_path": "ok",
            "equals": True,
        },
        tmp_path,
        {
            "evidence": [
                {
                    "evidence_id": "validator_json_artifacts_validation_contract_json",
                    "artifact_kind": "validator_json",
                    "artifact_path": artifact_path,
                    "artifact_size_bytes": artifact.stat().st_size + 1,
                    "artifact_sha256": artifact_hash,
                }
            ]
        },
    )

    assert result["ok"] is False
    assert "size mismatch" in result["failure_detail"]


def test_validation_evidence_spec_for_command_recognizes_hostile_audit_validator() -> None:
    command = (
        "py -3.14 tools/viewer_host_validate_hostile_audit.py --plan "
        "docs/notes/workflow_guard_hostile_review_enforcement_PHASED_PLAN.md --out-json "
        "artifacts/validation/workflow_guard_hostile_review_enforcement_hostile_audit.json"
    )

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "validator_json"
    assert spec.artifact_path == "artifacts/validation/workflow_guard_hostile_review_enforcement_hostile_audit.json"


def test_validation_evidence_spec_for_command_recognizes_dynamic_pytest_junit() -> None:
    command = (
        "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py "
        "tests/test_viewer_host_contract_proof.py -q --junitxml artifacts/pytest/anti_lie_workflow_tools.junit.xml"
    )

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "junit_xml"
    assert spec.artifact_path == "artifacts/pytest/anti_lie_workflow_tools.junit.xml"


def test_validation_evidence_spec_for_command_recognizes_truth_report_validator() -> None:
    command = "py -3.14 tools/viewer_host_truth_report.py --out-json artifacts/validation/viewer_host_truth_report.json"

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "validator_json"
    assert spec.artifact_path == "artifacts/validation/viewer_host_truth_report.json"


def test_validation_evidence_spec_for_command_recognizes_forensic_timeline_validator() -> None:
    command = "py -3.14 tools/viewer_host_forensic_timeline.py --out-json artifacts/validation/anti_lie_forensic_timeline.json"

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "validator_json"
    assert spec.artifact_path == "artifacts/validation/anti_lie_forensic_timeline.json"


def test_validation_evidence_spec_for_command_recognizes_logged_command_pytest() -> None:
    command = (
        "py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py -q --junitxml "
        "artifacts/pytest/test_viewer_host_run_logged_command.junit.xml"
    )

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "junit_xml"
    assert spec.artifact_path == "artifacts/pytest/test_viewer_host_run_logged_command.junit.xml"


def test_validation_evidence_spec_for_command_recognizes_test_coverage_audit() -> None:
    command = "py -3.14 tools/test_coverage_audit.py --check-baseline --matrix --out artifacts/test_coverage_report.json"

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "validator_json"
    assert spec.artifact_path == "artifacts/test_coverage_report.json"


def test_validation_evidence_spec_for_command_recognizes_code_quality_audit() -> None:
    command = "py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json"

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "validator_json"
    assert spec.artifact_path == "artifacts/code_quality_report.json"


def test_validation_evidence_spec_for_command_recognizes_salt_ndepend_freeze_gate() -> None:
    command = "py -3.14 tools/viewer_host_salt_ndepend.py freeze-gate --out-dir artifacts/salt_ndepend/latest"

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "validator_json"
    assert spec.artifact_path == "artifacts/salt_ndepend/latest/doctor.json"


def test_validation_evidence_spec_for_command_recognizes_redirected_log_command() -> None:
    command = "cmd /c ui_app\\build_tests_vsdevcmd.cmd > artifacts\\logs\\phase4d_full_native.log 2>&1"

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "text_log"
    assert spec.artifact_path == "artifacts/logs/phase4d_full_native.log"


def test_validation_evidence_spec_for_command_prefers_logged_command_out_json() -> None:
    command = (
        "py -3.14 tools/viewer_host_run_logged_command.py --label sprint_runtime_publish "
        "--log artifacts/logs/sprint_runtime_publish.log "
        "--out-json artifacts/validation/sprint_runtime_publish.json "
        "--heartbeat-seconds 30 --timeout-seconds 1200 -- cmd /c ui_app\\build_vsdevcmd.cmd"
    )

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "validator_json"
    assert spec.artifact_path == "artifacts/validation/sprint_runtime_publish.json"



def test_validation_evidence_spec_for_command_recognizes_logged_command_wrapper() -> None:
    command = (
        "py -3.14 tools/viewer_host_run_logged_command.py --label phase8_full_native_dead_control_repair_rerun "
        "--log artifacts/logs/phase8_full_native_dead_control_repair_rerun.log -- ui_app/build_tests_vsdevcmd.cmd"
    )

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "text_log"
    assert spec.artifact_path == "artifacts/logs/phase8_full_native_dead_control_repair_rerun.log"


def test_fractal_parameter_surface_contract_does_not_require_relaunching_runtime_walk_viewer_module() -> None:
    contract = json.loads((REPO_ROOT / "docs" / "contracts" / "fractal_parameter_surface_matrix.contract.json").read_text(encoding="utf-8"))

    runtime_commands = [
        command for command in contract["required_validation_commands"]
        if "tests/test_fractal_runtime_runtime_walk_viewer.py" in command
    ]

    assert runtime_commands == []

    runtime_assertions = [
        assertion for assertion in contract["required_acceptance_assertions"]
        if assertion.get("evidence_kind") == "runtime_junit_case"
        and str(assertion.get("test_nodeid", "")).startswith("tests/test_fractal_runtime_runtime_walk_viewer.py::")
    ]

    assert runtime_assertions == []
    persistent_commands = [
        command for command in contract["required_validation_commands"]
        if "tests/test_fractal_runtime_persistent_viewer_harness.py" in command
    ]
    assert len(persistent_commands) == 1

    persistent_assertions = [
        assertion for assertion in contract["required_acceptance_assertions"]
        if assertion.get("test_nodeid")
        == "tests/test_fractal_runtime_persistent_viewer_harness.py::test_mcmullen_direct_controls_change_live_frame_in_one_process"
    ]
    assert len(persistent_assertions) == 1
    assert contract["workflow_type"] == "viewer_first"
    assert contract["required_defaults"].get("runtime_walk_viewer_default_proof") == "forbidden_legacy_relaunch_module"
    assert contract["required_defaults"].get("persistent_runtime_viewer_harness") == "required"



def test_fractal_parameter_surface_contract_requires_all44_scope() -> None:
    contract = json.loads((REPO_ROOT / "docs" / "contracts" / "fractal_parameter_surface_matrix.contract.json").read_text(encoding="utf-8"))
    plan_text = (REPO_ROOT / "docs" / "notes" / "fractal_parameter_surface_matrix_PHASED_PLAN.md").read_text(encoding="utf-8")
    inventory_text = (REPO_ROOT / "docs" / "notes" / "fractal_control_surface_audit_inventory.md").read_text(encoding="utf-8")
    schema = json.loads((REPO_ROOT / "ui" / "fractal_binding_surface_v1.ui_schema.json").read_text(encoding="utf-8"))

    options = []
    for panel in schema["panels"]:
        for control in panel.get("controls", []):
            if control.get("id") == "fractal_type":
                options = [option["id"] for option in control["options"]]
                break
        if options:
            break

    enum_ids_text = (REPO_ROOT / "ui_app" / "src" / "enum_id_utils.h").read_text(encoding="utf-8")
    enum_id_rows = [line for line in enum_ids_text.splitlines() if "{FractalType::" in line and line.count('"') >= 2]

    expected_count = contract["required_defaults"].get("all_fractal_count")

    assert len(options) == expected_count
    assert len(enum_id_rows) == expected_count
    assert contract["feature_id"] == "fractal_parameter_surface_matrix_phase11_parameter_api_hardening"
    assert expected_count >= 44
    assert contract["required_defaults"].get("selected_subset_closeout_is_insufficient") is True
    assert contract["forbidden_defaults"].get("subset_matrix_closeout_as_all_current_completion") == "forbidden"
    assert contract["forbidden_defaults"].get("missing_control_omission") == "forbidden"
    assert contract["forbidden_defaults"].get("dead_slider_omission") == "forbidden"
    assert any("all-fractal" in item for item in contract["required_operator_inputs"])
    assert any("selected subset" in item for item in contract["forbidden_operator_prompts"])
    assert any("legacy relaunching runtime-walk viewer module" in item for item in contract["forbidden_operator_prompts"])
    assert "Phase 11 is closed" in plan_text
    assert "current schema/enum count" in plan_text
    assert "No remaining parameter-surface cleanup phase is open in this plan" not in plan_text
    assert f"FractalType` enum count: {expected_count}" in inventory_text
    assert f"fractal_type` schema option count: {expected_count}" in inventory_text


def test_fractal_parameter_surface_contract_requires_phase11_parameter_api_hardening() -> None:
    contract = json.loads((REPO_ROOT / "docs" / "contracts" / "fractal_parameter_surface_matrix.contract.json").read_text(encoding="utf-8"))
    plan_text = (REPO_ROOT / "docs" / "notes" / "fractal_parameter_surface_matrix_PHASED_PLAN.md").read_text(encoding="utf-8")

    assert contract["feature_id"] == "fractal_parameter_surface_matrix_phase11_parameter_api_hardening"
    assert contract["required_defaults"].get("descriptor_export") == "required_all_current_schema_cpp_derived"
    assert contract["required_defaults"].get("mcmullen_direct_controls") == "required"
    assert contract["required_defaults"].get("phase9_10_descriptor_and_mcmullen_baseline") == "preserved"
    assert contract["required_defaults"].get("parameter_api_hardening") == "required"
    assert contract["required_defaults"].get("mcmullen_state_authority_consistency") == "required"
    assert contract["required_defaults"].get("descriptor_artifact_readback") == "required"
    assert contract["required_defaults"].get("defer_remaining_future_slices") is True
    assert contract["forbidden_defaults"].get("collatz_controls") == "deferred"
    assert contract["forbidden_defaults"].get("perturbation_zoom") == "deferred"
    assert contract["forbidden_defaults"].get("state_display_runtime_authority_mismatch") == "forbidden"
    assert any("phase11_parameter_api_native" in command for command in contract["required_validation_commands"])
    assert any("fractal_runtime_persistent_viewer_harness_phase11_mcmullen" in command for command in contract["required_validation_commands"])
    assert "Phase 11 is closed" in plan_text
    assert "Collatz controls" in plan_text and "deferred" in plan_text


def test_fractal_parameter_surface_contract_requires_code_quality_guard() -> None:
    contract = json.loads((REPO_ROOT / "docs" / "contracts" / "fractal_parameter_surface_matrix.contract.json").read_text(encoding="utf-8"))

    quality_commands = [
        command for command in contract["required_validation_commands"]
        if "tools/code_quality_audit.py" in command and "--check-baseline" in command
    ]

    assert len(quality_commands) == 1




def test_runtime_walk_viewer_e2e_module_is_explicitly_opt_in() -> None:
    module_text = (REPO_ROOT / "tests" / "test_fractal_runtime_runtime_walk_viewer.py").read_text(encoding="utf-8")

    assert "VIEWER_HOST_ENABLE_RUNTIME_VIEWER_E2E" in module_text
    assert "pytestmark = pytest.mark.skipif" in module_text
    assert "subprocess.Popen" in module_text


def test_build_validation_evidence_entries_records_artifact_metadata(tmp_path: Path) -> None:
    artifact = tmp_path / "artifacts" / "validation" / "contract.json"
    artifact.parent.mkdir(parents=True)
    artifact.write_text(json.dumps({"ok": True}, indent=2), encoding="utf-8")
    command = (
        "py -3.14 tools/viewer_host_validate_slice_contract.py --contract "
        "docs/contracts/slice.contract.json --out-json artifacts/validation/contract.json"
    )

    entries = build_validation_evidence_entries([command], tmp_path)

    assert len(entries) == 1
    assert entries[0]["artifact_size_bytes"] == artifact.stat().st_size
    assert entries[0]["artifact_mtime_utc"].endswith("+00:00")
    assert entries[0]["artifact_sha256"] == hashlib.sha256(artifact.read_bytes()).hexdigest()


def test_contract_proof_writer_fails_when_required_assertion_evidence_is_missing(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    (repo_root / ".gitignore").write_text("artifacts/\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md", ".gitignore"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)

    (repo_root / "tools").mkdir()
    (repo_root / "docs" / "notes").mkdir(parents=True)
    (repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md").write_text(
        "# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )
    (repo_root / "docs" / "contracts").mkdir(parents=True)
    (repo_root / "docs" / "contracts" / "slice.contract.json").write_text(
        json.dumps(
            {
                "version": 1,
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["fits"],
                "forbidden_operator_prompts": ["state.json"],
                "required_defaults": {"base_fractal_type": "explaino"},
                "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
                "required_validation_commands": [
                    "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml"
                ],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "strict_banner_emitted",
                        "description": "strict banner emitted",
                        "evidence_kind": "pytest_junit_case",
                        "artifact_path": "artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                        "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner",
                    }
                ],
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    subprocess.run(
        ["git", "add", "tools", "docs/notes/plan_PHASED_PLAN.md", "docs/contracts/slice.contract.json"],
        cwd=repo_root,
        check=True,
        capture_output=True,
        text=True,
    )
    subprocess.run(["git", "commit", "-m", "seed slice"], cwd=repo_root, check=True, capture_output=True, text=True)

    subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_prepare_slice.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
            "--plan",
            "docs/notes/plan_PHASED_PLAN.md",
            "--contract",
            "docs/contracts/slice.contract.json",
        ],
        cwd=str(repo_root),
        check=True,
        capture_output=True,
        text=True,
    )

    receipts = repo_root / "artifacts" / "hooks" / "viewer_host_validation_receipts"
    receipts.mkdir(parents=True, exist_ok=True)
    head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=repo_root, check=True, capture_output=True, text=True).stdout.strip()
    (receipts / f"{head}.json").write_text(
        json.dumps(
                {
                    "head": head,
                    "summary": "validated",
                    "commands": [
                        "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml"
                    ],
                    "evidence": [
                        {
                            "evidence_id": "pytest_checkpoint_guard",
                            "command": "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                            "artifact_kind": "junit_xml",
                            "artifact_path": "artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                            "artifact_sha256": "missing",
                        }
                    ],
                    "clean": True,
                },
                indent=2,
            ),
            encoding="utf-8",
        )

    proc = subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_write_contract_proof_receipt.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
        ],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )

    assert proc.returncode != 0
    assert "assertion" in (proc.stderr or proc.stdout).lower()


def test_contract_proof_writer_requires_exact_required_validation_command_match(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    (repo_root / ".gitignore").write_text("artifacts/\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md", ".gitignore"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)

    (repo_root / "tools").mkdir()
    (repo_root / "docs" / "notes").mkdir(parents=True)
    (repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md").write_text(
        "# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )
    (repo_root / "docs" / "contracts").mkdir(parents=True)
    (repo_root / "docs" / "contracts" / "slice.contract.json").write_text(
        json.dumps(
            {
                "version": 1,
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["fits"],
                "forbidden_operator_prompts": ["state.json"],
                "required_defaults": {"base_fractal_type": "explaino"},
                "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
                "required_validation_commands": [
                    "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml"
                ],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "strict_banner_emitted",
                        "description": "strict banner emitted",
                        "evidence_kind": "pytest_junit_case",
                        "artifact_path": "artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                        "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner",
                    }
                ],
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    subprocess.run(
        ["git", "add", "tools", "docs/notes/plan_PHASED_PLAN.md", "docs/contracts/slice.contract.json"],
        cwd=repo_root,
        check=True,
        capture_output=True,
        text=True,
    )
    subprocess.run(["git", "commit", "-m", "seed slice"], cwd=repo_root, check=True, capture_output=True, text=True)

    subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_prepare_slice.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
            "--plan",
            "docs/notes/plan_PHASED_PLAN.md",
            "--contract",
            "docs/contracts/slice.contract.json",
        ],
        cwd=str(repo_root),
        check=True,
        capture_output=True,
        text=True,
    )

    artifact = repo_root / "artifacts" / "pytest" / "test_viewer_host_checkpoint_guard.junit.xml"
    artifact.parent.mkdir(parents=True, exist_ok=True)
    artifact.write_text(
        """
<testsuite tests="1" failures="0">
  <testcase classname="tests.test_viewer_host_checkpoint_guard" name="test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner" time="0.01" />
</testsuite>
""".strip(),
        encoding="utf-8",
    )
    artifact_hash = hashlib.sha256(artifact.read_bytes()).hexdigest()
    head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=repo_root, check=True, capture_output=True, text=True).stdout.strip()
    receipts = repo_root / "artifacts" / "hooks" / "viewer_host_validation_receipts"
    receipts.mkdir(parents=True, exist_ok=True)
    (receipts / f"{head}.json").write_text(
        json.dumps(
            {
                "head": head,
                "summary": "validated",
                "commands": [
                    "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml --maxfail=1"
                ],
                "evidence": [
                    {
                        "evidence_id": "pytest_checkpoint_guard",
                        "command": "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml --maxfail=1",
                        "artifact_kind": "junit_xml",
                        "artifact_path": "artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                        "artifact_sha256": artifact_hash,
                    }
                ],
                "clean": True,
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    proc = subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_write_contract_proof_receipt.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
        ],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )

    assert proc.returncode != 0
    assert "missing required validation commands" in (proc.stderr or proc.stdout).lower()


def test_contract_proof_writer_requires_evidence_entry_for_each_required_validation_command(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    (repo_root / ".gitignore").write_text("artifacts/\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md", ".gitignore"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)

    (repo_root / "tools").mkdir()
    (repo_root / "docs" / "notes").mkdir(parents=True)
    (repo_root / "docs" / "notes" / "plan_PHASED_PLAN.md").write_text(
        "# Plan\n\n## Current Phase\n\nPhase 1 - X\n\n## Phase Checklist\n\n- [ ] Phase 1 - X\n",
        encoding="utf-8",
    )
    (repo_root / "docs" / "contracts").mkdir(parents=True)
    (repo_root / "docs" / "contracts" / "slice.contract.json").write_text(
        json.dumps(
            {
                "version": 1,
                "contract_id": "slice",
                "feature_id": "feature",
                "workflow_type": "workflow_only",
                "plan_path": "docs/notes/plan_PHASED_PLAN.md",
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["fits"],
                "forbidden_operator_prompts": ["state.json"],
                "required_defaults": {"base_fractal_type": "explaino"},
                "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
                "required_validation_commands": [
                    "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                    "py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/hard_denial_workflow_lock_and_fits_reclosure.contract.json --out-json artifacts/validation/viewer_host_validate_slice_contract.json",
                ],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "strict_banner_emitted",
                        "description": "strict banner emitted",
                        "evidence_kind": "pytest_junit_case",
                        "artifact_path": "artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                        "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner",
                    }
                ],
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    subprocess.run(
        ["git", "add", "tools", "docs/notes/plan_PHASED_PLAN.md", "docs/contracts/slice.contract.json"],
        cwd=repo_root,
        check=True,
        capture_output=True,
        text=True,
    )
    subprocess.run(["git", "commit", "-m", "seed slice"], cwd=repo_root, check=True, capture_output=True, text=True)

    subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_prepare_slice.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
            "--plan",
            "docs/notes/plan_PHASED_PLAN.md",
            "--contract",
            "docs/contracts/slice.contract.json",
        ],
        cwd=str(repo_root),
        check=True,
        capture_output=True,
        text=True,
    )

    artifact = repo_root / "artifacts" / "pytest" / "test_viewer_host_checkpoint_guard.junit.xml"
    artifact.parent.mkdir(parents=True, exist_ok=True)
    artifact.write_text(
        """
<testsuite tests="1" failures="0">
  <testcase classname="tests.test_viewer_host_checkpoint_guard" name="test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner" time="0.01" />
</testsuite>
""".strip(),
        encoding="utf-8",
    )
    artifact_hash = hashlib.sha256(artifact.read_bytes()).hexdigest()
    head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=repo_root, check=True, capture_output=True, text=True).stdout.strip()
    receipts = repo_root / "artifacts" / "hooks" / "viewer_host_validation_receipts"
    receipts.mkdir(parents=True, exist_ok=True)
    (receipts / f"{head}.json").write_text(
        json.dumps(
            {
                "head": head,
                "summary": "validated",
                "commands": [
                    "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                    "py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/hard_denial_workflow_lock_and_fits_reclosure.contract.json --out-json artifacts/validation/viewer_host_validate_slice_contract.json",
                ],
                "evidence": [
                    {
                        "evidence_id": "pytest_checkpoint_guard",
                        "command": "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                        "artifact_kind": "junit_xml",
                        "artifact_path": "artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                        "artifact_sha256": artifact_hash,
                    }
                ],
                "clean": True,
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    proc = subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_write_contract_proof_receipt.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
        ],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )

    assert proc.returncode != 0
    assert "missing parseable evidence" in (proc.stderr or proc.stdout).lower()


def test_contract_proof_receipt_reports_missing_commands_and_evidence_together(monkeypatch, tmp_path: Path, capsys) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    opaque_command = "opaque required command"
    missing_command = "missing required command"

    monkeypatch.setattr("tools.viewer_host_write_contract_proof_receipt.discover_repo_root", lambda _path: repo_root)
    monkeypatch.setattr(
        "tools.viewer_host_write_contract_proof_receipt.capture_repo_snapshot",
        lambda _repo_root: {"clean": True, "head": "abc123"},
    )
    monkeypatch.setattr(
        "tools.viewer_host_write_contract_proof_receipt.validate_locked_contract_state",
        lambda _session_id, _repo_root: (
            {
                "contract_id": "contract",
                "contract_path": "docs/contracts/contract.json",
                "contract_hash": "hash",
            },
            "",
        ),
    )
    monkeypatch.setattr(
        "tools.viewer_host_write_contract_proof_receipt.load_and_validate_slice_contract",
        lambda _contract_path, _repo_root: (
            {
                "required_validation_commands": [
                    opaque_command,
                    missing_command,
                ],
                "required_acceptance_assertions": [],
            },
            type("_Result", (), {"ok": True, "errors": []})(),
        ),
    )
    monkeypatch.setattr(
        "tools.viewer_host_write_contract_proof_receipt.load_validation_receipt",
        lambda _head, _repo_root: {
            "head": "abc123",
            "commands": [opaque_command],
            "evidence": [],
        },
    )

    rc = write_contract_proof_main([
        "--session-id",
        "session-1",
        "--cwd",
        str(repo_root),
    ])

    assert rc == 2
    err = capsys.readouterr().err
    assert "contract proof preflight failed" in err
    assert "missing required validation commands" in err
    assert missing_command in err
    assert "missing parseable evidence for required validation commands" in err
    assert opaque_command in err
