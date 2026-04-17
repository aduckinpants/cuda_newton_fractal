from __future__ import annotations

import argparse
import sys
from pathlib import Path

try:
    from tools.viewer_host_checkpoint_guard import discover_repo_root
    from tools.viewer_host_contract_state import load_active_contract_state
    from tools.viewer_host_prepare_slice import main as prepare_slice_main
except ModuleNotFoundError:
    from viewer_host_checkpoint_guard import discover_repo_root
    from viewer_host_contract_state import load_active_contract_state
    from viewer_host_prepare_slice import main as prepare_slice_main


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Re-lock the active checked-in slice contract after an intentional contract edit")
    parser.add_argument("--session-id", required=True, help="Current host session id")
    parser.add_argument("--cwd", default=".", help="Repo cwd for resolution")
    parser.add_argument("--plan", required=True, help="Checked-in phased plan path")
    parser.add_argument("--contract", required=True, help="Checked-in machine-readable contract path")
    args = parser.parse_args(argv)

    repo_root = discover_repo_root(Path(args.cwd))
    state = load_active_contract_state(args.session_id, repo_root)
    if state is None:
        sys.stderr.write("viewer_host_revise_contract: no active contract state to revise\n")
        return 2
    return prepare_slice_main(
        [
            "--session-id",
            args.session_id,
            "--cwd",
            str(repo_root),
            "--plan",
            args.plan,
            "--contract",
            args.contract,
        ]
    )


if __name__ == "__main__":
    raise SystemExit(main())
