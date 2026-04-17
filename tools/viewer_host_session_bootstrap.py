from __future__ import annotations

import argparse
import json
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

try:
    from tools.viewer_host_repo_status import repo_is_dirty
    from tools.viewer_host_contract_state import GLOBAL_CONTRACT_SESSION_ID, build_strict_banner, load_active_contract_state
except ModuleNotFoundError:
    from viewer_host_repo_status import repo_is_dirty
    from viewer_host_contract_state import GLOBAL_CONTRACT_SESSION_ID, build_strict_banner, load_active_contract_state


REPO_ROOT = Path(__file__).resolve().parents[1]
EXTERNAL_TESTING_CHEAT_SHEET = Path(r"C:\code\salticid-cuda\docs\testing_cheat_sheet.md")


@dataclass(frozen=True)
class BootstrapDoc:
    label: str
    path: str
    exists: bool


@dataclass(frozen=True)
class AuditSummary:
    command: list[str]
    returncode: int
    output_tail: list[str]


def _extract_task_output_paths(task: dict[str, Any]) -> list[str]:
    args = task.get("args", [])
    if not isinstance(args, list):
        return []

    outputs: list[str] = []
    output_flags = {"--log", "--out", "--out-dir", "--json-out"}
    for index, raw in enumerate(args[:-1]):
        if str(raw) not in output_flags:
            continue
        value = args[index + 1]
        if not isinstance(value, (str, int, float)):
            continue
        outputs.append(str(value))
    return list(dict.fromkeys(outputs))


def build_validation_profiles(repo_root: Path = REPO_ROOT) -> dict[str, Any]:
    tasks_json_path = repo_root / ".vscode" / "tasks.json"
    if not tasks_json_path.exists():
        return {}

    payload = json.loads(tasks_json_path.read_text(encoding="utf-8"))
    raw_tasks = payload.get("tasks", [])
    if not isinstance(raw_tasks, list):
        return {}

    tasks_by_label: dict[str, dict[str, Any]] = {}
    for task in raw_tasks:
        if not isinstance(task, dict):
            continue
        label = task.get("label")
        if not isinstance(label, str) or not label.strip():
            continue
        tasks_by_label[label] = task

    profiles: dict[str, Any] = {}
    for label, task in tasks_by_label.items():
        if not label.startswith("verify: profile "):
            continue

        profile_name = label.removeprefix("verify: profile ")
        depends_on = task.get("dependsOn", [])
        if isinstance(depends_on, str):
            depends = [depends_on]
        elif isinstance(depends_on, list):
            depends = [str(item) for item in depends_on]
        else:
            depends = []

        steps: list[dict[str, Any]] = []
        for step_label in depends:
            step_task = tasks_by_label.get(step_label, {})
            steps.append(
                {
                    "label": step_label,
                    "outputs": _extract_task_output_paths(step_task),
                }
            )

        profiles[profile_name] = {
            "task_label": label,
            "steps": steps,
        }

    return profiles


def required_docs(repo_root: Path) -> list[BootstrapDoc]:
    docs = [
        ("Testing cheat sheet", EXTERNAL_TESTING_CHEAT_SHEET),
        ("Agent bootstrap", repo_root / "AGENTS.md"),
        ("Agent working protocol", repo_root / "AGENT_WORKING_PROTOCOL.md"),
        ("Spec status", repo_root / "spec_intake" / "_STATUS.md"),
        ("Deferred threads", repo_root / "DEFERRED_THREADS.md"),
        ("Known issues", repo_root / "KNOWN_ISSUES.md"),
        ("Handoff log", repo_root / "HANDOFF_LOG.md"),
        ("Repo instructions", repo_root / ".github" / "copilot-instructions.md"),
        ("Phased plan continuity", repo_root / "docs" / "PHASED_PLAN_CONTINUITY_PROTOCOL.md"),
    ]
    return [BootstrapDoc(label=label, path=str(path), exists=path.exists()) for label, path in docs]


def tail_handoff_entries(text: str, count: int) -> list[str]:
    entries = [line.strip()[2:] for line in text.splitlines() if line.strip().startswith("- `")]
    if count <= 0:
        return []
    return entries[-count:]


def legacy_pending_handoff_entries(text: str) -> list[str]:
    return [
        line.strip()[2:]
        for line in text.splitlines()
        if line.strip().startswith("- `") and "— pending:" in line
    ]


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
        raise RuntimeError(detail)
    return (proc.stdout or "").strip()


def _run_audit(py: str) -> AuditSummary:
    cmd = [py, "tools/code_quality_audit.py", "--check-baseline", "--out", "artifacts/code_quality_report.json"]
    proc = subprocess.run(
        cmd,
        cwd=str(REPO_ROOT),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    lines = [line.rstrip("\n") for line in (proc.stdout or "").splitlines()]
    return AuditSummary(command=cmd, returncode=int(proc.returncode), output_tail=lines[-20:])


def build_bootstrap_state(*, py: str = sys.executable, run_audit: bool, tail_handoff: int) -> dict[str, Any]:
    branch = _capture_git("rev-parse", "--abbrev-ref", "HEAD")
    head = _capture_git("rev-parse", "--short", "HEAD")
    dirty = repo_is_dirty(REPO_ROOT)
    handoff_log = (REPO_ROOT / "HANDOFF_LOG.md").read_text(encoding="utf-8")
    legacy_pending = legacy_pending_handoff_entries(handoff_log)
    audit = _run_audit(py) if run_audit else None
    docs = [asdict(doc) for doc in required_docs(REPO_ROOT)]
    validation_profiles = build_validation_profiles(REPO_ROOT)
    active_contract = load_active_contract_state(GLOBAL_CONTRACT_SESSION_ID, REPO_ROOT)
    return {
        "repo_root": str(REPO_ROOT),
        "git": {"branch": branch, "head": head, "dirty": dirty},
        "strict_banner": build_strict_banner(active_contract),
        "active_contract": active_contract,
        "docs": docs,
        "recent_handoff": tail_handoff_entries(handoff_log, tail_handoff),
        "handoff_warnings": {
            "legacy_pending_count": len(legacy_pending),
            "legacy_pending_entries": legacy_pending[-tail_handoff:] if tail_handoff > 0 else [],
        },
        "audit": asdict(audit) if audit is not None else None,
        "validation_profiles": validation_profiles,
        "next_commands": {
            "begin_work_slice": "py -3.14 tools/viewer_host_begin_work_slice.py --intent \"<slice>\" --profile <native|runtime|catalog|checkpoint|unspecified> --plan <plan> --contract <contract>",
            "append_handoff": "py -3.14 tools/viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> \"<message>\"",
            "append_handoff_legacy_pending": "py -3.14 tools/viewer_host_append_handoff.py --resolve-last-pending --score <n> \"<message>\"",
            "assert_plan_sync": "py -3.14 tools/viewer_host_assert_phased_plan_sync.py",
            "prepare_slice": "py -3.14 tools/viewer_host_prepare_slice.py --session-id <session_id> --plan <plan> --contract <contract>",
            "profiles": "Use the VS Code tasks under verify: profile ...; bootstrap now lists each profile's steps and artifact paths.",
        },
    }


def collect_bootstrap_state(*, py: str, run_audit: bool, tail_handoff: int) -> dict[str, Any]:
    return build_bootstrap_state(py=py, run_audit=run_audit, tail_handoff=tail_handoff)


def print_bootstrap_report(state: dict[str, Any]) -> None:
    print("Viewer-host session bootstrap")
    print(f"repo: {state['repo_root']}")
    git_state = state["git"]
    print(f"git: branch={git_state['branch']} head={git_state['head']} status={'dirty' if git_state['dirty'] else 'clean'}")
    print(f"strict banner: {state['strict_banner']}")
    active_contract = state.get("active_contract")
    if active_contract:
        print(
            "active contract: "
            + f"{active_contract.get('contract_id', '<unknown>')} "
            + f"({active_contract.get('contract_path', '<unknown>')})"
        )
    print("required docs:")
    for doc in state["docs"]:
        status = "ok" if doc["exists"] else "missing"
        print(f"- [{status}] {doc['label']}: {doc['path']}")
    print("recent handoff:")
    recent = state["recent_handoff"]
    if recent:
        for entry in recent:
            print(f"- {entry}")
    else:
        print("- <no checkpoint entries found>")
    warnings = state["handoff_warnings"]
    if warnings["legacy_pending_count"]:
        print(f"handoff warnings: legacy_pending_entries={warnings['legacy_pending_count']}")
        for entry in warnings["legacy_pending_entries"]:
            print(f"- {entry}")
    audit = state["audit"]
    if audit is not None:
        result = "ok" if audit["returncode"] == 0 else f"nonzero={audit['returncode']}"
        print(f"code quality audit: {result}")
        for line in audit["output_tail"]:
            print(f"  {line}")
    print("validation profiles:")
    profiles = state.get("validation_profiles", {}) or {}
    if profiles:
        for profile_name, profile in profiles.items():
            step_summaries = []
            for step in profile.get("steps", []):
                outputs = step.get("outputs", []) or []
                suffix = f" -> {', '.join(outputs)}" if outputs else ""
                step_summaries.append(f"{step.get('label', '<unknown>')}{suffix}")
            if step_summaries:
                print(f"- {profile_name}: " + " | ".join(step_summaries))
            else:
                print(f"- {profile_name}: {profile.get('task_label', '<unknown>')}")
    else:
        print("- <no validation profiles found>")
    print("next commands:")
    for label, command in state["next_commands"].items():
        print(f"- {label}: {command}")


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(
        description="Print the repeatable session-start bootstrap state for the viewer-host repo.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    ap.add_argument("--audit", action="store_true", help="Run the code-quality audit and include a short summary")
    ap.add_argument("--tail-handoff", type=int, default=8, help="How many recent HANDOFF_LOG checkpoint entries to show")
    ap.add_argument("--json-out", default=None, help="Optional path to write the collected bootstrap state as JSON")
    ap.add_argument("--python", default=sys.executable, help="Python executable to use for Python-backed helper commands")
    ns = ap.parse_args(argv)

    state = build_bootstrap_state(py=ns.python, run_audit=ns.audit, tail_handoff=ns.tail_handoff)
    print_bootstrap_report(state)
    if ns.json_out:
        out_path = Path(ns.json_out)
        if not out_path.is_absolute():
            out_path = REPO_ROOT / out_path
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(state, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    missing = [doc for doc in state["docs"] if not doc["exists"]]
    return 2 if missing else 0


if __name__ == "__main__":
    raise SystemExit(main())
