from __future__ import annotations

import argparse
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

try:
    from tools.viewer_host_checkpoint_guard import capture_repo_snapshot, discover_repo_root, load_validation_receipt
    from tools.viewer_host_contract_state import (
        REPO_ROOT,
        contract_proof_receipt_path,
        load_and_validate_slice_contract,
        validate_locked_contract_state,
    )
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import capture_repo_snapshot, discover_repo_root, load_validation_receipt
    from viewer_host_contract_state import (
        REPO_ROOT,
        contract_proof_receipt_path,
        load_and_validate_slice_contract,
        validate_locked_contract_state,
    )


VALIDATOR_COMMANDS: dict[str, list[str]] = {
    "fits_defaults": ["py", "-3.14", "tools/viewer_host_validate_fits_contract.py", "--contract", "docs/contracts/runtime_walk_fits.contract.json"],
}


def _run_validator(command: list[str], repo_root: Path) -> tuple[bool, str]:
    proc = subprocess.run(
        command,
        cwd=str(repo_root),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )
    return proc.returncode == 0, (proc.stdout or "").strip()


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Write a machine-derived contract-proof receipt for the current clean committed HEAD")
    parser.add_argument("--session-id", required=True, help="Current host session id")
    parser.add_argument("--cwd", default=str(REPO_ROOT), help="Repo cwd for resolution")
    args = parser.parse_args(argv)

    repo_root = discover_repo_root(Path(args.cwd))
    snapshot = capture_repo_snapshot(repo_root)
    if not snapshot.get("clean", False):
        sys.stderr.write("viewer_host_write_contract_proof_receipt: requires a clean repository state\n")
        return 2

    contract_state, contract_error = validate_locked_contract_state(args.session_id, repo_root)
    if contract_error:
        sys.stderr.write(f"viewer_host_write_contract_proof_receipt: {contract_error}\n")
        return 2

    contract_path = repo_root / str(contract_state["contract_path"])
    contract_payload, contract_result = load_and_validate_slice_contract(contract_path, repo_root)
    if contract_payload is None or not contract_result.ok:
        sys.stderr.write("viewer_host_write_contract_proof_receipt: active contract failed validation\n")
        for error in contract_result.errors:
            sys.stderr.write(f"- {error}\n")
        return 2

    validation_receipt = load_validation_receipt(str(snapshot["head"]), repo_root)
    if validation_receipt is None:
        sys.stderr.write("viewer_host_write_contract_proof_receipt: missing validation receipt for current HEAD\n")
        return 2

    required_commands = list(contract_payload.get("required_validation_commands", []))
    executed_commands = [str(command) for command in validation_receipt.get("commands", [])]
    missing_commands = [
        command
        for command in required_commands
        if not any(command in executed for executed in executed_commands)
    ]
    if missing_commands:
        sys.stderr.write("viewer_host_write_contract_proof_receipt: missing required validation commands\n")
        for command in missing_commands:
            sys.stderr.write(f"- {command}\n")
        return 2

    validator_results: list[dict[str, object]] = []
    for validator_id in contract_payload.get("required_validators", []):
        command = VALIDATOR_COMMANDS.get(str(validator_id))
        if command is None:
            sys.stderr.write(f"viewer_host_write_contract_proof_receipt: unknown validator id: {validator_id}\n")
            return 2
        ok, output = _run_validator(command, repo_root)
        validator_results.append(
            {
                "validator_id": str(validator_id),
                "command": " ".join(command),
                "ok": ok,
                "output": output,
            }
        )
        if not ok:
            sys.stderr.write(
                "viewer_host_write_contract_proof_receipt: validator failed: "
                + str(validator_id)
                + "\n"
                + output
                + "\n"
            )
            return 2

    receipt_path = contract_proof_receipt_path(str(snapshot["head"]), repo_root)
    receipt_path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "repo_root": str(repo_root),
        "head": str(snapshot["head"]),
        "contract_id": contract_state["contract_id"],
        "contract_path": contract_state["contract_path"],
        "contract_hash": contract_state["contract_hash"],
        "validation_receipt_head": validation_receipt["head"],
        "validated_commands": executed_commands,
        "validator_results": validator_results,
        "written_at_utc": datetime.now(timezone.utc).isoformat(),
    }
    receipt_path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"viewer_host_write_contract_proof_receipt: wrote {receipt_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
