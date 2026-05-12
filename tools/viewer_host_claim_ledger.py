from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_LEDGER = Path("artifacts/validation/viewer_host_claim_ledger.json")


def _ledger_path(repo_root: Path, path_text: str | None) -> Path:
    path = Path(path_text) if path_text else DEFAULT_LEDGER
    if not path.is_absolute():
        path = repo_root / path
    return path


def _load(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {"version": 1, "entries": []}
    return json.loads(path.read_text(encoding="utf-8"))


def _write(path: Path, payload: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def validate_ledger(payload: dict[str, Any], repo_root: Path) -> dict[str, Any]:
    entries = payload.get("entries", [])
    invalid: list[dict[str, Any]] = []
    for entry in entries if isinstance(entries, list) else []:
        if not isinstance(entry, dict):
            continue
        if entry.get("status") == "invalid" and not entry.get("corrected_at_utc"):
            invalid.append(entry)
            continue
        for artifact_text in entry.get("evidence_artifacts", []) or []:
            artifact = Path(str(artifact_text))
            if not artifact.is_absolute():
                artifact = repo_root / artifact
            if not artifact.exists():
                item = dict(entry)
                item["missing_artifact"] = str(artifact_text)
                invalid.append(item)
    return {"ok": not invalid, "entry_count": len(entries) if isinstance(entries, list) else 0, "invalid_entries": invalid}


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Record or validate machine-auditable status claims")
    sub = parser.add_subparsers(dest="command", required=True)
    record = sub.add_parser("record")
    record.add_argument("--claim", required=True)
    record.add_argument("--status", default="valid", choices=("valid", "invalid", "corrected"))
    record.add_argument("--artifact", action="append", default=[])
    record.add_argument("--ledger", default=None)
    validate = sub.add_parser("validate")
    validate.add_argument("--ledger", default=None)
    validate.add_argument("--out-json", default=None)
    args = parser.parse_args(argv)
    repo_root = REPO_ROOT.resolve()
    ledger_path = _ledger_path(repo_root, getattr(args, "ledger", None))
    payload = _load(ledger_path)
    if args.command == "record":
        entries = payload.setdefault("entries", [])
        entry = {
            "entry_id": f"claim-{len(entries) + 1:04d}",
            "timestamp_utc": datetime.now(timezone.utc).isoformat(),
            "claim": args.claim,
            "status": args.status,
            "evidence_artifacts": list(args.artifact or []),
        }
        entries.append(entry)
        _write(ledger_path, payload)
        print(json.dumps(entry, indent=2, sort_keys=True))
        return 0
    verdict = validate_ledger(payload, repo_root)
    out_json = getattr(args, "out_json", None)
    if out_json:
        out_path = _ledger_path(repo_root, out_json)
        _write(out_path, verdict)
    print(json.dumps(verdict, indent=2, sort_keys=True))
    return 0 if verdict["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
