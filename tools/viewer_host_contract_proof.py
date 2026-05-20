from __future__ import annotations

import json
import re
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

try:
    from tools.viewer_host_contract_state import hash_file
except ModuleNotFoundError:
    from viewer_host_contract_state import hash_file


@dataclass(frozen=True)
class ValidationEvidenceSpec:
    evidence_id: str
    command: str
    artifact_kind: str
    artifact_path: str


KNOWN_VALIDATION_EVIDENCE_SPECS: tuple[ValidationEvidenceSpec, ...] = (
    ValidationEvidenceSpec(
        evidence_id="pytest_checkpoint_guard",
        command="py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
        artifact_kind="junit_xml",
        artifact_path="artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml",
    ),
    ValidationEvidenceSpec(
        evidence_id="pytest_agent_workflow_tools",
        command="py -3.14 -m pytest tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/test_agent_workflow_tools.junit.xml",
        artifact_kind="junit_xml",
        artifact_path="artifacts/pytest/test_agent_workflow_tools.junit.xml",
    ),
    ValidationEvidenceSpec(
        evidence_id="pytest_viewer_host_run_logged_command",
        command="py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py -q --junitxml artifacts/pytest/test_viewer_host_run_logged_command.junit.xml",
        artifact_kind="junit_xml",
        artifact_path="artifacts/pytest/test_viewer_host_run_logged_command.junit.xml",
    ),
    ValidationEvidenceSpec(
        evidence_id="pytest_viewer_host_contract_tools",
        command="py -3.14 -m pytest tests/test_viewer_host_contract_tools.py -q --junitxml artifacts/pytest/test_viewer_host_contract_tools.junit.xml",
        artifact_kind="junit_xml",
        artifact_path="artifacts/pytest/test_viewer_host_contract_tools.junit.xml",
    ),
    ValidationEvidenceSpec(
        evidence_id="pytest_viewer_host_contract_proof",
        command="py -3.14 -m pytest tests/test_viewer_host_contract_proof.py -q --junitxml artifacts/pytest/test_viewer_host_contract_proof.junit.xml",
        artifact_kind="junit_xml",
        artifact_path="artifacts/pytest/test_viewer_host_contract_proof.junit.xml",
    ),
    ValidationEvidenceSpec(
        evidence_id="pytest_runtime_walk_viewer",
        command="py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -q --junitxml artifacts/pytest/test_fractal_runtime_runtime_walk_viewer.junit.xml",
        artifact_kind="junit_xml",
        artifact_path="artifacts/pytest/test_fractal_runtime_runtime_walk_viewer.junit.xml",
    ),
    ValidationEvidenceSpec(
        evidence_id="validator_slice_contract",
        command="py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/hard_denial_workflow_lock_and_fits_reclosure.contract.json --out-json artifacts/validation/viewer_host_validate_slice_contract.json",
        artifact_kind="validator_json",
        artifact_path="artifacts/validation/viewer_host_validate_slice_contract.json",
    ),
    ValidationEvidenceSpec(
        evidence_id="validator_fits_contract",
        command="py -3.14 tools/viewer_host_validate_fits_contract.py --contract docs/contracts/runtime_walk_fits.contract.json --out-json artifacts/validation/viewer_host_validate_fits_contract.json",
        artifact_kind="validator_json",
        artifact_path="artifacts/validation/viewer_host_validate_fits_contract.json",
    ),
)


KNOWN_VALIDATION_EVIDENCE_BY_COMMAND = {spec.command: spec for spec in KNOWN_VALIDATION_EVIDENCE_SPECS}


def _dynamic_validator_json_spec(command: str) -> ValidationEvidenceSpec | None:
    match = re.search(r"--out-json\s+(?P<artifact>\S+)", command)
    if match is None:
        return None
    normalized = command.replace("\\", "/")
    allowed_tools = (
        "tools/viewer_host_validate_",
        "tools/viewer_host_truth_report.py",
        "tools/viewer_host_forensic_timeline.py",
        "tools/viewer_host_claim_ledger.py",
    )
    if not any(tool in normalized for tool in allowed_tools):
        return None
    artifact_path = match.group("artifact").replace("\\", "/")
    safe_suffix = re.sub(r"[^A-Za-z0-9]+", "_", artifact_path).strip("_") or "artifact"
    return ValidationEvidenceSpec(
        evidence_id=f"validator_json_{safe_suffix}",
        command=command,
        artifact_kind="validator_json",
        artifact_path=artifact_path,
    )


def _dynamic_pytest_junit_spec(command: str) -> ValidationEvidenceSpec | None:
    normalized = command.replace("\\", "/").strip()
    if " -m pytest " not in f" {normalized} ":
        return None
    match = re.search(r"--junitxml\s+(?P<artifact>\S+)", normalized)
    if match is None:
        return None
    artifact_path = match.group("artifact")
    safe_suffix = re.sub(r"[^A-Za-z0-9]+", "_", artifact_path).strip("_") or "pytest"
    return ValidationEvidenceSpec(
        evidence_id=f"junit_xml_{safe_suffix}",
        command=command,
        artifact_kind="junit_xml",
        artifact_path=artifact_path,
    )


def _dynamic_test_coverage_audit_spec(command: str) -> ValidationEvidenceSpec | None:
    normalized = command.replace("\\", "/").strip()
    if "tools/test_coverage_audit.py" not in normalized:
        return None
    match = re.search(r"--out\s+(?P<artifact>\S+)", normalized)
    if match is None:
        return None
    artifact_path = match.group("artifact")
    return ValidationEvidenceSpec(
        evidence_id="validator_test_coverage_audit",
        command=command,
        artifact_kind="validator_json",
        artifact_path=artifact_path,
    )


def _dynamic_code_quality_audit_spec(command: str) -> ValidationEvidenceSpec | None:
    normalized = command.replace("\\", "/").strip()
    if "tools/code_quality_audit.py" not in normalized:
        return None
    match = re.search(r"--out\s+(?P<artifact>\S+)", normalized)
    if match is None:
        return None
    artifact_path = match.group("artifact")
    return ValidationEvidenceSpec(
        evidence_id="validator_code_quality_audit",
        command=command,
        artifact_kind="validator_json",
        artifact_path=artifact_path,
    )


def _dynamic_salt_ndepend_freeze_gate_spec(command: str) -> ValidationEvidenceSpec | None:
    normalized = command.replace("\\", "/").strip()
    if "tools/viewer_host_salt_ndepend.py freeze-gate" not in normalized:
        return None
    match = re.search(r"--out-dir\s+(?P<artifact_dir>\S+)", normalized)
    if match is None:
        return None
    artifact_dir = match.group("artifact_dir").rstrip("/")
    return ValidationEvidenceSpec(
        evidence_id="validator_salt_ndepend_freeze_gate",
        command=command,
        artifact_kind="validator_json",
        artifact_path=f"{artifact_dir}/doctor.json",
    )


def _dynamic_redirected_log_spec(command: str) -> ValidationEvidenceSpec | None:
    normalized = command.replace("\\", "/").strip()
    match = re.search(r">\s+(?P<artifact>\S+\.log)\s+2>&1\s*$", normalized)
    if match is None:
        return None
    artifact_path = match.group("artifact")
    if not artifact_path.startswith("artifacts/"):
        return None
    safe_suffix = re.sub(r"[^A-Za-z0-9]+", "_", artifact_path).strip("_") or "log"
    return ValidationEvidenceSpec(
        evidence_id=f"text_log_{safe_suffix}",
        command=command,
        artifact_kind="text_log",
        artifact_path=artifact_path,
    )


def validation_evidence_spec_for_command(command: str) -> ValidationEvidenceSpec | None:
    spec = KNOWN_VALIDATION_EVIDENCE_BY_COMMAND.get(command)
    if spec is not None:
        return spec
    normalized = command.replace("\\", "/").strip()
    if normalized == "py -3.14 tools/viewer_host_assert_phased_plan_sync.py":
        return ValidationEvidenceSpec(
            evidence_id="validator_plan_sync",
            command=command,
            artifact_kind="validator_json",
            artifact_path="artifacts/validation/viewer_host_assert_phased_plan_sync.json",
        )
    spec = _dynamic_pytest_junit_spec(command)
    if spec is not None:
        return spec
    spec = _dynamic_test_coverage_audit_spec(command)
    if spec is not None:
        return spec
    spec = _dynamic_code_quality_audit_spec(command)
    if spec is not None:
        return spec
    spec = _dynamic_salt_ndepend_freeze_gate_spec(command)
    if spec is not None:
        return spec
    spec = _dynamic_redirected_log_spec(command)
    if spec is not None:
        return spec
    return _dynamic_validator_json_spec(command)


def build_validation_evidence_entries(commands: list[str], repo_root: Path) -> list[dict[str, Any]]:
    entries: list[dict[str, Any]] = []
    seen_ids: set[str] = set()
    for command in commands:
        spec = validation_evidence_spec_for_command(command)
        if spec is None or spec.evidence_id in seen_ids:
            continue
        artifact_path = (repo_root / spec.artifact_path).resolve()
        if not artifact_path.exists():
            raise RuntimeError(
                "Missing validation artifact for command: "
                + command
                + " | expected "
                + spec.artifact_path
            )
        artifact_stat = artifact_path.stat()
        entries.append(
            {
                "evidence_id": spec.evidence_id,
                "command": spec.command,
                "artifact_kind": spec.artifact_kind,
                "artifact_path": spec.artifact_path,
                "artifact_mtime_utc": datetime.fromtimestamp(
                    artifact_stat.st_mtime,
                    timezone.utc,
                ).isoformat(),
                "artifact_size_bytes": artifact_stat.st_size,
                "artifact_sha256": hash_file(artifact_path),
            }
        )
        seen_ids.add(spec.evidence_id)
    return entries


def junit_case_node_id(class_name: str, test_name: str) -> str:
    if class_name.endswith(".py"):
        file_path = class_name.replace(".", "/")
    else:
        file_path = class_name.replace(".", "/") + ".py"
    return f"{file_path}::{test_name}"


def load_junit_case_results(path: Path) -> dict[str, dict[str, Any]]:
    root = ET.fromstring(path.read_text(encoding="utf-8"))
    out: dict[str, dict[str, Any]] = {}
    for testcase in root.iter("testcase"):
        class_name = testcase.attrib.get("classname", "").strip()
        test_name = testcase.attrib.get("name", "").strip()
        if not class_name or not test_name:
            continue
        node_id = junit_case_node_id(class_name, test_name)
        failure = testcase.find("failure")
        error = testcase.find("error")
        skipped = testcase.find("skipped")
        ok = failure is None and error is None and skipped is None
        message = ""
        if failure is not None:
            message = failure.attrib.get("message", "") or (failure.text or "").strip()
        elif error is not None:
            message = error.attrib.get("message", "") or (error.text or "").strip()
        elif skipped is not None:
            message = skipped.attrib.get("message", "") or (skipped.text or "").strip()
        out[node_id] = {
            "ok": ok,
            "message": message,
        }
    return out


def resolve_json_path(payload: Any, path_text: str) -> Any:
    current: Any = payload
    for raw_part in path_text.split("."):
        part = raw_part.strip()
        if not part:
            raise KeyError(path_text)
        if isinstance(current, list):
            index = int(part)
            current = current[index]
            continue
        if not isinstance(current, dict) or part not in current:
            raise KeyError(path_text)
        current = current[part]
    return current


def load_validator_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def load_artifact_evidence(evidence_kind: str, artifact_path: Path) -> dict[str, Any]:
    if evidence_kind in {"pytest_junit_case", "runtime_junit_case"}:
        return load_junit_case_results(artifact_path)
    if evidence_kind == "validator_json":
        return load_validator_json(artifact_path)
    raise RuntimeError(f"Unsupported evidence_kind: {evidence_kind}")


def _find_matching_validation_evidence(assertion: dict[str, Any], validation_receipt: dict[str, Any] | None) -> dict[str, Any] | None:
    if validation_receipt is None:
        return None
    artifact_path = str(assertion.get("artifact_path", "")).replace("\\", "/")
    for entry in validation_receipt.get("evidence", []) or []:
        if not isinstance(entry, dict):
            continue
        if str(entry.get("artifact_path", "")).replace("\\", "/") == artifact_path:
            return entry
    return None


def evaluate_assertion(
    assertion: dict[str, Any],
    repo_root: Path,
    validation_receipt: dict[str, Any] | None = None,
) -> dict[str, Any]:
    evidence_kind = str(assertion["evidence_kind"])
    artifact_path = Path(str(assertion["artifact_path"]))
    if not artifact_path.is_absolute():
        artifact_path = (repo_root / artifact_path).resolve()
    result = {
        "assertion_id": str(assertion["assertion_id"]),
        "ok": False,
        "evidence_kind": evidence_kind,
        "artifact_path": str(artifact_path),
    }
    evidence_entry = _find_matching_validation_evidence(assertion, validation_receipt)
    if validation_receipt is not None and evidence_entry is None:
        result["failure_detail"] = "missing validation evidence entry"
        return result
    if validation_receipt is not None and evidence_entry is not None:
        result["evidence_id"] = str(evidence_entry.get("evidence_id", ""))
        recorded_kind = str(evidence_entry.get("artifact_kind", "")).strip()
        expected_recorded_kind = "validator_json" if evidence_kind == "validator_json" else "junit_xml"
        if recorded_kind and recorded_kind != expected_recorded_kind:
            result["failure_detail"] = (
                "validation evidence kind mismatch: "
                + recorded_kind
                + " != "
                + expected_recorded_kind
            )
            return result
    if not artifact_path.exists():
        result["failure_detail"] = "artifact missing"
        return result
    if validation_receipt is not None and evidence_entry is not None:
        artifact_stat = artifact_path.stat()
        recorded_size = evidence_entry.get("artifact_size_bytes")
        if recorded_size is not None:
            try:
                expected_size = int(recorded_size)
            except (TypeError, ValueError):
                result["failure_detail"] = "validation evidence artifact size is invalid"
                return result
            if artifact_stat.st_size != expected_size:
                result["failure_detail"] = "validation evidence artifact size mismatch"
                return result

        recorded_mtime = str(evidence_entry.get("artifact_mtime_utc", "")).strip()
        if recorded_mtime:
            current_mtime = datetime.fromtimestamp(artifact_stat.st_mtime, timezone.utc).isoformat()
            if recorded_mtime != current_mtime:
                result["failure_detail"] = "validation evidence artifact mtime mismatch"
                return result

        recorded_hash = str(evidence_entry.get("artifact_sha256", "")).strip()
        if recorded_hash and recorded_hash != hash_file(artifact_path):
            result["failure_detail"] = "validation evidence artifact hash mismatch"
            return result

    payload = load_artifact_evidence(evidence_kind, artifact_path)
    if evidence_kind in {"pytest_junit_case", "runtime_junit_case"}:
        node_id = str(assertion.get("test_nodeid", "")).strip()
        case = payload.get(node_id)
        result["matched_key"] = node_id
        if case is None:
            result["failure_detail"] = "missing junit testcase"
            return result
        if not bool(case.get("ok", False)):
            result["failure_detail"] = str(case.get("message", "")).strip() or "junit testcase failed"
            return result
        result["ok"] = True
        return result

    json_path = str(assertion.get("json_path", "")).strip()
    result["matched_key"] = json_path
    try:
        actual_value = resolve_json_path(payload, json_path)
    except (KeyError, IndexError, ValueError):
        result["failure_detail"] = "missing validator_json path"
        return result
    expected_value = assertion.get("equals")
    if actual_value != expected_value:
        result["failure_detail"] = f"validator_json mismatch: expected {expected_value!r}, got {actual_value!r}"
        return result
    result["ok"] = True
    return result
