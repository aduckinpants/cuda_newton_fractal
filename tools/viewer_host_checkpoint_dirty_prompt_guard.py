from __future__ import annotations

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
        discover_repo_root,
        evaluate_checkpoint_guard,
        evaluate_contract_proof_receipt_guard,
        evaluate_validation_receipt_guard,
        load_active_contract_state,
        load_session_baseline,
        summarize_changed_paths,
        validation_receipt_path,
    )
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import (
        GLOBAL_CONTRACT_SESSION_ID,
        REPO_ROOT,
        build_strict_banner,
        capture_repo_snapshot,
        discover_repo_root,
        evaluate_checkpoint_guard,
        evaluate_contract_proof_receipt_guard,
        evaluate_validation_receipt_guard,
        load_active_contract_state,
        load_session_baseline,
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


def build_userprompt_response(
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    prompt_text: str,
    session_id: str,
    repo_root: Path = REPO_ROOT,
) -> dict[str, Any] | None:
    banner = build_strict_banner(load_active_contract_state(session_id, repo_root))
    status = evaluate_checkpoint_guard(baseline, current)
    if status.should_block:
        return {
            "continue": False,
            "systemMessage": banner + " " + build_dirty_prompt_message(status.changed_paths, prompt_text),
        }

    should_block, _reason = evaluate_validation_receipt_guard(baseline, current, repo_root)
    if not should_block:
        should_block, _reason = evaluate_contract_proof_receipt_guard(baseline, current, session_id, repo_root)
    if should_block:
        return {
            "continue": False,
            "systemMessage": banner + " " + build_validation_receipt_prompt_message(prompt_text, repo_root, str(current.get("head", "")).strip()),
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
        baseline = load_session_baseline(session_id, repo_root)
        response = build_userprompt_response(baseline, current, _extract_prompt_text(payload), session_id, repo_root)
        print(json.dumps(response))
        return 0
    except Exception as exc:
        sys.stderr.write(f"viewer_host_checkpoint_dirty_prompt_guard: {exc}\n")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
