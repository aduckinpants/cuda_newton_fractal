from __future__ import annotations

import hashlib
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
CONTRACT_STATE_DIR = REPO_ROOT / "artifacts" / "hooks" / "viewer_host_contract_guard"
CONTRACT_PROOF_RECEIPT_DIR = REPO_ROOT / "artifacts" / "hooks" / "viewer_host_contract_proof_receipts"
GLOBAL_CONTRACT_SESSION_ID = "global_active_contract"

ALLOWED_WORKFLOW_TYPES = {"viewer_first", "cli_first", "headless_only", "workflow_only"}
STRICT_BANNER_PREFIX = (
    "STRICT REPO RULE: the active checked-in plan/contract is binding. "
    "Raw mutation is forbidden. Contract drift is forbidden. "
    "Closure without machine proof is forbidden."
)


@dataclass(frozen=True)
class SliceContractValidationResult:
    ok: bool
    errors: list[str]


def _sanitize_session_id(session_id: str) -> str:
    safe_chars: list[str] = []
    for char in session_id:
        if char.isalnum() or char in ("-", "_"):
            safe_chars.append(char)
        else:
            safe_chars.append("_")
    sanitized = "".join(safe_chars).strip("_")
    return sanitized or "unknown_session"


def _normalize_repo_relative(path: str | Path, repo_root: Path = REPO_ROOT) -> str:
    candidate = Path(path)
    if not candidate.is_absolute():
        candidate = repo_root / candidate
    return candidate.resolve().relative_to(repo_root.resolve()).as_posix()


def hash_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        while True:
            chunk = handle.read(65536)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()


def load_json_file(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def contract_state_path_for_session(session_id: str, repo_root: Path = REPO_ROOT) -> Path:
    return repo_root / "artifacts" / "hooks" / "viewer_host_contract_guard" / f"{_sanitize_session_id(session_id)}.json"


def load_active_contract_state(session_id: str, repo_root: Path = REPO_ROOT) -> dict[str, Any] | None:
    path = contract_state_path_for_session(session_id, repo_root)
    if not path.exists():
        if session_id != GLOBAL_CONTRACT_SESSION_ID:
            global_path = contract_state_path_for_session(GLOBAL_CONTRACT_SESSION_ID, repo_root)
            if global_path.exists():
                return load_json_file(global_path)
        return None
    return load_json_file(path)


def validate_locked_contract_state(session_id: str, repo_root: Path = REPO_ROOT) -> tuple[dict[str, Any] | None, str]:
    contract_state = load_active_contract_state(session_id, repo_root)
    if contract_state is None:
        return None, "no active contract state"
    contract_path_text = str(contract_state.get("contract_path", "")).strip()
    if not contract_path_text:
        return None, "active contract state is missing contract_path"
    contract_path = repo_root / contract_path_text
    if not contract_path.exists():
        return None, f"active contract file is missing: {contract_path_text}"
    current_hash = hash_file(contract_path)
    locked_hash = str(contract_state.get("contract_hash", "")).strip()
    if current_hash != locked_hash:
        return None, "active contract changed after it was locked; run viewer_host_revise_contract.py"
    return contract_state, ""


def write_active_contract_state(
    session_id: str,
    *,
    repo_root: Path,
    contract_path: Path,
    contract_payload: dict[str, Any],
) -> Path:
    path = contract_state_path_for_session(session_id, repo_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "session_id": session_id,
        "contract_id": str(contract_payload["contract_id"]),
        "feature_id": str(contract_payload["feature_id"]),
        "workflow_type": str(contract_payload["workflow_type"]),
        "contract_path": _normalize_repo_relative(contract_path, repo_root),
        "plan_path": _normalize_repo_relative(contract_payload["plan_path"], repo_root),
        "contract_hash": hash_file(contract_path),
        "allowed_mutation_scope": list(contract_payload["allowed_mutation_scope"]),
        "required_validation_commands": list(contract_payload.get("required_validation_commands", [])),
        "required_validators": list(contract_payload.get("required_validators", [])),
    }
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return path


def contract_proof_receipt_path(head: str, repo_root: Path = REPO_ROOT) -> Path:
    sanitized = "".join(char for char in head if char.isalnum())
    if not sanitized:
        raise RuntimeError("Contract proof receipt requires a non-empty HEAD id")
    return repo_root / "artifacts" / "hooks" / "viewer_host_contract_proof_receipts" / f"{sanitized}.json"


def load_contract_proof_receipt(head: str, repo_root: Path = REPO_ROOT) -> dict[str, Any] | None:
    path = contract_proof_receipt_path(head, repo_root)
    if not path.exists():
        return None
    return load_json_file(path)


def validate_slice_contract_payload(payload: dict[str, Any], repo_root: Path = REPO_ROOT) -> SliceContractValidationResult:
    errors: list[str] = []
    required_string_fields = ("contract_id", "feature_id", "workflow_type", "plan_path")
    for field in required_string_fields:
        value = payload.get(field)
        if not isinstance(value, str) or not value.strip():
            errors.append(f"missing required string field: {field}")

    workflow_type = str(payload.get("workflow_type", "")).strip()
    if workflow_type and workflow_type not in ALLOWED_WORKFLOW_TYPES:
        errors.append(f"invalid workflow_type: {workflow_type}")

    required_list_fields = (
        "allowed_mutation_scope",
        "required_operator_inputs",
        "forbidden_operator_prompts",
        "required_acceptance_assertions",
        "required_validation_commands",
    )
    for field in required_list_fields:
        value = payload.get(field)
        if not isinstance(value, list) or not value:
            errors.append(f"missing required non-empty list field: {field}")
        elif not all(isinstance(item, str) and item.strip() for item in value):
            errors.append(f"invalid list entries for field: {field}")

    for dict_field in ("required_defaults", "forbidden_defaults"):
        value = payload.get(dict_field)
        if not isinstance(value, dict) or not value:
            errors.append(f"missing required non-empty object field: {dict_field}")

    allowed_scope = payload.get("allowed_mutation_scope")
    if isinstance(allowed_scope, list):
        for raw_path in allowed_scope:
            if not isinstance(raw_path, str) or not raw_path.strip():
                continue
            candidate = repo_root / raw_path
            if not candidate.exists():
                errors.append(f"allowed_mutation_scope path missing: {raw_path}")

    plan_path = payload.get("plan_path")
    if isinstance(plan_path, str) and plan_path.strip():
        candidate = repo_root / plan_path
        if not candidate.exists():
            errors.append(f"plan_path missing: {plan_path}")

    contract_id = str(payload.get("contract_id", "")).strip()
    if contract_id and any(char.isspace() for char in contract_id):
        errors.append("contract_id must not contain whitespace")

    return SliceContractValidationResult(ok=not errors, errors=errors)


def load_and_validate_slice_contract(contract_path: Path, repo_root: Path = REPO_ROOT) -> tuple[dict[str, Any] | None, SliceContractValidationResult]:
    payload = load_json_file(contract_path)
    result = validate_slice_contract_payload(payload, repo_root)
    return (payload if result.ok else None), result


def build_strict_banner(contract_state: dict[str, Any] | None) -> str:
    if contract_state is None:
        return STRICT_BANNER_PREFIX + " Active contract: <none>."
    return (
        STRICT_BANNER_PREFIX
        + " Active contract: "
        + str(contract_state.get("contract_id", "<unknown>"))
        + " ("
        + str(contract_state.get("contract_path", "<unknown>"))
        + ")."
    )


def contract_paths_in_scope(contract_state: dict[str, Any] | None) -> list[str]:
    if not contract_state:
        return []
    raw = contract_state.get("allowed_mutation_scope", [])
    if not isinstance(raw, list):
        return []
    return [str(item) for item in raw if isinstance(item, str) and item.strip()]


def file_path_is_in_contract_scope(candidate: str | Path, contract_state: dict[str, Any] | None, repo_root: Path = REPO_ROOT) -> bool:
    if contract_state is None:
        return False
    candidate_path = Path(candidate)
    if not candidate_path.is_absolute():
        candidate_path = repo_root / candidate_path
    candidate_path = candidate_path.resolve()
    for raw_scope in contract_paths_in_scope(contract_state):
        scope_path = (repo_root / raw_scope).resolve()
        if scope_path == candidate_path:
            return True
        if scope_path.is_dir():
            try:
                candidate_path.relative_to(scope_path)
                return True
            except ValueError:
                continue
    return False
