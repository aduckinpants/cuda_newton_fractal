from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer.generic_equation_pack import run_equation_pack_workbench


def _parse_key_value(entries: list[str], label: str) -> dict[str, float]:
    parsed: dict[str, float] = {}
    for entry in entries:
        if "=" not in entry:
            raise SystemExit(f"{label} must use key=value syntax: {entry}")
        key, raw_value = entry.split("=", 1)
        key = key.strip()
        if not key:
            raise SystemExit(f"{label} key must not be empty: {entry}")
        try:
            parsed[key] = float(raw_value)
        except ValueError as exc:
            raise SystemExit(f"{label} value must be numeric: {entry}") from exc
    return parsed


def main() -> int:
    parser = argparse.ArgumentParser(description="Run a Generic CUDA equation pack through generic.sample and write a gallery.")
    parser.add_argument("--pack-json", required=True, type=Path)
    parser.add_argument("--out-dir", required=True, type=Path)
    parser.add_argument("--backend", choices=["default", "cpu", "cuda"], default="default")
    parser.add_argument("--control", action="append", default=[], help="Control or param override as id=value.")
    parser.add_argument("--center-x", type=float)
    parser.add_argument("--center-y", type=float)
    parser.add_argument("--span-x", type=float)
    parser.add_argument("--span-y", type=float)
    parser.add_argument("--grid-width", type=int)
    parser.add_argument("--grid-height", type=int)
    parser.add_argument("--exe-path", type=Path)
    parser.add_argument("--timeout-seconds", type=float, default=180.0)
    args = parser.parse_args()

    region_overrides: dict[str, object] = {}
    for attr, key in (
        ("center_x", "center_x"),
        ("center_y", "center_y"),
        ("span_x", "span_x"),
        ("span_y", "span_y"),
        ("grid_width", "grid_width"),
        ("grid_height", "grid_height"),
    ):
        value = getattr(args, attr)
        if value is not None:
            region_overrides[key] = value

    result = run_equation_pack_workbench(
        args.pack_json,
        args.out_dir,
        backend=args.backend,
        control_overrides=_parse_key_value(args.control, "--control"),
        region_overrides=region_overrides or None,
        exe_path=args.exe_path,
        timeout_seconds=args.timeout_seconds,
    )
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
