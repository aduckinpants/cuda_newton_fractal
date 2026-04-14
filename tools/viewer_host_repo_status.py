from __future__ import annotations

import argparse
import subprocess
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


def _run_git(repo_root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["git", *args],
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )


def _git_lines(repo_root: Path, *args: str) -> list[str]:
    proc = _run_git(repo_root, *args)
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout or "git command failed").strip()
        raise RuntimeError(detail)
    return sorted(
        {
            Path(line.strip()).as_posix()
            for line in (proc.stdout or "").splitlines()
            if line.strip()
        }
    )


def capture_repo_status(repo_root: Path = REPO_ROOT) -> dict[str, list[str]]:
    repo_root = repo_root.resolve()
    return {
        "staged": _git_lines(repo_root, "diff", "--cached", "--name-only", "--diff-filter=ACDMRTUXB", "--relative"),
        "unstaged": _git_lines(repo_root, "diff", "--name-only", "--diff-filter=ACDMRTUXB", "--relative"),
        "untracked": _git_lines(repo_root, "ls-files", "--others", "--exclude-standard"),
    }


def repo_is_dirty(repo_root: Path = REPO_ROOT) -> bool:
    status = capture_repo_status(repo_root)
    return any(status[key] for key in ("staged", "unstaged", "untracked"))


def format_repo_status_summary(status: dict[str, list[str]], *, prefix: str = "viewer_host_repo_status") -> str:
    def format_paths(paths: list[str]) -> str:
        return ", ".join(paths) if paths else "none"

    return (
        prefix
        + ": repo-status staged="
        + format_paths(status.get("staged", []))
        + " | unstaged="
        + format_paths(status.get("unstaged", []))
        + " | untracked="
        + format_paths(status.get("untracked", []))
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Print deterministic repo status using git diff + git ls-files instead of plain git status.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--repo-root", default=str(REPO_ROOT), help="Repository root to inspect")
    args = parser.parse_args(argv)

    repo_root = Path(args.repo_root)
    if not repo_root.is_absolute():
        repo_root = (REPO_ROOT / repo_root).resolve()

    print(format_repo_status_summary(capture_repo_status(repo_root)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())