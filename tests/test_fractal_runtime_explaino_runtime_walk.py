from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[1]
RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"


def _active_runtime_exe() -> Path:
    if not ACTIVE_RUNTIME_FILE.exists():
        pytest.skip(f"missing active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    active_name = ACTIVE_RUNTIME_FILE.read_text(encoding="utf-8").strip()
    if not active_name:
        pytest.skip(f"empty active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    exe_path = RUNTIME_DIR / active_name
    if not exe_path.exists():
        pytest.skip(f"active runtime missing: {exe_path}")
    return exe_path


def _write_state_json(path: Path) -> None:
    payload = {
        "state_version": 3,
        "fractal_type": "explaino_fp",
        "view": {
            "center_x": 0.0,
            "center_y": 0.0,
            "zoom": 1.0,
            "rotation_degrees": 0.0,
            "center_hp_x": 0.0,
            "center_hp_y": 0.0,
            "log2_zoom": 0.0,
            "explaino_phase": 0.0,
            "explaino_seed_drift": 0.0,
            "explaino_seed_tween": True,
            "auto_max_iter": False,
            "auto_increment_seed": False,
            "explaino_seed_rate": 0.001,
            "explaino_phase_strength": 0.0,
        },
        "params": {
            "max_iter": 500,
            "epsilon": 1.0e-6,
            "exposure": 1.0,
            "poly_kind": 2,
            "coloring_mode": "smooth_escape",
            "nova_alpha": 0.5,
            "phoenix_p_real": 0.0,
            "phoenix_p_imag": 0.0,
            "multibrot_power": 3,
            "multibrot_power_float": 3.0,
            "lambda_real": 0.0,
            "lambda_imag": 0.0,
            "explaino_seed": 0.0,
            "explaino_seed_b": 0.0,
            "explaino_mix": 0.0,
            "explaino_warp_strength": 0.0,
            "explaino_root_spread": 0.0,
            "explaino_root_count": 0,
            "poly_coeffs": [-1, 0, 0, 1, 0],
            "color_saturation": 1.0,
            "color_contrast": 1.0,
            "color_tint_r": 1.0,
            "color_tint_g": 1.0,
            "color_tint_b": 1.0,
        },
        "render": {
            "width": 320,
            "height": 240,
            "interaction_debounce_ms": 250,
            "preview_target_fps": 30.0,
            "preview_min_scale": 0.5,
            "block_size": 256,
            "device_id": 0,
        },
    }
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _write_bundle_json(path: Path) -> None:
    payload = {
        "version": 1,
        "field_name": "mr_zipper_branch",
        "samples": [
            {"id": "start", "t": 0.0, "channels": [0.0] * 13},
            {"id": "mid", "t": 0.5, "channels": [0.4, 0.2, 0.8, 0.3, 0.6, 0.75, 0.9, 0.1, 0.2, 0.7, 0.3, 0.4, 1.0]},
            {"id": "end", "t": 1.0, "channels": [1.0] * 13},
        ],
        "branch_markers": [
            {"id": "fork_a", "label": "fork-a", "parent_id": "main", "t": 0.5, "sticky_radius": 0.1}
        ],
    }
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def test_runtime_walk_tool_regenerates_trace_bundle(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("runtime walk regression is Windows-only")

    _active_runtime_exe()
    state_path = tmp_path / "state.json"
    bundle_path = tmp_path / "bundle.json"
    out_dir = tmp_path / "runtime_walk"
    request_path = tmp_path / "runtime_walk_request.json"
    stale_manifest_path = out_dir / "runtime_walk_branch_manifest.json"

    _write_state_json(state_path)
    _write_bundle_json(bundle_path)
    out_dir.mkdir(parents=True, exist_ok=True)
    stale_manifest_path.write_text("{\"stale\": true}\n", encoding="utf-8")
    stale_manifest_mtime = stale_manifest_path.stat().st_mtime
    request_path.write_text(
        json.dumps(
                {
                    "version": 1,
                    "base_state_json": str(state_path),
                    "bundle_json": str(bundle_path),
                    "out_dir": str(out_dir),
                    "ticks": 5,
                },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )

    result = subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "explaino_runtime_walk.py"),
            "--runtime-dir",
            str(RUNTIME_DIR),
            "--request-json",
            str(request_path),
        ],
        cwd=str(REPO_ROOT),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout

    report_path = out_dir / "runtime_walk_report.json"
    branch_manifest_path = out_dir / "runtime_walk_branch_manifest.json"
    assert report_path.exists()
    assert branch_manifest_path.exists()
    assert branch_manifest_path.stat().st_mtime > stale_manifest_mtime
    assert (out_dir / "runtime_walk_trace.csv").exists()
    assert (out_dir / "runtime_walk_trace.obj").exists()
    assert (out_dir / "runtime_walk_trace.stl").exists()
    assert (out_dir / "runtime_walk_trace_overlay.bmp").exists()
    assert (out_dir / "ticks" / "tick_0000" / "state.json").exists()
    assert (out_dir / "ticks" / "tick_0000" / "frame.bmp").exists()

    report = json.loads(report_path.read_text(encoding="utf-8"))
    assert report["summary"]["tick_count"] == 5
    assert len(report["trace"]) == 5
    assert report["trace"][2]["branch"]["nearest_marker_id"] == "fork_a"
    assert report["trace"][2]["branch"]["sticky"] is True
