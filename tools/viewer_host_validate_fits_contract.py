from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

try:
    from tools.viewer_host_contract_state import REPO_ROOT, load_json_file
except ModuleNotFoundError:
    from viewer_host_contract_state import REPO_ROOT, load_json_file


DEFAULT_MAPPING_PATH = REPO_ROOT / "ui" / "runtime_walk_fits_mapping_profiles_v1.json"
BOOTSTRAP_HEADER_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk_bootstrap.h"
VIEWER_IMGUI_PATH = REPO_ROOT / "ui_app" / "src" / "runtime_walk_viewer_imgui.cpp"


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate the shipped FITS runtime-walk contract defaults")
    parser.add_argument("--contract", required=True, help="Path to the runtime_walk_fits contract JSON")
    parser.add_argument("--out-json", help="Optional JSON output path")
    args = parser.parse_args(argv)

    contract_path = Path(args.contract)
    if not contract_path.is_absolute():
        contract_path = REPO_ROOT / contract_path
    if not contract_path.exists():
        sys.stderr.write(f"viewer_host_validate_fits_contract: missing contract: {contract_path}\n")
        return 2

    contract_payload = load_json_file(contract_path)
    mapping_payload = load_json_file(DEFAULT_MAPPING_PATH)
    header_text = BOOTSTRAP_HEADER_PATH.read_text(encoding="utf-8")
    imgui_text = VIEWER_IMGUI_PATH.read_text(encoding="utf-8")

    errors: list[str] = []
    bindings = []
    for profile in mapping_payload.get("profiles", []):
        bindings.extend(profile.get("bindings", []))

    checks = {
        "default_mapping_has_no_warp_binding": not any(
            binding.get("target_path") == "params.explaino_warp_strength" for binding in bindings
        ),
        "default_synthesized_fractal_is_explaino": 'FractalType base_fractal_type = FractalType::explaino;' in header_text,
        "default_ui_has_no_warp_control": '"Warp Motion"' not in imgui_text,
    }
    if not checks["default_mapping_has_no_warp_binding"]:
        errors.append("default FITS mapping still binds params.explaino_warp_strength")
    if not checks["default_synthesized_fractal_is_explaino"]:
        errors.append("runtime_walk_bootstrap.h does not default base_fractal_type to explaino")
    if not checks["default_ui_has_no_warp_control"]:
        errors.append("runtime_walk_viewer_imgui.cpp still exposes Warp Motion in the default UI")

    expected_default = str(contract_payload.get("required_defaults", {}).get("base_fractal_type", ""))
    if expected_default and expected_default != "explaino":
        errors.append("runtime_walk_fits contract no longer declares explaino as the required default fractal")

    if errors:
        sys.stderr.write("viewer_host_validate_fits_contract: FAILED\n")
        for error in errors:
            sys.stderr.write(f"- {error}\n")
        return 2

    payload = {
        "ok": True,
        "contract": str(contract_path.relative_to(REPO_ROOT).as_posix()),
        "mapping": str(DEFAULT_MAPPING_PATH.relative_to(REPO_ROOT).as_posix()),
        "checks": checks,
    }
    if args.out_json:
        out_path = Path(args.out_json)
        if not out_path.is_absolute():
            out_path = REPO_ROOT / out_path
        out_path.parent.mkdir(parents=True, exist_ok=True)
        out_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(payload, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
