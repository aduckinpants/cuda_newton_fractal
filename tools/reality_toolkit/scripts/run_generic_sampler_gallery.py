from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer import run_sample_request, write_generic_sample_gallery


def main() -> int:
    parser = argparse.ArgumentParser(description="Run a generic.sample request and render grid or sequence_grid responses into PNG gallery frames.")
    parser.add_argument("--request-json", required=True, help="Path to a generic.sample grid or sequence_grid request JSON file.")
    parser.add_argument("--out-dir", required=True, help="Output directory for request/response JSON, gallery manifest, and PNG frames.")
    parser.add_argument("--exe-path", help="Optional explicit runtime path.")
    parser.add_argument("--timeout-seconds", type=float, default=180.0, help="Sample request timeout in seconds.")
    args = parser.parse_args()

    request_path = Path(args.request_json).resolve()
    out_dir = Path(args.out_dir).resolve()
    exe_path = Path(args.exe_path).resolve() if args.exe_path else None

    request = json.loads(request_path.read_text(encoding="utf-8"))
    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "request.json").write_text(json.dumps(request, indent=2), encoding="utf-8")

    response = run_sample_request(
        REPO_ROOT,
        request,
        exe_path=exe_path,
        timeout_seconds=args.timeout_seconds,
    )
    manifest = write_generic_sample_gallery(response, out_dir)
    print(json.dumps({
        "request_json": str(request_path),
        "out_dir": str(out_dir),
        "gallery_manifest": str(out_dir / "gallery_manifest.json"),
        "frame_count": manifest["frame_count"],
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())