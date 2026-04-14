from __future__ import annotations

import argparse
import importlib.util
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

try:
    from tools.viewer_host_append_handoff import build_handoff_append_commands, resolve_handoff_checkpoint_token
    from tools.viewer_host_repo_status import repo_is_dirty
except ModuleNotFoundError:
    from viewer_host_append_handoff import build_handoff_append_commands, resolve_handoff_checkpoint_token
    from viewer_host_repo_status import repo_is_dirty


REPO_ROOT = Path(__file__).resolve().parents[1]
MAINLINE_AGENT_BEGIN = Path(r"C:\code\salticid-cuda\tools\agent_begin_work_slice.py")
MAINLINE_HANDOFF_APPEND = Path(r"C:\code\salticid-cuda\tools\handoff_append.py")
MAINLINE_REPO_ROOT = MAINLINE_AGENT_BEGIN.parents[1]
PROFILE_CHOICES = ("native", "runtime", "catalog", "checkpoint", "unspecified")


@dataclass(frozen=True)
class BreadcrumbAppendPlan:
    message: str
    command: list[str]
    checkpoint_id: str | None
    generated_checkpoint_id: bool


def build_breadcrumb_message(*, branch: str, head: str, dirty: bool, intent: str, profile: str) -> str:
    module = _load_mainline_agent_begin_module()
    return module.build_handoff_message(
        branch=branch,
        head=head,
        dirty=dirty,
        intent=intent,
        profile=profile,
    )


def _load_mainline_agent_begin_module():
    if not MAINLINE_AGENT_BEGIN.exists():
        raise FileNotFoundError(f"Mainline begin-work-slice helper not found: {MAINLINE_AGENT_BEGIN}")
    spec = importlib.util.spec_from_file_location("mainline_agent_begin_work_slice", MAINLINE_AGENT_BEGIN)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load mainline helper from: {MAINLINE_AGENT_BEGIN}")
    module = importlib.util.module_from_spec(spec)
    inserted: list[str] = []
    for path in (str(MAINLINE_REPO_ROOT), str(MAINLINE_AGENT_BEGIN.parent)):
        if path not in sys.path:
            sys.path.insert(0, path)
            inserted.append(path)
    try:
        spec.loader.exec_module(module)
    finally:
        for path in reversed(inserted):
            try:
                sys.path.remove(path)
            except ValueError:
                pass
    return module


def _capture_git(*args: str) -> str:
    proc = subprocess.run(
        ["git", *args],
        cwd=str(REPO_ROOT),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout or "git command failed").strip()
        raise SystemExit(detail)
    return (proc.stdout or "").strip()


def build_breadcrumb_append_plan(*, py: str, message: str) -> BreadcrumbAppendPlan:
    checkpoint_id, generated_checkpoint_id = resolve_handoff_checkpoint_token(commit=None, message=message)
    commands = build_handoff_append_commands(
        py=py,
        message=message,
        commit=checkpoint_id,
        resolve_last_pending=False,
        repo_root=REPO_ROOT,
    )
    if len(commands) != 1:
        raise RuntimeError("viewer_host_begin_work_slice expected a single breadcrumb append command")
    return BreadcrumbAppendPlan(
        message=message,
        command=commands[0],
        checkpoint_id=checkpoint_id,
        generated_checkpoint_id=generated_checkpoint_id,
    )


def build_handoff_append_cmd(*, py: str, message: str) -> list[str]:
    return build_breadcrumb_append_plan(py=py, message=message).command


def _print_checkpoint_guidance(plan: BreadcrumbAppendPlan) -> None:
    if not plan.generated_checkpoint_id or not plan.checkpoint_id:
        return
    print(f"viewer_host_begin_work_slice: checkpoint_id={plan.checkpoint_id}")
    print(
        "viewer_host_begin_work_slice: reuse this token with "
        f'py -3.14 tools/viewer_host_append_handoff.py --commit {plan.checkpoint_id} --score <n> "<message>" '
        "when checkpointing this slice"
    )


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(
        description="Repo-specific breadcrumb helper that appends a session-start HANDOFF_LOG entry using the local checkpoint-id flow.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    ap.add_argument("--intent", required=True, help="Short intent for this work slice")
    ap.add_argument("--profile", default="native", choices=PROFILE_CHOICES, help="Expected validation profile/task lane")
    ap.add_argument("--dry-run", action="store_true", help="Print the handoff message and command without appending")
    ns = ap.parse_args(argv)

    branch = _capture_git("rev-parse", "--abbrev-ref", "HEAD")
    head = _capture_git("rev-parse", "--short", "HEAD")
    dirty = repo_is_dirty(REPO_ROOT)
    message = build_breadcrumb_message(
        branch=branch,
        head=head,
        dirty=dirty,
        intent=ns.intent,
        profile=ns.profile,
    )
    plan = build_breadcrumb_append_plan(py=sys.executable, message=message)
    if ns.dry_run:
        print(message)
        _print_checkpoint_guidance(plan)
        print(subprocess.list2cmdline(plan.command))
        return 0
    proc = subprocess.run(plan.command, cwd=str(REPO_ROOT), check=False)
    if proc.returncode == 0:
        _print_checkpoint_guidance(plan)
    return int(proc.returncode)


if __name__ == "__main__":
    raise SystemExit(main())