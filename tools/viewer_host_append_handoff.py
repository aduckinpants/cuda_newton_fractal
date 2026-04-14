from __future__ import annotations

import argparse
import subprocess
import sys
import uuid
from pathlib import Path

try:
    from tools.viewer_host_repo_status import capture_repo_status, format_repo_status_summary
except ModuleNotFoundError:
    from viewer_host_repo_status import capture_repo_status, format_repo_status_summary


REPO_ROOT = Path(__file__).resolve().parents[1]
MAINLINE_HANDOFF_APPEND = Path(r"C:\code\salticid-cuda\tools\handoff_append.py")


def _base_handoff_command(*, py: str, repo_root: Path, commit: str | None) -> list[str]:
    if not MAINLINE_HANDOFF_APPEND.exists():
        raise FileNotFoundError(f"Mainline handoff helper not found: {MAINLINE_HANDOFF_APPEND}")

    command = [py, str(MAINLINE_HANDOFF_APPEND), "--repo-root", str(repo_root)]
    if commit:
        command.extend(["--commit", commit])
    return command


def resolve_handoff_checkpoint_token(
    *,
    commit: str | None,
    message: str | None,
) -> tuple[str | None, bool]:
    explicit = (commit or "").strip()
    if explicit:
        return explicit, False
    if message is not None:
        return f"ck:{uuid.uuid4().hex[:8]}", True
    return None, False


def build_handoff_append_commands(
    *,
    py: str,
    message: str | None,
    commit: str | None,
    resolve_last_pending: bool,
    score: int | None = None,
    auto_score: bool = False,
    no_auto_rotate: bool = False,
    repo_root: Path = REPO_ROOT,
) -> list[list[str]]:
    if not message and not resolve_last_pending:
        raise ValueError("viewer_host_append_handoff requires a message, --resolve-last-pending, or both")

    commit_token, _generated = resolve_handoff_checkpoint_token(commit=commit, message=message)
    commands: list[list[str]] = []
    if resolve_last_pending:
        resolve_cmd = _base_handoff_command(py=py, repo_root=repo_root, commit=commit_token)
        resolve_cmd.append("--resolve-last-pending")
        commands.append(resolve_cmd)

    if message is not None:
        append_cmd = _base_handoff_command(py=py, repo_root=repo_root, commit=commit_token)
        if score is not None:
            append_cmd.extend(["--score", str(score)])
        if auto_score:
            append_cmd.append("--auto-score")
        if no_auto_rotate:
            append_cmd.append("--no-auto-rotate")
        append_cmd.append(message)
        commands.append(append_cmd)

    return commands


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Repo-local handoff wrapper that can resolve the pending breadcrumb and append the final checkpoint entry in one invocation.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("message", nargs="?", help="Checkpoint message to append after any pending-resolution step")
    parser.add_argument("--commit", help="Explicit checkpoint token or legacy commit hash to record; omit for the normal generated checkpoint-id flow")
    parser.add_argument("--score", type=int, help="Optional code-quality score to attach to the appended entry")
    parser.add_argument("--resolve-last-pending", action="store_true", help="Resolve the most recent pending breadcrumb before appending the message")
    parser.add_argument("--auto-score", action="store_true", help="Pass through to the mainline helper for slow audit-based score capture")
    parser.add_argument("--no-auto-rotate", action="store_true", help="Pass through to the mainline helper to disable automatic log rotation")
    parser.add_argument("--dry-run", action="store_true", help="Print the delegated commands without executing them")
    args = parser.parse_args(argv)

    try:
        commit_token, generated_token = resolve_handoff_checkpoint_token(commit=args.commit, message=args.message)
        commands = build_handoff_append_commands(
            py=sys.executable,
            message=args.message,
            commit=commit_token,
            resolve_last_pending=args.resolve_last_pending,
            score=args.score,
            auto_score=args.auto_score,
            no_auto_rotate=args.no_auto_rotate,
        )
    except (FileNotFoundError, ValueError) as exc:
        parser.error(str(exc))

    if args.dry_run:
        if generated_token and commit_token:
            print(f"viewer_host_append_handoff: checkpoint_id={commit_token}")
            print("viewer_host_append_handoff: include this token in the upcoming commit message to avoid a follow-up continuity commit")
        for command in commands:
            print(subprocess.list2cmdline(command))
        return 0

    for command in commands:
        proc = subprocess.run(command, cwd=str(REPO_ROOT), check=False)
        if proc.returncode != 0:
            return int(proc.returncode)

    if generated_token and commit_token:
        print(f"viewer_host_append_handoff: checkpoint_id={commit_token}")
        print("viewer_host_append_handoff: include this token in the upcoming commit message to avoid a follow-up continuity commit")

    try:
        print(format_repo_status_summary(capture_repo_status(REPO_ROOT), prefix="viewer_host_append_handoff"))
    except RuntimeError as exc:
        print(f"viewer_host_append_handoff: repo-status unavailable: {exc}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())