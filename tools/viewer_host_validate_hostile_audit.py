from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
CHECKLIST_ITEM_RE = re.compile(r"^- \[([^\]]+)\]\s*(.+?)\s*$")
STATUS_RE = re.compile(r"^- Status:\s*(.+?)\s*$", re.IGNORECASE)
COMPLETE_STATUSES = {"done", "complete", "clean", "closed"}
COMPLETE_CHECKLIST_STATUSES = COMPLETE_STATUSES | {"x"}


def _is_clean_reaudit_text(text: str) -> bool:
    lower = text.lower()
    markers = (
        "clean re-read",
        "no additional real defect found",
        "no additional real issue found",
        "no additional workflow mistake found",
        "re-read the repaired state",
        "proved cleanly",
        "did not expose another",
        "confirmed the repaired state",
    )
    return any(marker in lower for marker in markers)


def plan_declares_hostile_audit(plan_path: Path) -> bool:
    text = plan_path.read_text(encoding="utf-8")
    return "## Hostile Audit" in text or "## Audit Passes" in text or "## Audit Findings" in text


def _section_lines(text: str, heading: str) -> list[str]:
    lines: list[str] = []
    in_section = False
    for raw_line in text.splitlines():
        stripped = raw_line.strip()
        if stripped.startswith("## "):
            if in_section:
                break
            in_section = stripped == heading
            continue
        if in_section and stripped:
            lines.append(stripped)
    return lines


def _checklist_items(lines: list[str]) -> list[tuple[str, str]]:
    items: list[tuple[str, str]] = []
    for line in lines:
        match = CHECKLIST_ITEM_RE.match(line)
        if match is None:
            continue
        items.append((match.group(1).strip().lower(), match.group(2).strip()))
    return items


def validate_hostile_audit_plan(plan_path: Path) -> dict[str, object]:
    text = plan_path.read_text(encoding="utf-8")
    hostile_audit_lines = _section_lines(text, "## Hostile Audit")
    audit_pass_lines = _section_lines(text, "## Audit Passes")
    audit_finding_lines = _section_lines(text, "## Audit Findings")

    status = "missing"
    for line in hostile_audit_lines:
        match = STATUS_RE.match(line)
        if match is not None:
            status = match.group(1).strip().lower()
            break

    audit_passes = _checklist_items(audit_pass_lines)
    audit_findings = _checklist_items(audit_finding_lines)
    open_passes = [text for pass_status, text in audit_passes if pass_status == "open"]
    completed_passes = [text for pass_status, text in audit_passes if pass_status in COMPLETE_CHECKLIST_STATUSES]
    clean_reaudit_passes = [text for text in completed_passes if _is_clean_reaudit_text(text)]
    completed_findings = [
        text
        for finding_status, text in audit_findings
        if finding_status in COMPLETE_CHECKLIST_STATUSES and "placeholder" not in text.lower()
    ]
    clean_reaudit_findings = [text for text in completed_findings if _is_clean_reaudit_text(text)]
    real_findings = [text for text in completed_findings if not _is_clean_reaudit_text(text)]
    clean_reaudit_evidence = clean_reaudit_passes + clean_reaudit_findings

    blocked_reason = ""
    if not hostile_audit_lines:
        blocked_reason = "hostile audit section is missing"
    elif not audit_pass_lines:
        blocked_reason = "audit passes section is missing"
    elif status not in COMPLETE_STATUSES:
        blocked_reason = f"hostile audit status is {status}"
    elif open_passes:
        blocked_reason = "hostile audit still has open audit passes"
    elif len(completed_passes) < 3:
        blocked_reason = "hostile audit needs at least three completed passes before closure"
    elif real_findings and not clean_reaudit_evidence:
        blocked_reason = "hostile audit found a real issue but does not yet prove the repaired state with a clean re-audit"
    elif not real_findings and len(clean_reaudit_evidence) < 2:
        blocked_reason = "hostile audit does not yet prove closure with second and third clean audit passes"

    return {
        "ok": not blocked_reason,
        "plan_path": str(plan_path),
        "status": status,
        "audit_passes_completed": len(completed_passes),
        "real_finding_recorded": bool(real_findings),
        "clean_passes_completed": len(clean_reaudit_passes),
        "clean_reaudit_evidence_count": len(clean_reaudit_evidence),
        "findings": real_findings,
        "open_audit_passes": open_passes,
        "blocked_reason": blocked_reason,
    }


def build_placeholder_payload(plan_path: Path) -> dict[str, object]:
    return {
        "ok": False,
        "plan_path": str(plan_path),
        "audit_passes_completed": 0,
        "real_finding_recorded": False,
        "clean_passes_completed": 0,
        "findings": [],
        "blocked_reason": "hostile-audit validator not implemented yet",
    }


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate hostile-review audit state from a phased plan")
    parser.add_argument("--plan", required=True, help="Checked-in phased plan path")
    parser.add_argument("--out-json", help="Optional JSON output path")
    args = parser.parse_args(argv)

    plan_path = Path(args.plan)
    if not plan_path.is_absolute():
        plan_path = REPO_ROOT / plan_path
    if not plan_path.exists():
        sys.stderr.write(f"viewer_host_validate_hostile_audit: missing plan: {plan_path}\n")
        return 2

    payload = validate_hostile_audit_plan(plan_path)
    if args.out_json:
        out_path = Path(args.out_json)
        if not out_path.is_absolute():
            out_path = REPO_ROOT / out_path
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(payload, indent=2))
    return 0 if bool(payload.get("ok", False)) else 1


if __name__ == "__main__":
    raise SystemExit(main())