from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

try:
    from tools.viewer_host_contract_state import REPO_ROOT, load_and_validate_slice_contract
except ModuleNotFoundError:
    from viewer_host_contract_state import REPO_ROOT, load_and_validate_slice_contract


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate a machine-readable viewer-host slice contract")
    parser.add_argument("--contract", required=True, help="Path to the checked-in contract JSON")
    args = parser.parse_args(argv)

    contract_path = Path(args.contract)
    if not contract_path.is_absolute():
        contract_path = REPO_ROOT / contract_path
    if not contract_path.exists():
        sys.stderr.write(f"viewer_host_validate_slice_contract: missing contract: {contract_path}\n")
        return 2

    _payload, result = load_and_validate_slice_contract(contract_path, REPO_ROOT)
    if not result.ok:
        sys.stderr.write("viewer_host_validate_slice_contract: FAILED\n")
        for error in result.errors:
            sys.stderr.write(f"- {error}\n")
        return 2

    print(
        json.dumps(
            {
                "ok": True,
                "contract": str(contract_path.relative_to(REPO_ROOT).as_posix()),
            },
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
