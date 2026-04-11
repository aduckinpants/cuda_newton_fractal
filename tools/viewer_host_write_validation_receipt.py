from __future__ import annotations

import argparse
from pathlib import Path

from viewer_host_checkpoint_guard import write_validation_receipt


def main() -> int:
    parser = argparse.ArgumentParser(description="Write a validation receipt for the current clean HEAD")
    parser.add_argument("--summary", required=True, help="Short summary of what was validated")
    parser.add_argument("--command", action="append", dest="commands", help="Validation command that was run; may be repeated")
    parser.add_argument("--note", action="append", dest="notes", help="Optional note to store with the receipt; may be repeated")
    parser.add_argument("--repo-root", default=Path(__file__).resolve().parents[1], help="Repository root to record against")
    args = parser.parse_args()

    path = write_validation_receipt(
        args.summary,
        repo_root=Path(args.repo_root),
        commands=args.commands,
        notes=args.notes,
    )
    print(f"viewer_host_write_validation_receipt: wrote {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())