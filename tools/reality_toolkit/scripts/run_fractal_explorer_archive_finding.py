from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys


REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer import archive_finding_bundle, diagnostics_last_dir, findings_root


def _default_repo_root() -> Path:
    return REPO_ROOT


def main() -> int:
    parser = argparse.ArgumentParser(description="Archive the current diagnostics bundle as a named fractal finding.")
    parser.add_argument("--repo-root", default=str(_default_repo_root()), help="Repository root used to resolve default diagnostics and findings paths.")
    parser.add_argument("--diagnostics-dir", help="Diagnostics directory to archive. Defaults to the repo runtime diagnostics/last folder.")
    parser.add_argument("--out-root", help="Directory that will contain the finding folder. Defaults to D:/salt-fractal/<repo>/findings.")
    parser.add_argument("--finding-id", required=True, help="Stable finding folder name, for example explaino_seed_0p375_shell_transition.")
    parser.add_argument("--why", required=True, help="Short importance note explaining why this frame is worth keeping.")
    parser.add_argument("--repro-command", required=True, help="Exact command line needed to reproduce the frame.")
    parser.add_argument("--overwrite", action="store_true", help="Replace an existing finding folder with the same finding id.")
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    diagnostics_dir = Path(args.diagnostics_dir).resolve() if args.diagnostics_dir else diagnostics_last_dir(repo_root)
    out_root = Path(args.out_root).resolve() if args.out_root else findings_root(repo_root)
    output_dir = out_root / args.finding_id

    archive_finding_bundle(
        diagnostics_dir=diagnostics_dir,
        output_dir=output_dir,
        finding_id=args.finding_id,
        why=args.why,
        repro_command=args.repro_command,
        overwrite=args.overwrite,
    )

    print(
        json.dumps(
            {
                "output_dir": str(output_dir),
                "frame_png": str(output_dir / "frame.png"),
                "state_json": str(output_dir / "state.json"),
                "sidecar": str(output_dir / "finding.md"),
                "field_notes": str(output_dir / "field-notes.md"),
            },
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())