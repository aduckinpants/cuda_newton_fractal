from __future__ import annotations

import json
import sys
from pathlib import Path
from typing import Any

try:
    import tools.viewer_host_checkpoint_guard as checkpoint_guard
except ModuleNotFoundError:
    import viewer_host_checkpoint_guard as checkpoint_guard


def _load_payload() -> dict[str, Any]:
    raw = sys.stdin.read()
    if not raw.strip():
        return {}
    try:
        payload = json.loads(raw)
    except Exception:
        return {}
    return payload if isinstance(payload, dict) else {}


def _resolve_repo_root(payload: dict[str, Any]) -> Path:
    raw_cwd = payload.get("cwd")
    if not raw_cwd:
        return checkpoint_guard.REPO_ROOT
    return checkpoint_guard.discover_repo_root(Path(str(raw_cwd)))


def _allow() -> None:
    print(json.dumps({"continue": True}))


def main() -> int:
    try:
        payload = _load_payload()
        session_id = str(payload.get("sessionId", "unknown_session"))
        repo_root = _resolve_repo_root(payload)
        current = checkpoint_guard.capture_repo_snapshot(repo_root)
        baseline = checkpoint_guard._bootstrap_missing_baseline_if_clean(session_id, current, repo_root)
        response = checkpoint_guard.build_stop_response(baseline, current, session_id, repo_root)
        if response is None:
            _allow()
        else:
            print(json.dumps(response))
        return 0
    except Exception as exc:
        sys.stderr.write(f"viewer_host_hook_stop_if_dirty_worktree: {exc}\n")
        return 2


if __name__ == "__main__":
    raise SystemExit(main())