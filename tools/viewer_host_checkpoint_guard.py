from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from datetime import datetime, timezone
from dataclasses import dataclass
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
STATE_DIR = REPO_ROOT / "artifacts" / "hooks" / "viewer_host_checkpoint_guard"
RECEIPT_DIR = REPO_ROOT / "artifacts" / "hooks" / "viewer_host_validation_receipts"
TASK_COMPLETE_TOOL = "task_complete"
DELETED_MARKER = "__DELETED__"


@dataclass(frozen=True)
class GuardStatus:
    should_block: bool
    reason: str
    changed_paths: list[str]


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

    head = str(snapshot.get("head", "")).strip()
    path = validation_receipt_path(head, repo_root)
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "repo_root": repo_root.as_posix(),
        "head": head,
        "summary": summary,
        "commands": list(commands or []),
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
    repo_root: Path = REPO_ROOT,
) -> dict[str, Any] | None:
    if tool_name != TASK_COMPLETE_TOOL:
        return None

    status = evaluate_checkpoint_guard(baseline, current)
    if not status.should_block:
        should_block, reason = evaluate_validation_receipt_guard(baseline, current, repo_root)
        if not should_block:
            return None
        return {
            "hookSpecificOutput": {
                "hookEventName": "PreToolUse",
                "permissionDecision": "deny",
                "permissionDecisionReason": reason,
                "additionalContext": (
                    "Expected receipt path: " + validation_receipt_path(str(current.get("head", "")), repo_root).relative_to(repo_root).as_posix()
                ),
            }
        }

    return {
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "deny",
            "permissionDecisionReason": status.reason,
            "additionalContext": (
                "Paths changed vs session baseline: " + summarize_changed_paths(status.changed_paths)
            ),
        }
    }


def build_posttool_response(
    tool_name: str,
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    repo_root: Path = REPO_ROOT,
) -> dict[str, Any] | None:
    status = evaluate_checkpoint_guard(baseline, current)
    if status.should_block:
        changed_summary = summarize_changed_paths(status.changed_paths)
        return {
            "systemMessage": (
                "Checkpoint debt after "
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
        return None

    return {
        "systemMessage": (
            "Validation debt after "
            + tool_name
            + ": "
            + reason
            + " Expected receipt path: "
            + validation_receipt_path(str(current.get("head", "")), repo_root).relative_to(repo_root).as_posix()
            + "."
        ),
        "hookSpecificOutput": {
            "hookEventName": "PostToolUse",
        },
    }


def build_stop_response(
    baseline: dict[str, Any] | None,
    current: dict[str, Any],
    repo_root: Path = REPO_ROOT,
) -> dict[str, Any] | None:
    status = evaluate_checkpoint_guard(baseline, current)
    if not status.should_block:
        should_block, reason = evaluate_validation_receipt_guard(baseline, current, repo_root)
        if not should_block:
            return None
        return {
            "hookSpecificOutput": {
                "hookEventName": "Stop",
                "decision": "block",
                "reason": reason,
            }
        }

    return {
        "hookSpecificOutput": {
            "hookEventName": "Stop",
            "decision": "block",
            "reason": status.reason + " Changed paths: " + summarize_changed_paths(status.changed_paths),
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


def _bootstrap_missing_baseline_if_clean(
    session_id: str,
    current: dict[str, Any],
    repo_root: Path,
) -> dict[str, Any] | None:
    baseline = load_session_baseline(session_id, repo_root)
    if baseline is not None:
        return baseline
    if not snapshot_is_clean(current):
        return None
    write_session_baseline(session_id, current, repo_root)
    return current


def _session_start_response(session_id: str, repo_root: Path) -> dict[str, Any]:
    snapshot = capture_repo_snapshot(repo_root)
    path = write_session_baseline(session_id, snapshot, repo_root)
    return {
        "hookSpecificOutput": {
            "hookEventName": "SessionStart",
            "additionalContext": (
                "viewer_host_checkpoint_guard baseline captured at "
                + path.relative_to(repo_root).as_posix()
                + f" | clean={snapshot_is_clean(snapshot)} | head={snapshot.get('head', '')[:12]}"
            ),
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

        if event_name == "PreToolUse" and str(payload.get("tool_name", "")) != TASK_COMPLETE_TOOL:
            return 0

        current = capture_repo_snapshot(repo_root)
        baseline = _bootstrap_missing_baseline_if_clean(session_id, current, repo_root)

        if event_name == "PreToolUse":
            response = build_pretool_response(str(payload.get("tool_name", "")), baseline, current, repo_root)
        elif event_name == "PostToolUse":
            response = build_posttool_response(str(payload.get("tool_name", "unknown_tool")), baseline, current, repo_root)
        else:
            response = build_stop_response(baseline, current, repo_root)

        if response is not None:
            print(json.dumps(response))
        return 0
    except Exception as exc:
        sys.stderr.write(f"viewer_host_checkpoint_guard: {exc}\n")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())