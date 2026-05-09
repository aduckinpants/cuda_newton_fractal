from __future__ import annotations

import json
import hashlib
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_contract_proof import evaluate_assertion, load_artifact_evidence, validation_evidence_spec_for_command


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


def test_validation_evidence_spec_for_command_recognizes_logged_command_pytest() -> None:
    command = (
        "py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py -q --junitxml "
        "artifacts/pytest/test_viewer_host_run_logged_command.junit.xml"
    )

    spec = validation_evidence_spec_for_command(command)

    assert spec is not None
    assert spec.artifact_kind == "junit_xml"
    assert spec.artifact_path == "artifacts/pytest/test_viewer_host_run_logged_command.junit.xml"


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
