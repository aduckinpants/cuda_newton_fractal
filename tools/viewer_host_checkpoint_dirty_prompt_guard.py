from __future__ import annotations

from datetime import datetime, timezone
import json
import sys
from pathlib import Path
from typing import Any

try:
    from tools.viewer_host_checkpoint_guard import (
        GLOBAL_CONTRACT_SESSION_ID,
        REPO_ROOT,
        build_strict_banner,
        capture_repo_snapshot,
        contract_proof_receipt_path,
        discover_repo_root,
        evaluate_checkpoint_guard,
        evaluate_inherited_clean_closure_guard,
        evaluate_contract_proof_receipt_guard,
        evaluate_validation_receipt_guard,
        load_active_contract_state,
        recovery_helper_command,
        resolve_session_baseline,
        summarize_changed_paths,
        validation_receipt_path,
    )
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import (
        GLOBAL_CONTRACT_SESSION_ID,
        REPO_ROOT,
        build_strict_banner,
        capture_repo_snapshot,
        contract_proof_receipt_path,
        discover_repo_root,
        evaluate_checkpoint_guard,
        evaluate_inherited_clean_closure_guard,
        evaluate_contract_proof_receipt_guard,
        evaluate_validation_receipt_guard,
        load_active_contract_state,
        recovery_helper_command,
        resolve_session_baseline,
        summarize_changed_paths,
        validation_receipt_path,
    )


def _load_payload() -> Any:
    raw = sys.stdin.read()
    if not raw.strip():
        return {}
    try:
        return json.loads(raw)
    except Exception:
        return {"_raw": raw}


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


def _extract_prompt_text(payload: Any) -> str:
    values: list[str] = []
    for key in ("prompt", "text", "message", "content"):
        values.extend(str(value) for value in _find_values(payload, key) if isinstance(value, (str, int, float)))
    if isinstance(payload, dict) and "_raw" in payload:
        values.append(str(payload.get("_raw") or ""))
    return "\n".join(values)


def build_dirty_prompt_message(changed_paths: list[str], prompt_text: str) -> str:
    prompt_excerpt = " ".join(prompt_text.split())[:240]
    message = (
        "Repo workflow carryover block: this prompt arrived while the repository still differs from the session baseline. "
        "Prompt text is workflow context only; it is not permission to skip closure rules. "
        "That includes tool-generated prompts such as `Start implementation` and steering or reorientation interruptions. "
        "Close the prior slice cleanly before treating this as unrelated new work. "
        "Required flow: update the active phased plan if needed, append HANDOFF_LOG.md with the checked-in helper, run the relevant validation rails, commit the slice, and if HEAD advanced write a validation receipt. "
        "Changed paths: "
        + summarize_changed_paths(changed_paths)
        + "."
    )
    if prompt_excerpt:
        message += " Prompt excerpt: " + prompt_excerpt
    return message


def carryover_state_path(repo_root: Path = REPO_ROOT) -> Path:
    return repo_root / "artifacts" / "hooks" / "viewer_host_checkpoint_guard" / "checkpoint_carryover_state.json"


def load_carryover_state(repo_root: Path = REPO_ROOT) -> dict[str, Any]:
    path = carryover_state_path(repo_root)
    if not path.exists():
        return {}
    try:
        payload = json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return {}
    return payload if isinstance(payload, dict) else {}


def clear_carryover_state(repo_root: Path = REPO_ROOT) -> None:
    path = carryover_state_path(repo_root)
    try:
        path.unlink()
    except FileNotFoundError:
        pass


def write_carryover_state(
    changed_paths: list[str],
    prompt_text: str,
    *,
    reason: str,
    repo_root: Path = REPO_ROOT,
) -> Path:
    path = carryover_state_path(repo_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "active": True,
        "changed_paths": list(changed_paths),
        "changed_summary": summarize_changed_paths(changed_paths),
        "prompt_excerpt": " ".join(prompt_text.split())[:240],
        "reason": reason,
        "written_at_utc": datetime.now(timezone.utc).isoformat(),
    }
    path.write_text(json.dumps(payload, indent=2, sort_keys=True), encoding="utf-8")
    return path


def build_validation_receipt_prompt_message(prompt_text: str, repo_root: Path, head: str) -> str:
    prompt_excerpt = " ".join(prompt_text.split())[:240]
    message = (
        "Repo workflow carryover block: this prompt arrived after the session advanced HEAD, but the current committed state still lacks a validation receipt. "
        "Prompt text is workflow context only; it does not override closure discipline. "
        "That includes tool-generated prompts such as `Start implementation` and steering or reorientation interruptions. "
        "Finish the prior slice first by recording the final validation evidence with `py -3.14 tools/viewer_host_write_validation_receipt.py ...`. "
        "Expected receipt path: "
        + validation_receipt_path(head, repo_root).relative_to(repo_root).as_posix()
        + "."
    )
    if prompt_excerpt:
        message += " Prompt excerpt: " + prompt_excerpt
    return message


def build_contract_proof_prompt_message(prompt_text: str, repo_root: Path, head: str, reason: str) -> str:
    prompt_excerpt = " ".join(prompt_text.split())[:240]
    message = (
        "Repo workflow carryover block: this prompt arrived after the session advanced HEAD, but the current committed state still fails the contract-proof closure guard. "
        + reason.strip()
        + " Expected contract proof receipt path: "
        + contract_proof_receipt_path(head, repo_root).relative_to(repo_root).as_posix()
        + "."
    )
    if prompt_excerpt:
        message += " Prompt excerpt: " + prompt_excerpt
    return message


def build_recovery_required_prompt_message(changed_paths: list[str], prompt_text: str) -> str:
    prompt_excerpt = " ".join(prompt_text.split())[:240]
    message = (
        "Crash recovery required: the repository is dirty, but this session has no checkpoint baseline. "
        "This usually means VS Code crashed, the host rolled back, or the chat session changed mid-slice. "
        "Do not treat this as a fresh session. Run "
        + recovery_helper_command()
        + ", inspect `artifacts/hooks/viewer_host_checkpoint_guard/recovery/`, and retry the prompt only after explicit recovery adoption. "
        "Changed paths: "
        + summarize_changed_paths(changed_paths)
        + "."
    )
    if prompt_excerpt:
        message += " Prompt excerpt: " + prompt_excerpt
    return message


def build_recovery_mismatch_prompt_message(changed_paths: list[str], prompt_text: str) -> str:
    prompt_excerpt = " ".join(prompt_text.split())[:240]
    message = (
        "Crash recovery required: a recovery adoption artifact exists, but it no longer matches the current dirty snapshot. "
        "Rerun "
        + recovery_helper_command()
        + " so the repo records the current stranded state before resuming. "
        "Changed paths: "
        + summarize_changed_paths(changed_paths)
        + "."
    )
    if prompt_excerpt:
        message += " Prompt excerpt: " + prompt_excerpt
    return message


def build_recovery_resumed_prompt_message(prompt_text: str, report_path: str | None) -> str:
    prompt_excerpt = " ".join(prompt_text.split())[:240]
    message = (
        "Crash recovery resumed this stranded dirty slice through explicit recovery adoption. "
        "Continue the existing slice and close its checkpoint debt before treating any new prompt as unrelated work."
    )
    if report_path:
        message += " Recovery report: " + report_path + "."
    if prompt_excerpt:
        message += " Prompt excerpt: " + prompt_excerpt
    return message


def build_userprompt_response(
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    prompt_text: str,
    session_id: str,
    repo_root: Path = REPO_ROOT,
    *,
    baseline_resolution: Any | None = None,
) -> dict[str, Any] | None:
    banner = build_strict_banner(load_active_contract_state(session_id, repo_root))
    resolution_status = "" if baseline_resolution is None else str(getattr(baseline_resolution, "status", ""))
    if resolution_status == "adopted_dirty":
        return {
            "continue": True,
            "systemMessage": banner + " " + build_recovery_resumed_prompt_message(
                prompt_text,
                str(getattr(baseline_resolution, "recovery_report_path", "")).strip() or None,
            ),
        }

    if resolution_status in {"clean_validation_receipt_required", "clean_contract_proof_required"}:
        should_block, reason = evaluate_inherited_clean_closure_guard(baseline, current, session_id, repo_root)
        if should_block:
            head = str(current.get("head", "")).strip()
            if "contract proof" in reason.lower() or "contract_id" in reason.lower() or "contract_hash" in reason.lower():
                return {
                    "continue": False,
                    "systemMessage": banner + " " + build_contract_proof_prompt_message(prompt_text, repo_root, head, reason),
                }
            return {
                "continue": False,
                "systemMessage": banner + " " + build_validation_receipt_prompt_message(prompt_text, repo_root, head),
            }

    if baseline is None and not bool(current.get("clean", False)):
        changed_paths = (
            evaluate_checkpoint_guard(None, current).changed_paths
            if baseline_resolution is None
            else list(getattr(baseline_resolution, "changed_paths", []))
        )
        if resolution_status == "adoption_stale":
            return {
                "continue": False,
                "systemMessage": banner + " " + build_recovery_mismatch_prompt_message(changed_paths, prompt_text),
            }
        return {
            "continue": False,
            "systemMessage": banner + " " + build_recovery_required_prompt_message(changed_paths, prompt_text),
        }

    status = evaluate_checkpoint_guard(baseline, current)
    if status.should_block:
        return {
            "continue": False,
            "systemMessage": banner + " " + build_dirty_prompt_message(status.changed_paths, prompt_text),
        }

    should_block, reason = evaluate_inherited_clean_closure_guard(baseline, current, session_id, repo_root)
    if not should_block:
        should_block, reason = evaluate_validation_receipt_guard(baseline, current, repo_root)
    if not should_block:
        should_block, reason = evaluate_contract_proof_receipt_guard(baseline, current, session_id, repo_root)
    if should_block:
        head = str(current.get("head", "")).strip()
        if "contract proof" in reason.lower() or "contract_id" in reason.lower() or "contract_hash" in reason.lower():
            return {
                "continue": False,
                "systemMessage": banner + " " + build_contract_proof_prompt_message(prompt_text, repo_root, head, reason),
            }
        return {
            "continue": False,
            "systemMessage": banner + " " + build_validation_receipt_prompt_message(prompt_text, repo_root, head),
        }
    return {"continue": True, "systemMessage": banner}


def _resolve_repo_root(payload: dict[str, Any]) -> Path:
    raw_cwd = payload.get("cwd") if isinstance(payload, dict) else None
    if not raw_cwd:
        return REPO_ROOT
    return discover_repo_root(Path(str(raw_cwd)))


def main() -> int:
    try:
        payload = _load_payload()
        session_id = str(payload.get("sessionId", "unknown_session")) if isinstance(payload, dict) else "unknown_session"
        repo_root = _resolve_repo_root(payload if isinstance(payload, dict) else {})
        current = capture_repo_snapshot(repo_root)
        baseline_resolution = resolve_session_baseline(session_id, current, repo_root)
        prompt_text = _extract_prompt_text(payload)
        response = build_userprompt_response(
            baseline_resolution.baseline,
            current,
            prompt_text,
            session_id,
            repo_root,
            baseline_resolution=baseline_resolution,
        )
        resolution_status = str(getattr(baseline_resolution, "status", "")).strip()
        if bool(response.get("continue", True)):
            clear_carryover_state(repo_root)
        elif not bool(current.get("clean", False)) and resolution_status != "adopted_dirty":
            changed_paths = list(getattr(baseline_resolution, "changed_paths", []))
            if not changed_paths:
                changed_paths = evaluate_checkpoint_guard(baseline_resolution.baseline, current).changed_paths
            write_carryover_state(
                changed_paths,
                prompt_text,
                reason=resolution_status or "dirty_carryover",
                repo_root=repo_root,
            )
        else:
            clear_carryover_state(repo_root)
        print(json.dumps(response))
        return 0
    except Exception as exc:
        sys.stderr.write(f"viewer_host_checkpoint_dirty_prompt_guard: {exc}\n")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
