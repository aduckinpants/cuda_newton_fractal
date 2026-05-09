from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

try:
    import tools.viewer_host_checkpoint_guard as checkpoint_guard
    from tools.viewer_host_checkpoint_dirty_prompt_guard import clear_carryover_state, load_carryover_state
except ModuleNotFoundError:
    import viewer_host_checkpoint_guard as checkpoint_guard
    from viewer_host_checkpoint_dirty_prompt_guard import clear_carryover_state, load_carryover_state


_READ_ONLY_TOOL_TOKENS = (
    "functions.read_file",
    "functions.grep_search",
    "functions.file_search",
    "functions.semantic_search",
    "functions.list_dir",
    "functions.get_changed_files",
    "functions.get_errors",
)
_ALLOWED_COMMAND_SNIPPETS = (
    "tools\\viewer_host_recover_crash_state.py",
    "tools\\viewer_host_repo_status.py",
    "tools\\viewer_host_validate_slice_contract.py",
    "tools\\viewer_host_assert_phased_plan_sync.py",
    "tools\\viewer_host_validate_hostile_audit.py",
    "tools\\viewer_host_checkpoint_slice.py",
    "tools\\viewer_host_run_logged_command.py",
    "tools\\code_quality_audit.py",
    "py -3.14 -m pytest",
    "tools\\viewer_host_runtime_pytest_lane.py",
    "git status",
    "git diff",
    "git rev-parse",
    "ui_app\\build_tests_vsdevcmd.cmd",
    "ui_app\\build_vsdevcmd.cmd",
)
_ALLOWED_TASK_SNIPPETS = (
    "verify:",
    "agent: assert phased plan sync",
)


def _load_payload() -> dict[str, Any]:
    raw = sys.stdin.read()
    if not raw.strip():
        return {}
    try:
        payload = json.loads(raw)
    except Exception:
        return {}
    return payload if isinstance(payload, dict) else {}


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


def _tool_haystack(payload: dict[str, Any]) -> str:
    values: list[str] = []
    for key in ("toolName", "tool_name", "tool", "name", "recipient_name", "recipientName"):
        values.extend(str(value) for value in _find_values(payload, key) if isinstance(value, (str, int, float)))
    return " ".join(values).lower()


def _resolve_repo_root(payload: dict[str, Any]) -> Path:
    raw_cwd = payload.get("cwd")
    if not raw_cwd:
        return checkpoint_guard.REPO_ROOT
    return checkpoint_guard.discover_repo_root(Path(str(raw_cwd)))


def _extract_command_text(payload: dict[str, Any]) -> str:
    values = [str(value) for value in _find_values(payload, "command") if isinstance(value, (str, int, float))]
    return "\n".join(values).lower()


def _extract_task_ids(payload: dict[str, Any]) -> list[str]:
    return [str(value).lower() for value in _find_values(payload, "id") if isinstance(value, (str, int, float))]


def _is_read_only_tool(payload: dict[str, Any]) -> bool:
    haystack = _tool_haystack(payload)
    if any(token in haystack for token in _READ_ONLY_TOOL_TOKENS):
        return True
    if "functions.memory" in haystack:
        commands = " ".join(str(value).lower() for value in _find_values(payload, "command") if isinstance(value, (str, int, float)))
        return "view" in commands
    return False


def _is_allowed_terminal_command(payload: dict[str, Any]) -> bool:
    haystack = _tool_haystack(payload)
    if "functions.run_in_terminal" not in haystack and "functions.send_to_terminal" not in haystack:
        return False
    command_text = _extract_command_text(payload)
    return any(token in command_text for token in _ALLOWED_COMMAND_SNIPPETS)


def _is_allowed_task(payload: dict[str, Any]) -> bool:
    haystack = _tool_haystack(payload)
    if "functions.run_task" not in haystack:
        return False
    task_ids = " ".join(_extract_task_ids(payload))
    return any(token in task_ids for token in _ALLOWED_TASK_SNIPPETS)


def _allow() -> None:
    print(
        json.dumps(
            {
                "hookSpecificOutput": {
                    "hookEventName": "PreToolUse",
                    "permissionDecision": "allow",
                }
            }
        )
    )


def _deny(reason: str) -> None:
    print(
        json.dumps(
            {
                "hookSpecificOutput": {
                    "hookEventName": "PreToolUse",
                    "permissionDecision": "deny",
                    "permissionDecisionReason": reason,
                },
                "systemMessage": reason,
            }
        )
    )


def build_carryover_block_message(state: dict[str, Any], session_id: str, repo_root: Path) -> str:
    banner = checkpoint_guard.build_strict_banner(checkpoint_guard.load_active_contract_state(session_id, repo_root))
    changed_paths = list(state.get("changed_paths", []))
    prompt_excerpt = str(state.get("prompt_excerpt", "")).strip()
    message = (
        banner
        + " Carryover dirty-worktree guard: the prior prompt arrived while the repository still differed from the session baseline. "
        + "Only read-only inspection and closure-repair commands are allowed until the repo is clean or recovery is explicitly adopted. "
        + "Changed paths: "
        + checkpoint_guard.summarize_changed_paths(changed_paths)
        + "."
    )
    if prompt_excerpt:
        message += " Prompt excerpt: " + prompt_excerpt
    return message


def main() -> int:
    try:
        payload = _load_payload()
        session_id = str(payload.get("sessionId", "unknown_session"))
        repo_root = _resolve_repo_root(payload)
        state = load_carryover_state(repo_root)
        if not state.get("active"):
            _allow()
            return 0

        current = checkpoint_guard.capture_repo_snapshot(repo_root)
        if bool(current.get("clean", False)):
            clear_carryover_state(repo_root)
            _allow()
            return 0

        if _is_read_only_tool(payload) or _is_allowed_terminal_command(payload) or _is_allowed_task(payload):
            _allow()
            return 0

        _deny(build_carryover_block_message(state, session_id, repo_root))
        return 2
    except Exception as exc:
        sys.stderr.write(f"viewer_host_hook_require_checkpoint_carryover: {exc}\n")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())