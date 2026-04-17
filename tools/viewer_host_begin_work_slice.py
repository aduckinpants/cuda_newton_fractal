from __future__ import annotations

import argparse
import importlib.util
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any

try:
    from tools.viewer_host_append_handoff import build_handoff_append_commands, resolve_handoff_checkpoint_token
    from tools.viewer_host_repo_status import repo_is_dirty
    from tools.viewer_host_contract_state import GLOBAL_CONTRACT_SESSION_ID, load_and_validate_slice_contract, write_active_contract_state
    from tools.viewer_host_assert_phased_plan_sync import validate_plan_text
except ModuleNotFoundError:
    from viewer_host_append_handoff import build_handoff_append_commands, resolve_handoff_checkpoint_token
    from viewer_host_repo_status import repo_is_dirty
    from viewer_host_contract_state import GLOBAL_CONTRACT_SESSION_ID, load_and_validate_slice_contract, write_active_contract_state
    from viewer_host_assert_phased_plan_sync import validate_plan_text


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
        description="Repo-specific breadcrumb helper that appends a session-start HANDOFF_LOG entry, prints checkpoint-id reuse guidance, and uses the local checkpoint-id flow.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    ap.add_argument("--intent", required=True, help="Short intent for this work slice")
    ap.add_argument("--profile", default="native", choices=PROFILE_CHOICES, help="Expected validation profile/task lane")
    ap.add_argument("--plan", required=True, help="Checked-in phased plan path for this slice")
    ap.add_argument("--contract", required=True, help="Checked-in machine-readable contract path for this slice")
    ap.add_argument("--session-id", default=GLOBAL_CONTRACT_SESSION_ID, help="Optional contract-lock session id; defaults to the global active contract state")
    ap.add_argument("--dry-run", action="store_true", help="Print the session-start message, checkpoint-id guidance, and delegated command without appending")
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
    plan_path = Path(ns.plan)
    if not plan_path.is_absolute():
        plan_path = REPO_ROOT / plan_path
    contract_path = Path(ns.contract)
    if not contract_path.is_absolute():
        contract_path = REPO_ROOT / contract_path
    if not plan_path.exists():
        raise SystemExit(f"Missing phased plan: {plan_path}")
    if not contract_path.exists():
        raise SystemExit(f"Missing slice contract: {contract_path}")
    plan_error = validate_plan_text(plan_path.read_text(encoding="utf-8"), display_path=str(plan_path))
    if plan_error:
        raise SystemExit(f"Phased plan invalid: {plan_error}")
    contract_payload, contract_result = load_and_validate_slice_contract(contract_path, REPO_ROOT)
    if contract_payload is None or not contract_result.ok:
        raise SystemExit(
            "Invalid slice contract:\n" + "\n".join(f"- {error}" for error in contract_result.errors)
        )
    if str(contract_payload["plan_path"]).replace("\\", "/") != plan_path.relative_to(REPO_ROOT).as_posix():
        raise SystemExit("Slice contract plan_path does not match the provided phased plan")
    plan = build_breadcrumb_append_plan(py=sys.executable, message=message)
    if ns.dry_run:
        print(message)
        _print_checkpoint_guidance(plan)
        print(f"viewer_host_begin_work_slice: plan={plan_path.relative_to(REPO_ROOT).as_posix()}")
        print(f"viewer_host_begin_work_slice: contract={contract_path.relative_to(REPO_ROOT).as_posix()}")
        print(subprocess.list2cmdline(plan.command))
        return 0
    proc = subprocess.run(plan.command, cwd=str(REPO_ROOT), check=False)
    if proc.returncode == 0:
        state_path = write_active_contract_state(
            ns.session_id,
            repo_root=REPO_ROOT,
            contract_path=contract_path,
            contract_payload=contract_payload,
        )
        _print_checkpoint_guidance(plan)
        print(
            "viewer_host_begin_work_slice: active contract locked at "
            + state_path.relative_to(REPO_ROOT).as_posix()
        )
    return int(proc.returncode)


if __name__ == "__main__":
    raise SystemExit(main())
