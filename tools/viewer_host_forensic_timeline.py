from __future__ import annotations

import argparse
import json
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_TRANSCRIPT = Path(r"c:\Users\Adam\AppData\Roaming\Code\User\workspaceStorage\b6d63ebe0b1a81eb9c5e0ff7a856efb1\GitHub.copilot-chat\transcripts\d751df96-0238-4a44-9852-fdd1ca0af265.jsonl")
DEFAULT_USER_AUDIT = Path(r"C:\Users\Adam\AppData\Local\Temp\d751df96_user_messages_audit.txt")
RESTRICTED_WORD_RE = re.compile(r"\b(clean|green|done|fixed|complete|closed|unblocked|verified|passed|ready)\b", re.IGNORECASE)
USER_CORRECTION_RE = re.compile(r"\b(lie|lied|lying|failed|ignored|dirty|waste|wrong|stop|do the work|no product|make it impossible)\b", re.IGNORECASE)

CATEGORY_INVARIANTS = {
    "status_overclaim_without_live_evidence": {
        "detector": "restricted status vocabulary claim without valid truth report artifact",
        "blocking_condition": "checkpoint guard denies the claim",
        "proof_artifact": "artifacts/pytest/anti_lie_workflow_tools.junit.xml",
        "recheck_command": "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q",
    },
    "stale_receipt_or_artifact_treated_as_truth": {
        "detector": "validation receipt evidence hash/size/mtime drift",
        "blocking_condition": "contract proof receipt guard denies closure",
        "proof_artifact": "artifacts/pytest/anti_lie_workflow_tools.junit.xml",
        "recheck_command": "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py tests/test_viewer_host_contract_proof.py -q",
    },
    "dirty_worktree_missed_after_closure": {
        "detector": "live git status has staged, unstaged, or untracked output",
        "blocking_condition": "truth report ok=false and status claims are invalid",
        "proof_artifact": "artifacts/validation/viewer_host_truth_report.json",
        "recheck_command": "py -3.14 tools/viewer_host_truth_report.py --out-json artifacts/validation/viewer_host_truth_report.json",
    },
    "scope_drift_from_controlling_ask": {
        "detector": "timeline event where active user instruction forbids product work but agent performs product work",
        "blocking_condition": "forensic dossier records violation and anti-lie contract forbids product mutation scope",
        "proof_artifact": "artifacts/validation/anti_lie_forensic_timeline.json",
        "recheck_command": "py -3.14 tools/viewer_host_forensic_timeline.py --out-json artifacts/validation/anti_lie_forensic_timeline.json",
    },
    "protocol_prose_substituted_for_enforcement": {
        "detector": "plan or summary claim without corresponding validator artifact",
        "blocking_condition": "truth report and claim ledger require validator artifact",
        "proof_artifact": "artifacts/validation/viewer_host_truth_report.json",
        "recheck_command": "py -3.14 tools/viewer_host_truth_report.py --out-json artifacts/validation/viewer_host_truth_report.json",
    },
    "hostile_review_omitted_or_weakened": {
        "detector": "meaningful plan lacks hostile audit or open audit passes remain",
        "blocking_condition": "viewer_host_validate_hostile_audit.py returns ok=false",
        "proof_artifact": "artifacts/validation/anti_lie_claim_enforcement_hostile_audit.json",
        "recheck_command": "py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/anti_lie_claim_enforcement_PHASED_PLAN.md --out-json artifacts/validation/anti_lie_claim_enforcement_hostile_audit.json",
    },
    "hook_friction_without_preventing_lie": {
        "detector": "recovery or hook event after a false status claim",
        "blocking_condition": "forensic dossier records hook friction and maps it to a guard invariant",
        "proof_artifact": "artifacts/validation/anti_lie_forensic_timeline.json",
        "recheck_command": "py -3.14 tools/viewer_host_forensic_timeline.py --out-json artifacts/validation/anti_lie_forensic_timeline.json",
    },
    "accidental_file_mutation_or_line_churn": {
        "detector": "git diff/recovery event containing unexpected changed path or full-file churn",
        "blocking_condition": "truth report ok=false when live status is dirty",
        "proof_artifact": "artifacts/validation/viewer_host_truth_report.json",
        "recheck_command": "git status --short && py -3.14 tools/viewer_host_truth_report.py --out-json artifacts/validation/viewer_host_truth_report.json",
    },
    "final_report_shape_ignored": {
        "detector": "user requested a restricted report shape and agent response diverged",
        "blocking_condition": "forensic dossier records the divergence and final summaries must quote truth report fields",
        "proof_artifact": "artifacts/validation/anti_lie_forensic_timeline.json",
        "recheck_command": "py -3.14 tools/viewer_host_forensic_timeline.py --out-json artifacts/validation/anti_lie_forensic_timeline.json",
    },
    "other_agent_framing_accepted_without_revalidation": {
        "detector": "agent follows inherited summary instead of controlling transcript instruction",
        "blocking_condition": "forensic dossier records inherited-framing violation and contract forbids product detour",
        "proof_artifact": "artifacts/validation/anti_lie_forensic_timeline.json",
        "recheck_command": "py -3.14 tools/viewer_host_forensic_timeline.py --out-json artifacts/validation/anti_lie_forensic_timeline.json",
    },
    "product_work_before_anti_lie_gate": {
        "detector": "product work starts while controlling instruction requires anti-lying proof first",
        "blocking_condition": "anti-lie contract mutation scope excludes product files",
        "proof_artifact": "docs/contracts/anti_lie_claim_enforcement.contract.json",
        "recheck_command": "py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/anti_lie_claim_enforcement.contract.json --out-json artifacts/validation/anti_lie_claim_enforcement_contract.json",
    },
    "regression_hidden_by_narrow_validation": {
        "detector": "user reports regression after a narrower validation claim",
        "blocking_condition": "forensic category must map to a focused RED before closure",
        "proof_artifact": "artifacts/pytest/anti_lie_workflow_tools.junit.xml",
        "recheck_command": "py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py tests/test_viewer_host_contract_proof.py -q",
    },
    "repeated_user_escalation_required": {
        "detector": "multiple user corrections are needed to force the agent back onto the controlling task",
        "blocking_condition": "forensic dossier records the recurrence and status reports must be generated from truth artifacts",
        "proof_artifact": "artifacts/validation/anti_lie_forensic_timeline.json",
        "recheck_command": "py -3.14 tools/viewer_host_forensic_timeline.py --out-json artifacts/validation/anti_lie_forensic_timeline.json",
    },
}


def _git(repo_root: Path, *args: str) -> str:
    proc = subprocess.run(["git", *args], cwd=str(repo_root), stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=False)
    return (proc.stdout or proc.stderr or "").strip()


def _event(events: list[dict[str, Any]], *, source: str, actor: str, action: str, instruction: str, violation_class: str, recurrence_key: str, evidence: str, cost: str, corrective: str) -> None:
    events.append({
        "event_id": f"event-{len(events) + 1:04d}",
        "sequence_index": len(events) + 1,
        "source": source,
        "actor_claim_action": action,
        "actor": actor,
        "user_instruction_in_force": instruction,
        "evidence_artifact": evidence,
        "violation_class": violation_class,
        "cost_risk": cost,
        "corrective_requirement": corrective,
        "recurrence_key": recurrence_key,
    })


def _classify_user_text(content: str) -> list[str]:
    lower = content.lower()
    categories: list[str] = []
    if "lie" in lower or "lying" in lower or "lied" in lower:
        categories.append("status_overclaim_without_live_evidence")
    if "dirty" in lower or "worktree" in lower:
        categories.append("dirty_worktree_missed_after_closure")
    if "hook" in lower or "recovery" in lower:
        categories.append("hook_friction_without_preventing_lie")
    if "no product" in lower or "product work first" in lower or "make it impossible" in lower:
        categories.append("product_work_before_anti_lie_gate")
    if "do the work" in lower or "color pipeline" in lower or "hat rack" in lower or "wrong" in lower:
        categories.append("scope_drift_from_controlling_ask")
    if "protocol" in lower or "plan" in lower or "receipt" in lower or "proof" in lower:
        categories.append("protocol_prose_substituted_for_enforcement")
    if "hostile" in lower or "hostil" in lower:
        categories.append("hostile_review_omitted_or_weakened")
    if "regression" in lower or "regressed" in lower or "broken" in lower:
        categories.append("regression_hidden_by_narrow_validation")
    if "report" in lower or "final" in lower or "summary" in lower:
        categories.append("final_report_shape_ignored")
    if "other agent" in lower or "prev liar" in lower or "previous" in lower or "liar agent" in lower:
        categories.append("other_agent_framing_accepted_without_revalidation")
    if "waste" in lower or "premium" in lower or "scream" in lower or "token" in lower:
        categories.append("repeated_user_escalation_required")
    if not categories:
        categories.append("final_report_shape_ignored")
    return sorted(set(categories))


def _record_user_text_events(events: list[dict[str, Any]], *, source: str, content: str) -> None:
    for category in _classify_user_text(content):
        _event(
            events,
            source=source,
            actor="user",
            action=content[:500],
            instruction="latest user message in evidence corpus",
            violation_class="user_correction",
            recurrence_key=category,
            evidence=source,
            cost="forced user correction/escalation and token/time waste",
            corrective="derive and enforce a guard invariant for this recurrence category",
        )


def _scan_transcript(path: Path, events: list[dict[str, Any]]) -> None:
    if not path.exists():
        _event(events, source=str(path), actor="tool", action="source missing", instruction="forensic evidence corpus must include transcript", violation_class="source_missing", recurrence_key="source_missing", evidence=str(path), cost="timeline incompleteness", corrective="block closure or provide the missing source")
        return
    for line_number, raw in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), start=1):
        try:
            payload = json.loads(raw)
        except json.JSONDecodeError:
            continue
        content = str(payload.get("content") or payload.get("message") or "")
        msg_type = str(payload.get("type", ""))
        if msg_type == "user.message" and USER_CORRECTION_RE.search(content):
            _record_user_text_events(events, source=f"{path}:L{line_number}", content=content)
        elif msg_type != "user.message" and RESTRICTED_WORD_RE.search(content):
            _event(events, source=f"{path}:L{line_number}", actor="agent", action=content[:500], instruction="restricted status vocabulary requires machine proof", violation_class="status_claim", recurrence_key="status_overclaim_without_live_evidence", evidence=f"{path}:L{line_number}", cost="possible unsupported confidence claim", corrective="require truth report or claim id")


def _scan_user_audit(path: Path, events: list[dict[str, Any]]) -> None:
    if not path.exists():
        return
    text = path.read_text(encoding="utf-8", errors="replace")
    pattern = re.compile(r"----- USER MESSAGE line=(?P<line>\d+) timestamp=(?P<timestamp>.*?) -----\n(?P<content>.*?)(?=\n----- USER MESSAGE line=|\Z)", re.DOTALL)
    for match in pattern.finditer(text):
        content = match.group("content").strip()
        if not content or not USER_CORRECTION_RE.search(content):
            continue
        source = f"{path}:transcript-line-{match.group('line')}"
        _record_user_text_events(events, source=source, content=content)


def _scan_recovery(repo_root: Path, events: list[dict[str, Any]]) -> None:
    recovery_dir = repo_root / "artifacts" / "hooks" / "viewer_host_checkpoint_guard" / "recovery"
    if not recovery_dir.exists():
        return
    for path in sorted(recovery_dir.glob("*.json")):
        try:
            payload = json.loads(path.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            continue
        changed = payload.get("changed_paths", {}).get("all", []) if isinstance(payload.get("changed_paths"), dict) else []
        action = f"recovery report clean={payload.get('git', {}).get('clean')} changed_paths={changed} reason={payload.get('reason', '')}"
        key = "dirty_worktree_missed_after_closure" if changed else "hook_friction_without_preventing_lie"
        _event(events, source=str(path), actor="hook", action=action, instruction="dirty state must be explicit and unrecoverable by prose", violation_class="recovery_report", recurrence_key=key, evidence=str(path), cost="forced crash/recovery workflow and user steering", corrective="truth report must read live git state")


def _scan_receipts(repo_root: Path, events: list[dict[str, Any]]) -> None:
    for dirname, kind in (("viewer_host_validation_receipts", "validation_receipt"), ("viewer_host_contract_proof_receipts", "contract_proof_receipt")):
        root = repo_root / "artifacts" / "hooks" / dirname
        if not root.exists():
            continue
        for path in sorted(root.glob("*.json")):
            _event(events, source=str(path), actor="receipt", action=f"{kind} exists", instruction="receipts are evidence only when current and revalidated", violation_class="proof_event", recurrence_key="stale_receipt_or_artifact_treated_as_truth", evidence=str(path), cost="stale proof can be mistaken for current truth", corrective="revalidate evidence at closure")


def build_dossier(repo_root: Path = REPO_ROOT, transcript: Path = DEFAULT_TRANSCRIPT) -> dict[str, Any]:
    events: list[dict[str, Any]] = []
    _scan_transcript(transcript, events)
    _scan_user_audit(DEFAULT_USER_AUDIT, events)
    _scan_recovery(repo_root, events)
    _scan_receipts(repo_root, events)
    status = _git(repo_root, "status", "--short")
    if status:
        _event(events, source="git status --short", actor="git", action=status, instruction="live worktree state is authoritative", violation_class="dirty_state", recurrence_key="dirty_worktree_missed_after_closure", evidence="git status --short", cost="dirty state invalidates closure", corrective="truth report blocks status claims")
    categories = sorted({event["recurrence_key"] for event in events if event["recurrence_key"] != "source_missing"})
    guard_invariants = {category: CATEGORY_INVARIANTS.get(category, {
        "detector": "forensic parser recurrence key",
        "blocking_condition": "category must be assigned an explicit invariant before closure",
        "proof_artifact": "artifacts/validation/anti_lie_forensic_timeline.json",
        "recheck_command": "py -3.14 tools/viewer_host_forensic_timeline.py --out-json artifacts/validation/anti_lie_forensic_timeline.json",
    }) for category in categories}
    missing_invariants = [category for category in categories if category not in guard_invariants]
    source_missing = [event for event in events if event["recurrence_key"] == "source_missing"]
    ok = bool(events) and not source_missing and not missing_invariants
    return {
        "ok": ok,
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "repo_root": repo_root.as_posix(),
        "evidence_set": {
            "transcript": str(transcript),
            "user_message_audit": str(DEFAULT_USER_AUDIT) if DEFAULT_USER_AUDIT.exists() else None,
            "git_status_short": status.splitlines() if status else [],
        },
        "event_count": len(events),
        "events": events,
        "categories": categories,
        "guard_invariants": guard_invariants,
        "quantifier_check": {
            "ok": not missing_invariants and not source_missing,
            "missing_invariants": missing_invariants,
            "source_missing": [event["evidence_artifact"] for event in source_missing],
            "formula": "for every c in C, exists g in G such that g.blocks(c) == true",
        },
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Build the anti-lie forensic timeline and recurrence taxonomy")
    parser.add_argument("--out-json", required=True)
    parser.add_argument("--repo-root", default=str(REPO_ROOT))
    parser.add_argument("--transcript", default=str(DEFAULT_TRANSCRIPT))
    args = parser.parse_args(argv)
    repo_root = Path(args.repo_root).resolve()
    payload = build_dossier(repo_root, Path(args.transcript))
    out_path = Path(args.out_json)
    if not out_path.is_absolute():
        out_path = repo_root / out_path
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"viewer_host_forensic_timeline: ok={payload['ok']} events={payload['event_count']} out={out_path}")
    return 0 if payload["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
