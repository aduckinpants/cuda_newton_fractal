from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer import load_fractal_extensions_manifest, run_fractal_extensions_composite


def main() -> int:
    parser = argparse.ArgumentParser(description="Capture and extend archived fractal findings with generic.sample sidecars.")
    parser.add_argument("--manifest-json", required=True, help="Manifest describing scenes, state bundles, and math sidecars.")
    parser.add_argument("--out-dir", help="Explicit output directory. Defaults to D:/salt-fractal/<repo>/findings/<group>/<date>.")
    parser.add_argument("--finding-group", help="Optional override for the manifest finding group.")
    parser.add_argument("--exe-path", help="Optional explicit runtime path.")
    parser.add_argument("--timeout-seconds", type=float, default=300.0, help="Capture and sample timeout in seconds.")
    parser.add_argument("--skip-analysis", action="store_true", help="Do not run the finding analyzer for captured scenes.")
    parser.add_argument("--dry-run", action="store_true", help="Write scene and request metadata without launching the runtime.")
    parser.add_argument("--overwrite", action="store_true", help="Replace existing scene output directories.")
    args = parser.parse_args()

    manifest_path = Path(args.manifest_json).resolve()
    manifest = load_fractal_extensions_manifest(manifest_path)
    out_dir = Path(args.out_dir).resolve() if args.out_dir else None
    exe_path = Path(args.exe_path).resolve() if args.exe_path else None

    summary = run_fractal_extensions_composite(
        repo_root=REPO_ROOT,
        manifest=manifest,
        manifest_path=manifest_path,
        out_dir=out_dir,
        finding_group=args.finding_group,
        analyze_findings=not args.skip_analysis,
        dry_run=args.dry_run,
        timeout_seconds=args.timeout_seconds,
        exe_path=exe_path,
        overwrite=args.overwrite,
    )
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())