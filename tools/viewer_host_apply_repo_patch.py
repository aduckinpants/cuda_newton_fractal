from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

try:
    from tools.viewer_host_checkpoint_guard import discover_repo_root
    from tools.viewer_host_contract_state import file_path_is_in_contract_scope, validate_locked_contract_state
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import discover_repo_root
    from viewer_host_contract_state import file_path_is_in_contract_scope, validate_locked_contract_state


def _extract_patch_targets(patch_text: str) -> list[str]:
    targets: list[str] = []
    for line in patch_text.splitlines():
        if line.startswith("+++ b/"):
            targets.append(line.removeprefix("+++ b/").strip())
        elif line.startswith("--- a/"):
            targets.append(line.removeprefix("--- a/").strip())
    return sorted(set(targets))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Apply a unified diff patch only when it stays inside the active contract scope")
    parser.add_argument("--session-id", required=True, help="Current host session id")
    parser.add_argument("--cwd", default=".", help="Repo cwd")
    parser.add_argument("--patch-file", required=True, help="Unified diff patch file to apply with git apply")
    args = parser.parse_args(argv)

    repo_root = discover_repo_root(Path(args.cwd))
    contract_state, contract_error = validate_locked_contract_state(args.session_id, repo_root)
    if contract_error:
        sys.stderr.write(f"viewer_host_apply_repo_patch: {contract_error}\n")
        return 2

    patch_path = Path(args.patch_file)
    if not patch_path.is_absolute():
        patch_path = repo_root / patch_path
    if not patch_path.exists():
        sys.stderr.write(f"viewer_host_apply_repo_patch: missing patch file: {patch_path}\n")
        return 2

    patch_text = patch_path.read_text(encoding="utf-8")
    targets = _extract_patch_targets(patch_text)
    if not targets:
        sys.stderr.write("viewer_host_apply_repo_patch: patch contains no file targets\n")
        return 2
    for target in targets:
        if not file_path_is_in_contract_scope(target, contract_state, repo_root):
            sys.stderr.write(f"viewer_host_apply_repo_patch: patch target outside contract scope: {target}\n")
            return 2

    proc = subprocess.run(["git", "apply", str(patch_path)], cwd=str(repo_root), check=False)
    return int(proc.returncode)


if __name__ == "__main__":
    raise SystemExit(main())
