from __future__ import annotations

import argparse
import subprocess
import sys
import tempfile
from pathlib import Path

try:
    from tools.viewer_host_checkpoint_guard import discover_repo_root
    from tools.viewer_host_contract_state import file_path_is_in_contract_scope, validate_locked_contract_state
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import discover_repo_root
    from viewer_host_contract_state import file_path_is_in_contract_scope, validate_locked_contract_state


def _normalize_patch_target(raw_target: str) -> str | None:
    target = raw_target.strip()
    if len(target) >= 2 and target[0] == target[-1] == '"':
        target = target[1:-1]
    target = target.replace("\\", "/")
    while "//" in target:
        target = target.replace("//", "/")
    if target == "/dev/null":
        return None
    if target.startswith("a/") or target.startswith("b/"):
        target = target[2:]
    return target.strip("/") or None


def _extract_patch_targets(patch_text: str) -> list[str]:
    targets: list[str] = []
    for line in patch_text.splitlines():
        if line.startswith("+++") or line.startswith("---"):
            parts = line.split(maxsplit=1)
            if len(parts) != 2 or parts[0] not in {"+++", "---"}:
                continue
            target = _normalize_patch_target(parts[1])
            if target is not None:
                targets.append(target)
    return sorted(set(targets))


def _patch_text_for_git_apply(patch_text: str) -> str:
    return patch_text.replace("\r\n", "\n").replace("\r", "\n")


def _write_git_apply_patch_file(patch_text: str) -> Path:
    handle = tempfile.NamedTemporaryFile(
        "w",
        encoding="utf-8",
        newline="\n",
        delete=False,
        suffix=".patch",
    )
    with handle:
        handle.write(_patch_text_for_git_apply(patch_text))
    return Path(handle.name)


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

    normalized_patch_path = _write_git_apply_patch_file(patch_text)
    try:
        proc = subprocess.run(["git", "apply", str(normalized_patch_path)], cwd=str(repo_root), check=False)
        return int(proc.returncode)
    finally:
        normalized_patch_path.unlink(missing_ok=True)


if __name__ == "__main__":
    raise SystemExit(main())
