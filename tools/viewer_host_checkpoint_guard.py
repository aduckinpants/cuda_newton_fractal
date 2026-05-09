from __future__ import annotations

import hashlib
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from dataclasses import dataclass
from pathlib import Path
from typing import Any

try:
    from tools.viewer_host_contract_state import (
        GLOBAL_CONTRACT_SESSION_ID,
        build_strict_banner,
        contract_proof_receipt_path,
        hash_file,
        load_and_validate_slice_contract,
        load_active_contract_state,
        load_contract_proof_receipt,
        validate_locked_contract_state,
    )
    from tools.viewer_host_contract_proof import build_validation_evidence_entries
    from tools.viewer_host_validate_hostile_audit import plan_declares_hostile_audit, validate_hostile_audit_plan
except ModuleNotFoundError:
    from viewer_host_contract_state import (
        GLOBAL_CONTRACT_SESSION_ID,
        build_strict_banner,
        contract_proof_receipt_path,
        hash_file,
        load_and_validate_slice_contract,
        load_active_contract_state,
        load_contract_proof_receipt,
        validate_locked_contract_state,
    )
    from viewer_host_contract_proof import build_validation_evidence_entries
    from viewer_host_validate_hostile_audit import plan_declares_hostile_audit, validate_hostile_audit_plan


REPO_ROOT = Path(__file__).resolve().parents[1]
STATE_DIR = REPO_ROOT / "artifacts" / "hooks" / "viewer_host_checkpoint_guard"
RECEIPT_DIR = REPO_ROOT / "artifacts" / "hooks" / "viewer_host_validation_receipts"
RECOVERY_DIRNAME = "recovery"
ACTIVE_RECOVERY_ADOPTION_FILENAME = "active_recovery_adoption.json"
TASK_COMPLETE_TOOL = "task_complete"
DELETED_MARKER = "__DELETED__"
EXPLICIT_USER_ASK_RE = re.compile(r"^- \[([^\]]+)\]\s*(.+?)\s*$")


@dataclass(frozen=True)
class GuardStatus:
    should_block: bool
    reason: str
    changed_paths: list[str]


@dataclass(frozen=True)
class SessionBaselineResolution:
    baseline: dict[str, Any] | None
    status: str
    changed_paths: list[str]
    baseline_path: str | None = None
    recovery_report_path: str | None = None
    recovery_adoption_path: str | None = None


def _run_git(repo_root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *args],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )


def _git_output(repo_root: Path, *args: str) -> str:
    proc = _run_git(repo_root, *args)
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout or "git command failed").strip()
        raise RuntimeError(detail)
    return proc.stdout or ""


def _git_lines(repo_root: Path, *args: str) -> list[str]:
    lines = []
    for line in _git_output(repo_root, *args).splitlines():
        stripped = line.strip()
        if stripped:
            lines.append(Path(stripped).as_posix())
    return sorted(set(lines))


def _hash_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        while True:
            chunk = handle.read(65536)
            if not chunk:
                break
            digest.update(chunk)
    return digest.hexdigest()


def _worktree_fingerprint(repo_root: Path, relative_path: str) -> str:
    path = repo_root / relative_path
    if not path.exists():
        return DELETED_MARKER
    if path.is_dir():
        return "__DIR__"
    return _hash_file(path)


def _index_fingerprint(repo_root: Path, relative_path: str) -> str:
    proc = _run_git(repo_root, "rev-parse", f":{relative_path}")
    if proc.returncode != 0:
        return DELETED_MARKER
    return (proc.stdout or "").strip()


def _current_head(repo_root: Path) -> str:
    return _git_output(repo_root, "rev-parse", "HEAD").strip()


def _current_branch(repo_root: Path) -> str:
    return _git_output(repo_root, "rev-parse", "--abbrev-ref", "HEAD").strip()


def _find_values(obj: Any, key: str) -> list[Any]:
    out: list[Any] = []
    if isinstance(obj, dict):
        for current_key, value in obj.items():
            if current_key == key:
                out.append(value)
            out.extend(_find_values(value, key))
    elif isinstance(obj, list):
        for item in obj:
            out.extend(_find_values(item, key))
    return out


def _payload_tool_candidates(payload: Any) -> list[str]:
    candidates: list[str] = []
    for key in ("toolName", "tool_name", "tool", "name", "recipient_name", "recipientName"):
        candidates.extend(str(value) for value in _find_values(payload, key) if isinstance(value, (str, int, float)))
    return candidates


def _extract_payload_tool_name(payload: Any, *, default: str = "") -> str:
    candidates = _payload_tool_candidates(payload)
    if not candidates:
        return default
    return str(candidates[0]).strip() or default


def _payload_command_candidates(payload: Any) -> list[str]:
    candidates: list[str] = []
    for key in ("command", "cmd", "script"):
        candidates.extend(str(value) for value in _find_values(payload, key) if isinstance(value, (str, int, float)))
    return candidates


def _extract_payload_command_text(payload: Any) -> str:
    candidates = _payload_command_candidates(payload)
    if not candidates:
        return ""
    return str(candidates[0]).strip()


def _payload_requests_task_complete(payload: Any) -> bool:
    haystack = " ".join(_payload_tool_candidates(payload)).lower()
    return "task_complete" in haystack or "taskcomplete" in haystack


def _normalize_text(text: str) -> str:
    return " ".join(text.replace("/", "\\").split()).lower()


def _contains_forbidden_shell_metacharacters(command_text: str) -> bool:
    normalized = _normalize_text(command_text)
    forbidden_tokens = ("&&", "||", ";", "|", ">", "<", " & ")
    return any(token in normalized for token in forbidden_tokens)


def _is_shell_tool(tool_name: str) -> bool:
    haystack = tool_name.lower()
    return "shell_command" in haystack or haystack.endswith(".shell")


def _is_apply_patch_tool(tool_name: str) -> bool:
    return "apply_patch" in tool_name.lower()


def _is_wrapper_shell_command(command_text: str) -> bool:
    normalized = _normalize_text(command_text)
    wrappers = (
        "tools\\viewer_host_prepare_slice.py",
        "tools\\viewer_host_revise_contract.py",
        "tools\\viewer_host_run_repo_mutation.py",
        "tools\\viewer_host_apply_repo_patch.py",
        "tools\\viewer_host_checkpoint_slice.py",
        "tools\\viewer_host_validate_slice_contract.py",
        "tools\\viewer_host_validate_fits_contract.py",
        "tools\\viewer_host_write_contract_proof_receipt.py",
        "tools\\viewer_host_begin_work_slice.py",
        "tools\\viewer_host_session_bootstrap.py",
        "tools\\viewer_host_assert_phased_plan_sync.py",
    )
    return any(wrapper in normalized for wrapper in wrappers)


def _is_allowed_raw_shell_command(command_text: str) -> bool:
    normalized = _normalize_text(command_text)
    if _contains_forbidden_shell_metacharacters(command_text):
        return False
    prefixes = (
        "get-content ",
        "get-childitem ",
        "rg ",
        "git status",
        "git diff",
        "git rev-parse",
        "git ls-files",
        "py -3.14 tools\\viewer_host_session_bootstrap.py",
        "py -3.14 tools\\viewer_host_begin_work_slice.py",
        "py -3.14 tools\\viewer_host_prepare_slice.py",
        "py -3.14 tools\\viewer_host_revise_contract.py",
        "py -3.14 tools\\viewer_host_recover_crash_state.py",
        "py -3.14 tools\\viewer_host_assert_phased_plan_sync.py",
        "py -3.14 tools\\viewer_host_validate_slice_contract.py",
        "py -3.14 tools\\viewer_host_validate_fits_contract.py",
        "py -3.14 tools\\viewer_host_run_logged_command.py",
        "py -3.14 -m pytest",
        "py -3.14 tools\\viewer_host_runtime_pytest_lane.py",
        "py -3.14 tools\\code_quality_audit.py",
        "cmd /c ui_app\\build_tests_vsdevcmd.cmd",
        "cmd /c ui_app\\build_vsdevcmd.cmd",
    )
    if any(normalized.startswith(prefix) for prefix in prefixes):
        return True
    return False


def _shell_command_is_forbidden_direct_mutation(command_text: str) -> bool:
    normalized = _normalize_text(command_text)
    forbidden = (
        "tools\\viewer_host_append_handoff.py",
        "tools\\viewer_host_write_validation_receipt.py",
        "tools\\viewer_host_write_contract_proof_receipt.py",
        "git commit",
        "git add ",
    )
    return any(token in normalized for token in forbidden)


def discover_repo_root(start_path: Path) -> Path:
    candidate = start_path.resolve()
    proc = _run_git(candidate, "rev-parse", "--show-toplevel")
    if proc.returncode != 0:
        raise RuntimeError((proc.stderr or proc.stdout or "unable to resolve git repo root").strip())
    return Path((proc.stdout or "").strip()).resolve()


def _snapshot_section_has_entries(snapshot: dict[str, Any], section: str) -> bool:
    return bool(snapshot.get(section, {}))


def snapshot_is_clean(snapshot: dict[str, Any]) -> bool:
    return not any(
        _snapshot_section_has_entries(snapshot, section)
        for section in ("unstaged", "staged", "untracked")
    )


def capture_repo_snapshot(repo_root: Path = REPO_ROOT) -> dict[str, Any]:
    repo_root = repo_root.resolve()
    unstaged = {
        path: _worktree_fingerprint(repo_root, path)
        for path in _git_lines(repo_root, "diff", "--name-only", "--diff-filter=ACDMRTUXB", "--relative")
    }
    staged = {
        path: _index_fingerprint(repo_root, path)
        for path in _git_lines(repo_root, "diff", "--cached", "--name-only", "--diff-filter=ACDMRTUXB", "--relative")
    }
    untracked = {
        path: _worktree_fingerprint(repo_root, path)
        for path in _git_lines(repo_root, "ls-files", "--others", "--exclude-standard")
    }
    snapshot = {
        "repo_root": repo_root.as_posix(),
        "head": _current_head(repo_root),
        "unstaged": unstaged,
        "staged": staged,
        "untracked": untracked,
    }
    snapshot["clean"] = snapshot_is_clean(snapshot)
    return snapshot


def recovery_dir(repo_root: Path = REPO_ROOT) -> Path:
    return repo_root / "artifacts" / "hooks" / "viewer_host_checkpoint_guard" / RECOVERY_DIRNAME


def recovery_adoption_path(repo_root: Path = REPO_ROOT) -> Path:
    return recovery_dir(repo_root) / ACTIVE_RECOVERY_ADOPTION_FILENAME


def snapshot_digest(snapshot: dict[str, Any]) -> str:
    return hashlib.sha256(
        json.dumps(snapshot, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def recovery_report_path_for_snapshot(snapshot: dict[str, Any], repo_root: Path = REPO_ROOT) -> Path:
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return recovery_dir(repo_root) / f"recovery_{stamp}_{snapshot_digest(snapshot)[:12]}.json"


def load_active_recovery_adoption(repo_root: Path = REPO_ROOT) -> dict[str, Any] | None:
    path = recovery_adoption_path(repo_root)
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def _snapshot_without_paths(
    snapshot: dict[str, Any],
    *,
    repo_root: Path = REPO_ROOT,
    ignored_relative_paths: list[str] | None = None,
) -> dict[str, Any]:
    normalized = json.loads(json.dumps(snapshot))
    ignored = {path.replace("\\", "/") for path in (ignored_relative_paths or []) if str(path).strip()}
    for section in ("unstaged", "staged", "untracked"):
        entries = normalized.get(section, {}) or {}
        normalized[section] = {path: value for path, value in entries.items() if path not in ignored}
    normalized["clean"] = snapshot_is_clean(normalized)
    return normalized


def recovery_adoption_matches_snapshot(
    adoption_payload: dict[str, Any],
    snapshot: dict[str, Any],
    repo_root: Path = REPO_ROOT,
) -> bool:
    expected = str(adoption_payload.get("snapshot_digest", "")).strip()
    comparable_snapshot = _snapshot_without_paths(
        snapshot,
        repo_root=repo_root,
        ignored_relative_paths=[
            str(adoption_payload.get("report_path", "")).strip(),
            recovery_adoption_path(repo_root).relative_to(repo_root).as_posix(),
        ],
    )
    return bool(expected) and expected == snapshot_digest(comparable_snapshot)


def consume_active_recovery_adoption(repo_root: Path = REPO_ROOT) -> None:
    path = recovery_adoption_path(repo_root)
    if path.exists():
        path.unlink()


def write_active_recovery_adoption(
    snapshot: dict[str, Any],
    *,
    report_path: Path,
    summary: str,
    reason: str,
    repo_root: Path = REPO_ROOT,
) -> Path:
    path = recovery_adoption_path(repo_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    changed_paths = compare_snapshots({"unstaged": {}, "staged": {}, "untracked": {}}, snapshot)
    payload = {
        "repo_root": repo_root.as_posix(),
        "created_at_utc": datetime.now(timezone.utc).isoformat(),
        "summary": summary,
        "reason": reason,
        "head": str(snapshot.get("head", "")).strip(),
        "snapshot_digest": snapshot_digest(snapshot),
        "changed_paths": changed_paths,
        "report_path": report_path.relative_to(repo_root).as_posix(),
    }
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return path


def compare_snapshots(baseline: dict[str, Any], current: dict[str, Any]) -> list[str]:
    changed_paths: set[str] = set()
    for section in ("unstaged", "staged", "untracked"):
        baseline_entries = baseline.get(section, {}) or {}
        current_entries = current.get(section, {}) or {}
        for path in set(baseline_entries) | set(current_entries):
            if baseline_entries.get(path) != current_entries.get(path):
                changed_paths.add(path)
    return sorted(changed_paths)


def validation_receipt_path(head: str, repo_root: Path = REPO_ROOT) -> Path:
    sanitized = "".join(ch for ch in head if ch.isalnum())
    if not sanitized:
        raise RuntimeError("Validation receipt requires a non-empty HEAD id")
    return repo_root / "artifacts" / "hooks" / "viewer_host_validation_receipts" / f"{sanitized}.json"


def load_validation_receipt(head: str, repo_root: Path = REPO_ROOT) -> dict[str, Any] | None:
    path = validation_receipt_path(head, repo_root)
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def clean_head_has_closure_receipts(snapshot: dict[str, Any], repo_root: Path = REPO_ROOT) -> bool:
    if not snapshot_is_clean(snapshot):
        return False
    head = str(snapshot.get("head", "")).strip()
    if not head:
        return False
    validation_receipt = load_validation_receipt(head, repo_root)
    if validation_receipt is None:
        return False
    if str(validation_receipt.get("head", "")).strip() != head:
        return False

    contract_receipt = load_contract_proof_receipt(head, repo_root)
    if contract_receipt is None:
        return False
    if str(contract_receipt.get("head", "")).strip() != head:
        return False

    contract_state, contract_error = validate_locked_contract_state(GLOBAL_CONTRACT_SESSION_ID, repo_root)
    if contract_error and contract_error != "no active contract state":
        return False
    if contract_state is None:
        return True

    if str(contract_receipt.get("contract_id", "")).strip() != str(contract_state.get("contract_id", "")).strip():
        return False
    if str(contract_receipt.get("contract_hash", "")).strip() != str(contract_state.get("contract_hash", "")).strip():
        return False
    return True


def _viewer_first_validation_commands(commands: Any) -> list[str]:
    if not isinstance(commands, list):
        return []
    out: list[str] = []
    for command in commands:
        text = str(command).strip()
        if text:
            out.append(text)
    return out


def _command_counts_as_runtime_publish(command: str) -> bool:
    normalized = _normalize_text(command)
    return any(
        token in normalized
        for token in (
            "ui_app\\build_vsdevcmd.cmd",
            "verify: runtime publish",
            "verify: profile runtime",
            "verify: profile checkpoint",
        )
    )


def _command_counts_as_published_runtime_proof(command: str) -> bool:
    normalized = _normalize_text(command)
    if "verify: profile runtime" in normalized or "verify: profile checkpoint" in normalized:
        return True
    if "tools\\viewer_host_runtime_pytest_lane.py" in normalized:
        return True
    if "pytest" not in normalized:
        return False
    return any(
        token in normalized
        for token in (
            "tests\\test_explaino_runtime_walk_tool.py",
            "tests\\test_flashlight_bridge_runner.py",
            "tests\\test_fractal_runtime",
            "tests\\test_function_descriptor_cli.py",
            "tests\\test_generic_probe_cli.py",
        )
    )


def evaluate_viewer_first_validation_command_guard(
    commands: Any,
    workflow_type: str | None,
) -> tuple[bool, str]:
    if str(workflow_type or "").strip() != "viewer_first":
        return False, ""

    command_list = _viewer_first_validation_commands(commands)
    if not any(_command_counts_as_runtime_publish(command) for command in command_list):
        return True, (
            "Viewer-first validation receipt is missing a runtime publish command. "
            "Record `ui_app/build_vsdevcmd.cmd` or the equivalent runtime/checkpoint profile before closure."
        )
    if not any(_command_counts_as_published_runtime_proof(command) for command in command_list):
        return True, (
            "Viewer-first validation receipt is missing published-runtime proof. "
            "Record `tools/viewer_host_runtime_pytest_lane.py` or direct pytest against `tests/test_fractal_runtime*.py` "
            "(or the runtime/checkpoint profile) before closure."
        )
    return False, ""


def write_validation_receipt(
    summary: str,
    *,
    repo_root: Path = REPO_ROOT,
    commands: list[str] | None = None,
    notes: list[str] | None = None,
) -> Path:
    repo_root = repo_root.resolve()
    snapshot = capture_repo_snapshot(repo_root)
    if not snapshot_is_clean(snapshot):
        raise RuntimeError("Validation receipt requires a clean repository state")

    commands_list = _viewer_first_validation_commands(list(commands or []))
    contract_state, contract_error = validate_locked_contract_state(GLOBAL_CONTRACT_SESSION_ID, repo_root)
    if contract_error and contract_error != "no active contract state":
        raise RuntimeError(contract_error)

    should_block, reason = evaluate_viewer_first_validation_command_guard(
        commands_list,
        None if contract_error else str(contract_state.get("workflow_type", "")).strip(),
    )
    if should_block:
        raise RuntimeError(reason)

    head = str(snapshot.get("head", "")).strip()
    path = validation_receipt_path(head, repo_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "repo_root": repo_root.as_posix(),
        "head": head,
        "summary": summary,
        "commands": commands_list,
        "evidence": build_validation_evidence_entries(commands_list, repo_root),
        "notes": list(notes or []),
        "clean": True,
        "written_at_utc": datetime.now(timezone.utc).isoformat(),
    }
    path.write_text(json.dumps(payload, indent=2, sort_keys=True), encoding="utf-8")
    return path


def summarize_changed_paths(paths: list[str], *, limit: int = 6) -> str:
    if not paths:
        return "none"
    if len(paths) <= limit:
        return ", ".join(paths)
    visible = ", ".join(paths[:limit])
    return f"{visible}, ... (+{len(paths) - limit} more)"


def summarize_open_user_asks(asks: list[str], *, limit: int = 3) -> str:
    if not asks:
        return "none"
    if len(asks) <= limit:
        return " | ".join(asks)
    visible = " | ".join(asks[:limit])
    return f"{visible} | ... (+{len(asks) - limit} more)"


def _load_explicit_user_asks(plan_path: Path) -> list[tuple[str, str]]:
    asks: list[tuple[str, str]] = []
    in_section = False
    for raw_line in plan_path.read_text(encoding="utf-8").splitlines():
        stripped = raw_line.strip()
        if stripped.startswith("## "):
            if in_section:
                break
            in_section = stripped == "## Explicit User Asks"
            continue
        if not in_section or not stripped:
            continue
        match = EXPLICIT_USER_ASK_RE.match(stripped)
        if match is None:
            continue
        asks.append((match.group(1).strip().lower(), match.group(2).strip()))
    return asks


def evaluate_open_explicit_user_asks_guard(
    session_id: str,
    repo_root: Path = REPO_ROOT,
) -> tuple[bool, str, list[str]]:
    contract_state = load_active_contract_state(session_id, repo_root)
    if contract_state is None:
        return False, "", []

    plan_path_text = str(contract_state.get("plan_path", "")).strip()
    if not plan_path_text:
        return False, "", []

    plan_path = repo_root / plan_path_text
    if not plan_path.exists():
        return True, f"Active phased plan is missing: {plan_path_text}", []

    open_asks = [text for status, text in _load_explicit_user_asks(plan_path) if status == "open"]
    if not open_asks:
        return False, "", []

    return (
        True,
        "Active phased plan still has open explicit user asks. Update the plan to mark them done or explicitly defer them before closure.",
        open_asks,
    )


def _contract_requires_hostile_audit(contract_state: dict[str, Any] | None) -> bool:
    if not isinstance(contract_state, dict):
        return False
    commands = contract_state.get("required_validation_commands", [])
    if not isinstance(commands, list):
        return False
    return any(
        isinstance(command, str) and "tools/viewer_host_validate_hostile_audit.py" in command.replace("\\", "/")
        for command in commands
    )


def evaluate_hostile_audit_guard(
    session_id: str,
    repo_root: Path = REPO_ROOT,
) -> tuple[bool, str, dict[str, Any]]:
    contract_state = load_active_contract_state(session_id, repo_root)
    if contract_state is None:
        return False, "", {}

    plan_path_text = str(contract_state.get("plan_path", "")).strip()
    if not plan_path_text:
        return True, "Active phased plan is missing from the locked contract state.", {}

    plan_path = repo_root / plan_path_text
    if not plan_path.exists():
        return True, f"Active phased plan is missing: {plan_path_text}", {}

    if not plan_declares_hostile_audit(plan_path) and not _contract_requires_hostile_audit(contract_state):
        return False, "", {}

    payload = validate_hostile_audit_plan(plan_path)
    if bool(payload.get("ok", False)):
        return False, "", payload

    reason = str(payload.get("blocked_reason", "")).strip() or "Hostile review is incomplete."
    return True, "Hostile review is incomplete: " + reason, payload


def evaluate_validation_receipt_guard(
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    repo_root: Path = REPO_ROOT,
) -> tuple[bool, str]:
    if baseline is None:
        return False, ""

    baseline_head = str(baseline.get("head", "")).strip()
    current_head = str(current.get("head", "")).strip()
    if not baseline_head or not current_head or baseline_head == current_head:
        return False, ""

    receipt = load_validation_receipt(current_head, repo_root)
    if receipt is None:
        return True, (
            "Current HEAD differs from the session baseline and lacks a validation receipt for the committed state. "
            "Run the required validation, then record it with tools/viewer_host_write_validation_receipt.py before completion."
        )

    if str(receipt.get("head", "")).strip() != current_head:
        return True, "Validation receipt head does not match the current committed state."

    return False, ""


def evaluate_contract_proof_receipt_guard(
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    session_id: str,
    repo_root: Path = REPO_ROOT,
) -> tuple[bool, str]:
    if baseline is None:
        return False, ""

    baseline_head = str(baseline.get("head", "")).strip()
    current_head = str(current.get("head", "")).strip()
    if not baseline_head or not current_head or baseline_head == current_head:
        return False, ""

    contract_state, contract_error = validate_locked_contract_state(session_id, repo_root)
    if contract_error:
        return True, contract_error

    contract_path = repo_root / str(contract_state["contract_path"])
    contract_payload, contract_result = load_and_validate_slice_contract(contract_path, repo_root)
    if contract_payload is None or not contract_result.ok:
        return True, "Active slice contract failed validation during closure."

    validation_receipt = load_validation_receipt(current_head, repo_root)
    should_block, reason = evaluate_viewer_first_validation_command_guard(
        None if validation_receipt is None else validation_receipt.get("commands", []),
        str(contract_payload.get("workflow_type", "")).strip(),
    )
    if should_block:
        return True, reason

    receipt = load_contract_proof_receipt(current_head, repo_root)
    if receipt is None:
        return True, (
            "Current HEAD differs from the session baseline and lacks a contract proof receipt for the committed state. "
            "Run the required validators and record proof with tools/viewer_host_checkpoint_slice.py write-receipts before completion."
        )
    if str(receipt.get("head", "")).strip() != current_head:
        return True, "Contract proof receipt head does not match the current committed state."
    if str(receipt.get("contract_id", "")).strip() != str(contract_state.get("contract_id", "")).strip():
        return True, "Contract proof receipt contract_id does not match the active locked contract."
    if str(receipt.get("contract_hash", "")).strip() != str(contract_state.get("contract_hash", "")).strip():
        return True, "Contract proof receipt contract_hash does not match the active locked contract."

    assertion_results = receipt.get("assertion_results")
    if not isinstance(assertion_results, list):
        return True, "Contract proof receipt is missing assertion_results."
    results_by_id: dict[str, dict[str, Any]] = {}
    for item in assertion_results:
        if not isinstance(item, dict):
            continue
        assertion_id = str(item.get("assertion_id", "")).strip()
        if assertion_id:
            results_by_id[assertion_id] = item
    for assertion in contract_payload.get("required_acceptance_assertions", []):
        assertion_id = str(assertion.get("assertion_id", "")).strip()
        result = results_by_id.get(assertion_id)
        if result is None:
            return True, f"Contract proof receipt is missing required assertion result: {assertion_id}"
        if not bool(result.get("ok", False)):
            detail = str(result.get("failure_detail", "")).strip()
            if detail:
                return True, f"Contract proof assertion failed: {assertion_id} ({detail})"
            return True, f"Contract proof assertion failed: {assertion_id}"
    return False, ""


def _contract_drift_reason(session_id: str, repo_root: Path) -> str:
    contract_state = load_active_contract_state(session_id, repo_root)
    if contract_state is None:
        return "No active slice contract is locked. Run viewer_host_prepare_slice.py or begin_work_slice with --plan/--contract before mutation."
    contract_path = repo_root / str(contract_state.get("contract_path", "")).strip()
    if not contract_path.exists():
        return "Active slice contract file is missing: " + contract_state.get("contract_path", "<unknown>")
    current_hash = hash_file(contract_path)
    if current_hash != str(contract_state.get("contract_hash", "")).strip():
        return (
            "Active slice contract changed after it was locked. "
            "Run viewer_host_revise_contract.py before any further mutation or closure."
        )
    return ""


def _evaluate_mutation_guard(payload: dict[str, Any], session_id: str, repo_root: Path) -> tuple[bool, str]:
    tool_name = _extract_payload_tool_name(payload, default="")
    command_text = _extract_payload_command_text(payload)
    if _is_apply_patch_tool(tool_name):
        return True, "Raw apply_patch is forbidden. Use tools/viewer_host_apply_repo_patch.py through shell_command instead."
    if not _is_shell_tool(tool_name):
        return False, ""
    if _is_wrapper_shell_command(command_text):
        normalized = _normalize_text(command_text)
        if "tools\\viewer_host_prepare_slice.py" in normalized or "tools\\viewer_host_revise_contract.py" in normalized:
            return False, ""
        drift_reason = _contract_drift_reason(session_id, repo_root)
        if drift_reason:
            return True, drift_reason
        return False, ""
    if _shell_command_is_forbidden_direct_mutation(command_text):
        return True, (
            "Direct handoff/receipt/git mutation is forbidden. "
            "Use tools/viewer_host_checkpoint_slice.py or the other approved repo wrappers."
        )
    if _is_allowed_raw_shell_command(command_text):
        return False, ""
    return True, (
        "Raw shell mutation is forbidden. Allowed raw shell is read/search/build/test/check only; "
        "all repo mutation must go through the approved viewer_host_* wrappers."
    )


def evaluate_checkpoint_guard(
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
) -> GuardStatus:
    if baseline is None:
        if snapshot_is_clean(current):
            return GuardStatus(False, "", [])
        current_paths = compare_snapshots({"unstaged": {}, "staged": {}, "untracked": {}}, current)
        return GuardStatus(
            True,
            "Checkpoint guard baseline is missing for a dirty repository state; create a checkpoint commit or restore the baseline before completion.",
            current_paths,
        )

    changed_paths = compare_snapshots(baseline, current)
    if not changed_paths:
        return GuardStatus(False, "", [])

    return GuardStatus(
        True,
        "Repository state differs from the session baseline; create a checkpoint commit or restore the baseline before completion.",
        changed_paths,
    )


def build_pretool_response(
    tool_name: str,
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    session_id: str,
    payload: dict[str, Any] | None = None,
    repo_root: Path = REPO_ROOT,
) -> dict[str, Any] | None:
    general_response = build_general_pretool_response(session_id, payload, repo_root)
    hook_payload = general_response["hookSpecificOutput"]
    if hook_payload.get("permissionDecision") == "deny":
        return general_response
    if tool_name != TASK_COMPLETE_TOOL:
        return general_response

    banner = str(hook_payload.get("permissionDecisionReason", "")).strip() or build_strict_banner(load_active_contract_state(session_id, repo_root))

    payload = payload or {}
    status = evaluate_checkpoint_guard(baseline, current)
    if not status.should_block:
        should_block, reason = evaluate_validation_receipt_guard(baseline, current, repo_root)
        if not should_block:
            should_block, reason = evaluate_contract_proof_receipt_guard(baseline, current, session_id, repo_root)
        if not should_block:
            should_block, reason, open_asks = evaluate_open_explicit_user_asks_guard(session_id, repo_root)
        else:
            open_asks = []
        if not should_block:
            should_block, reason, hostile_audit_payload = evaluate_hostile_audit_guard(session_id, repo_root)
        else:
            hostile_audit_payload = {}
        if not should_block:
            return {
                "hookSpecificOutput": {
                    "hookEventName": "PreToolUse",
                    "permissionDecision": "allow",
                    "permissionDecisionReason": banner,
                }
            }
        return {
            "hookSpecificOutput": {
                "hookEventName": "PreToolUse",
                "permissionDecision": "deny",
                "permissionDecisionReason": banner + " " + reason,
                "additionalContext": (
                    (
                        "Open explicit user asks: " + summarize_open_user_asks(open_asks)
                    )
                    if open_asks
                    else (
                        "Hostile audit status: " + str(hostile_audit_payload.get("status", "unknown"))
                        + " | blocked_reason: "
                        + str(hostile_audit_payload.get("blocked_reason", ""))
                    )
                    if hostile_audit_payload
                    else (
                        "Expected receipt path: " + validation_receipt_path(str(current.get("head", "")), repo_root).relative_to(repo_root).as_posix()
                        + " | expected contract proof receipt: "
                        + contract_proof_receipt_path(str(current.get("head", "")), repo_root).relative_to(repo_root).as_posix()
                    )
                ),
            }
        }

    return {
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "deny",
            "permissionDecisionReason": banner + " " + status.reason,
            "additionalContext": (
                "Paths changed vs session baseline: " + summarize_changed_paths(status.changed_paths)
            ),
        }
    }


def build_general_pretool_response(
    session_id: str,
    payload: dict[str, Any] | None = None,
    repo_root: Path = REPO_ROOT,
) -> dict[str, Any]:
    payload = payload or {}
    banner = build_strict_banner(load_active_contract_state(session_id, repo_root))
    mutation_block, mutation_reason = _evaluate_mutation_guard(payload, session_id, repo_root)
    if mutation_block:
        return {
            "hookSpecificOutput": {
                "hookEventName": "PreToolUse",
                "permissionDecision": "deny",
                "permissionDecisionReason": banner + " " + mutation_reason,
            }
        }
    return {
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "allow",
            "permissionDecisionReason": banner,
        }
    }


def build_posttool_response(
    tool_name: str,
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    session_id: str,
    repo_root: Path = REPO_ROOT,
) -> dict[str, Any] | None:
    banner = build_strict_banner(load_active_contract_state(session_id, repo_root))
    status = evaluate_checkpoint_guard(baseline, current)
    if status.should_block:
        changed_summary = summarize_changed_paths(status.changed_paths)
        return {
            "systemMessage": (
                banner
                + " Checkpoint debt after "
                + tool_name
                + ": repository state differs from the session baseline. "
                + status.reason
                + " Changed paths: "
                + changed_summary
                + ". Before any final response, update continuity surfaces and create a checkpoint commit or restore the baseline."
            ),
            "hookSpecificOutput": {
                "hookEventName": "PostToolUse",
            },
        }

    should_block, reason = evaluate_validation_receipt_guard(baseline, current, repo_root)
    if not should_block:
        should_block, reason = evaluate_contract_proof_receipt_guard(baseline, current, session_id, repo_root)
    if not should_block:
        return {
            "systemMessage": banner,
            "hookSpecificOutput": {
                "hookEventName": "PostToolUse",
            },
        }

    return {
        "systemMessage": (
            banner
            + " Validation debt after "
            + tool_name
            + ": "
            + reason
            + " Expected receipt path: "
            + validation_receipt_path(str(current.get("head", "")), repo_root).relative_to(repo_root).as_posix()
            + " | expected contract proof receipt: "
            + contract_proof_receipt_path(str(current.get("head", "")), repo_root).relative_to(repo_root).as_posix()
            + "."
        ),
        "hookSpecificOutput": {
            "hookEventName": "PostToolUse",
        },
    }


def build_stop_response(
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    session_id: str,
    repo_root: Path = REPO_ROOT,
) -> dict[str, Any] | None:
    banner = build_strict_banner(load_active_contract_state(session_id, repo_root))
    status = evaluate_checkpoint_guard(baseline, current)
    if not status.should_block:
        should_block, reason = evaluate_validation_receipt_guard(baseline, current, repo_root)
        if not should_block:
            should_block, reason = evaluate_contract_proof_receipt_guard(baseline, current, session_id, repo_root)
        if not should_block:
            should_block, reason, open_asks = evaluate_open_explicit_user_asks_guard(session_id, repo_root)
        else:
            open_asks = []
        if not should_block:
            should_block, reason, hostile_audit_payload = evaluate_hostile_audit_guard(session_id, repo_root)
        else:
            hostile_audit_payload = {}
        if not should_block:
            return None
        return {
            "hookSpecificOutput": {
                "hookEventName": "Stop",
                "decision": "block",
                "reason": banner + " " + reason + (
                    " Open explicit user asks: " + summarize_open_user_asks(open_asks)
                    if open_asks
                    else " Hostile audit status: " + str(hostile_audit_payload.get("status", "unknown"))
                    if hostile_audit_payload
                    else ""
                ),
            }
        }

    return {
        "hookSpecificOutput": {
            "hookEventName": "Stop",
            "decision": "block",
            "reason": banner + " " + status.reason + " Changed paths: " + summarize_changed_paths(status.changed_paths),
        }
    }


def _sanitize_session_id(session_id: str) -> str:
    safe_chars = []
    for char in session_id:
        if char.isalnum() or char in ("-", "_"):
            safe_chars.append(char)
        else:
            safe_chars.append("_")
    sanitized = "".join(safe_chars).strip("_")
    return sanitized or "unknown_session"


def state_path_for_session(session_id: str, repo_root: Path = REPO_ROOT) -> Path:
    return repo_root / "artifacts" / "hooks" / "viewer_host_checkpoint_guard" / f"{_sanitize_session_id(session_id)}.json"


def write_session_baseline(session_id: str, snapshot: dict[str, Any], repo_root: Path = REPO_ROOT) -> Path:
    path = state_path_for_session(session_id, repo_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(snapshot, indent=2, sort_keys=True), encoding="utf-8")
    return path


def load_session_baseline(session_id: str, repo_root: Path = REPO_ROOT) -> dict[str, Any] | None:
    path = state_path_for_session(session_id, repo_root)
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def recovery_helper_command() -> str:
    return 'py -3.14 tools/viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state'


def resolve_session_baseline(
    session_id: str,
    current: dict[str, Any],
    repo_root: Path = REPO_ROOT,
) -> SessionBaselineResolution:
    baseline = load_session_baseline(session_id, repo_root)
    adoption_payload = load_active_recovery_adoption(repo_root)
    changed_paths = compare_snapshots({"unstaged": {}, "staged": {}, "untracked": {}}, current)
    adoption_path_text = (
        None
        if adoption_payload is None
        else recovery_adoption_path(repo_root).relative_to(repo_root).as_posix()
    )
    report_path_text = None if adoption_payload is None else str(adoption_payload.get("report_path", "")).strip() or None
    adopted_snapshot = _snapshot_without_paths(
        current,
        repo_root=repo_root,
        ignored_relative_paths=[
            report_path_text or "",
            adoption_path_text or "",
        ],
    )
    if baseline is not None:
        if (
            adoption_payload is not None
            and recovery_adoption_matches_snapshot(adoption_payload, current, repo_root)
            and snapshot_digest(baseline) != snapshot_digest(adopted_snapshot)
        ):
            path = write_session_baseline(session_id, adopted_snapshot, repo_root)
            consume_active_recovery_adoption(repo_root)
            return SessionBaselineResolution(
                baseline=adopted_snapshot,
                status="adopted_dirty",
                changed_paths=changed_paths,
                baseline_path=path.relative_to(repo_root).as_posix(),
                recovery_report_path=report_path_text,
                recovery_adoption_path=adoption_path_text,
            )
        if (
            snapshot_is_clean(current)
            and snapshot_digest(baseline) != snapshot_digest(current)
            and clean_head_has_closure_receipts(current, repo_root)
        ):
            path = write_session_baseline(session_id, current, repo_root)
            if adoption_payload is not None:
                consume_active_recovery_adoption(repo_root)
            return SessionBaselineResolution(
                baseline=current,
                status="refreshed_clean",
                changed_paths=[],
                baseline_path=path.relative_to(repo_root).as_posix(),
                recovery_report_path=report_path_text,
                recovery_adoption_path=adoption_path_text,
            )
        return SessionBaselineResolution(
            baseline=baseline,
            status="existing",
            changed_paths=compare_snapshots({"unstaged": {}, "staged": {}, "untracked": {}}, baseline),
            baseline_path=state_path_for_session(session_id, repo_root).relative_to(repo_root).as_posix(),
        )

    if snapshot_is_clean(current):
        path = write_session_baseline(session_id, current, repo_root)
        return SessionBaselineResolution(
            baseline=current,
            status="bootstrapped_clean",
            changed_paths=[],
            baseline_path=path.relative_to(repo_root).as_posix(),
        )

    if adoption_payload is None:
        return SessionBaselineResolution(
            baseline=None,
            status="adoption_required",
            changed_paths=changed_paths,
        )

    if not recovery_adoption_matches_snapshot(adoption_payload, current, repo_root):
        return SessionBaselineResolution(
            baseline=None,
            status="adoption_stale",
            changed_paths=changed_paths,
            recovery_report_path=report_path_text,
            recovery_adoption_path=adoption_path_text,
        )

    path = write_session_baseline(session_id, adopted_snapshot, repo_root)
    consume_active_recovery_adoption(repo_root)
    return SessionBaselineResolution(
        baseline=adopted_snapshot,
        status="adopted_dirty",
        changed_paths=changed_paths,
        baseline_path=path.relative_to(repo_root).as_posix(),
        recovery_report_path=report_path_text,
        recovery_adoption_path=adoption_path_text,
    )


def _bootstrap_missing_baseline_if_clean(
    session_id: str,
    current: dict[str, Any],
    repo_root: Path,
) -> dict[str, Any] | None:
    return resolve_session_baseline(session_id, current, repo_root).baseline


def _session_start_response(session_id: str, repo_root: Path) -> dict[str, Any]:
    snapshot = capture_repo_snapshot(repo_root)
    active_contract = load_active_contract_state(GLOBAL_CONTRACT_SESSION_ID, repo_root)
    resolution = resolve_session_baseline(session_id, snapshot, repo_root)
    if resolution.status in {"existing", "bootstrapped_clean", "adopted_dirty", "refreshed_clean"}:
        detail = (
            build_strict_banner(active_contract)
            + " viewer_host_checkpoint_guard baseline captured at "
            + str(resolution.baseline_path)
            + f" | clean={snapshot_is_clean(snapshot)} | head={snapshot.get('head', '')[:12]}"
        )
        if resolution.status == "adopted_dirty":
            detail += (
                " | crash recovery resumed the stranded dirty slice"
                + (
                    f" | recovery_report={resolution.recovery_report_path}"
                    if resolution.recovery_report_path
                    else ""
                )
            )
        elif resolution.status == "refreshed_clean":
            detail += " | refreshed a stale baseline to the current clean receipted head"
        return {
            "hookSpecificOutput": {
                "hookEventName": "SessionStart",
                "additionalContext": detail,
            }
        }

    detail = (
        build_strict_banner(active_contract)
        + " viewer_host_checkpoint_guard did not capture a dirty-session baseline automatically. "
        + "Run "
        + recovery_helper_command()
        + " and retry once the recovery adoption artifact is in place. "
        + "Changed paths: "
        + summarize_changed_paths(resolution.changed_paths)
    )
    if resolution.status == "adoption_stale":
        detail += " Existing recovery adoption artifact does not match the current dirty snapshot; rerun the recovery helper."
    return {
        "hookSpecificOutput": {
            "hookEventName": "SessionStart",
            "additionalContext": detail,
        }
    }


def _resolve_repo_root(payload: dict[str, Any]) -> Path:
    raw_cwd = payload.get("cwd")
    if not raw_cwd:
        return REPO_ROOT
    return discover_repo_root(Path(str(raw_cwd)))


def main(argv: list[str] | None = None) -> int:
    del argv
    try:
        payload = json.load(sys.stdin)
        event_name = str(payload.get("hookEventName", ""))
        session_id = str(payload.get("sessionId", "unknown_session"))
        repo_root = _resolve_repo_root(payload)

        if event_name == "SessionStart":
            print(json.dumps(_session_start_response(session_id, repo_root)))
            return 0

        if event_name not in ("PreToolUse", "PostToolUse", "Stop"):
            return 0

        if event_name == "PreToolUse":
            response = build_general_pretool_response(session_id, payload, repo_root)
        else:
            current = capture_repo_snapshot(repo_root)
            baseline = _bootstrap_missing_baseline_if_clean(session_id, current, repo_root)
            if event_name == "PostToolUse":
                response = build_posttool_response(_extract_payload_tool_name(payload, default="unknown_tool"), baseline, current, session_id, repo_root)
            else:
                response = build_stop_response(baseline, current, session_id, repo_root)

        if response is not None:
            print(json.dumps(response))
        return 0
    except Exception as exc:
        sys.stderr.write(f"viewer_host_checkpoint_guard: {exc}\n")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
