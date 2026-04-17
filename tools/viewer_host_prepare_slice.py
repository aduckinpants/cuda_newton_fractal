from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

try:
    from tools.viewer_host_assert_phased_plan_sync import validate_plan_text
    from tools.viewer_host_checkpoint_guard import discover_repo_root
    from tools.viewer_host_contract_state import (
        REPO_ROOT,
        build_strict_banner,
        load_and_validate_slice_contract,
        write_active_contract_state,
    )
except ModuleNotFoundError:
    from viewer_host_assert_phased_plan_sync import validate_plan_text
    from viewer_host_checkpoint_guard import discover_repo_root
    from viewer_host_contract_state import (
        REPO_ROOT,
        build_strict_banner,
        load_and_validate_slice_contract,
        write_active_contract_state,
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Register an active checked-in slice contract for the current session")
    parser.add_argument("--session-id", required=True, help="Current host session id")
    parser.add_argument("--plan", required=True, help="Checked-in phased plan path")
    parser.add_argument("--contract", required=True, help="Checked-in machine-readable contract path")
    parser.add_argument("--cwd", default=str(REPO_ROOT), help="Repo cwd for resolution")
    args = parser.parse_args(argv)

    repo_root = discover_repo_root(Path(args.cwd))
    plan_path = Path(args.plan)
    if not plan_path.is_absolute():
        plan_path = repo_root / plan_path
    contract_path = Path(args.contract)
    if not contract_path.is_absolute():
        contract_path = repo_root / contract_path

    if not plan_path.exists():
        sys.stderr.write(f"viewer_host_prepare_slice: missing plan: {plan_path}\n")
        return 2
    if not contract_path.exists():
        sys.stderr.write(f"viewer_host_prepare_slice: missing contract: {contract_path}\n")
        return 2

    plan_text = plan_path.read_text(encoding="utf-8")
    plan_error = validate_plan_text(plan_text, display_path=str(plan_path))
    if plan_error:
        sys.stderr.write(f"viewer_host_prepare_slice: phased plan invalid: {plan_error}\n")
        return 2

    payload, result = load_and_validate_slice_contract(contract_path, repo_root)
    if payload is None or not result.ok:
        sys.stderr.write("viewer_host_prepare_slice: contract invalid\n")
        for error in result.errors:
            sys.stderr.write(f"- {error}\n")
        return 2

    if str(payload["plan_path"]).replace("\\", "/") != plan_path.relative_to(repo_root).as_posix():
        sys.stderr.write(
            "viewer_host_prepare_slice: contract plan_path does not match the provided phased plan\n"
        )
        return 2

    state_path = write_active_contract_state(args.session_id, repo_root=repo_root, contract_path=contract_path, contract_payload=payload)
    print(
        json.dumps(
            {
                "ok": True,
                "contract_id": payload["contract_id"],
                "contract_state": str(state_path.relative_to(repo_root).as_posix()),
                "banner": build_strict_banner(
                    {
                        "contract_id": payload["contract_id"],
                        "contract_path": contract_path.relative_to(repo_root).as_posix(),
                    }
                ),
            },
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
