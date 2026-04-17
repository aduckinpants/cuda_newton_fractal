from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_contract_state import contract_state_path_for_session
from tools.viewer_host_prepare_slice import main as prepare_slice_main
from tools.viewer_host_validate_slice_contract import main as validate_slice_contract_main
from tools.viewer_host_contract_state import validate_slice_contract_payload


def _write_minimal_plan(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        "# Example\n\n## Current Phase\n\nPhase 1 - Lock\n\n## Phase Checklist\n\n- [ ] Phase 1 - Lock\n",
        encoding="utf-8",
    )


def _write_minimal_contract(path: Path, plan_relative: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(
            {
                "contract_id": "example_contract",
                "feature_id": "example_feature",
                "workflow_type": "workflow_only",
                "plan_path": plan_relative,
                "allowed_mutation_scope": ["tools"],
                "required_operator_inputs": ["fits"],
                "forbidden_operator_prompts": ["state.json"],
                "required_defaults": {"base_fractal_type": "explaino"},
                "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
                "required_validation_commands": ["py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q"],
                "required_acceptance_assertions": [
                    {
                        "assertion_id": "strict_banner_present",
                        "description": "Strict banner test passes",
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


def test_validate_slice_contract_accepts_repo_contract() -> None:
    assert (
        validate_slice_contract_main(
            [
                "--contract",
                "docs/contracts/hard_denial_workflow_lock_and_fits_reclosure.contract.json",
            ]
        )
        == 0
    )


def test_prepare_slice_writes_active_contract_state(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "tools").mkdir()

    plan_relative = "docs/notes/example_PHASED_PLAN.md"
    contract_relative = "docs/contracts/example.contract.json"
    _write_minimal_plan(repo_root / plan_relative)
    _write_minimal_contract(repo_root / contract_relative, plan_relative)

    assert (
        prepare_slice_main(
            [
                "--session-id",
                "session-1",
                "--cwd",
                str(repo_root),
                "--plan",
                plan_relative,
                "--contract",
                contract_relative,
            ]
        )
        == 0
    )

    state_path = contract_state_path_for_session("session-1", repo_root)
    payload = json.loads(state_path.read_text(encoding="utf-8"))
    assert payload["contract_id"] == "example_contract"
    assert payload["contract_path"] == contract_relative
    assert payload["plan_path"] == plan_relative


def test_validate_fits_contract_passes_on_repo_defaults() -> None:
    proc = subprocess.run(
        [
            sys.executable,
            "tools/viewer_host_validate_fits_contract.py",
            "--contract",
            "docs/contracts/runtime_walk_fits.contract.json",
        ],
        cwd=str(REPO_ROOT),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    assert proc.returncode == 0, proc.stderr or proc.stdout


def test_validate_slice_contract_payload_rejects_string_assertions() -> None:
    payload = {
        "contract_id": "example_contract",
        "feature_id": "example_feature",
        "workflow_type": "workflow_only",
        "plan_path": "docs/notes/example_PHASED_PLAN.md",
        "allowed_mutation_scope": ["tools"],
        "required_operator_inputs": ["fits"],
        "forbidden_operator_prompts": ["state.json"],
        "required_defaults": {"base_fractal_type": "explaino"},
        "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
        "required_validation_commands": ["py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml"],
        "required_acceptance_assertions": ["strict banner present"],
    }

    result = validate_slice_contract_payload(payload, REPO_ROOT)

    assert result.ok is False
    assert any("required_acceptance_assertions" in error for error in result.errors)


def test_validate_slice_contract_payload_rejects_unknown_assertion_evidence_kind() -> None:
    payload = {
        "contract_id": "example_contract",
        "feature_id": "example_feature",
        "workflow_type": "workflow_only",
        "plan_path": "docs/notes/example_PHASED_PLAN.md",
        "allowed_mutation_scope": ["tools"],
        "required_operator_inputs": ["fits"],
        "forbidden_operator_prompts": ["state.json"],
        "required_defaults": {"base_fractal_type": "explaino"},
        "forbidden_defaults": {"default_warp_binding": "params.explaino_warp_strength"},
        "required_validation_commands": ["py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml"],
        "required_acceptance_assertions": [
            {
                "assertion_id": "strict_banner_present",
                "description": "Strict banner test passes",
                "evidence_kind": "not_real",
                "artifact_path": "artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
                "test_nodeid": "tests/test_viewer_host_checkpoint_guard.py::test_build_pretool_response_allows_other_tools_but_still_emits_strict_banner",
            }
        ],
    }

    result = validate_slice_contract_payload(payload, REPO_ROOT)

    assert result.ok is False
    assert any("evidence_kind" in error for error in result.errors)


def test_run_repo_mutation_rejects_non_wrapper_delegate(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "tools").mkdir()

    plan_relative = "docs/notes/example_PHASED_PLAN.md"
    contract_relative = "docs/contracts/example.contract.json"
    _write_minimal_plan(repo_root / plan_relative)
    _write_minimal_contract(repo_root / contract_relative, plan_relative)
    assert (
        prepare_slice_main(
            [
                "--session-id",
                "session-1",
                "--cwd",
                str(repo_root),
                "--plan",
                plan_relative,
                "--contract",
                contract_relative,
            ]
        )
        == 0
    )

    proc = subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_run_repo_mutation.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
            "--",
            "cmd",
            "/c",
            "echo",
            "mutate",
        ],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    assert proc.returncode != 0
    assert "only approved viewer_host_* wrapper delegates are allowed" in (proc.stderr or proc.stdout)


def test_run_repo_mutation_rejects_drifted_locked_contract(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    subprocess.run(["git", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.email", "agent@example.com"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "config", "user.name", "Agent"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "README.md").write_text("hello\n", encoding="utf-8")
    subprocess.run(["git", "add", "README.md"], cwd=repo_root, check=True, capture_output=True, text=True)
    subprocess.run(["git", "commit", "-m", "init"], cwd=repo_root, check=True, capture_output=True, text=True)
    (repo_root / "tools").mkdir()

    plan_relative = "docs/notes/example_PHASED_PLAN.md"
    contract_relative = "docs/contracts/example.contract.json"
    _write_minimal_plan(repo_root / plan_relative)
    contract_path = repo_root / contract_relative
    _write_minimal_contract(contract_path, plan_relative)
    assert (
        prepare_slice_main(
            [
                "--session-id",
                "session-1",
                "--cwd",
                str(repo_root),
                "--plan",
                plan_relative,
                "--contract",
                contract_relative,
            ]
        )
        == 0
    )

    contract_path.write_text(contract_path.read_text(encoding="utf-8").replace("example_feature", "drifted_feature"), encoding="utf-8")
    proc = subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_run_repo_mutation.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
            "--",
            sys.executable,
            str(REPO_ROOT / "tools" / "viewer_host_prepare_slice.py"),
            "--session-id",
            "session-1",
            "--cwd",
            str(repo_root),
            "--plan",
            plan_relative,
            "--contract",
            contract_relative,
        ],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    assert proc.returncode != 0
    assert "active contract changed after it was locked" in (proc.stderr or proc.stdout)
