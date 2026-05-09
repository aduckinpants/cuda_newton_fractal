from __future__ import annotations

import hashlib
import json
import math
import subprocess
import sys
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
DIAGNOSTICS_STATE_FILE = RUNTIME_DIR / "diagnostics" / "last" / "state.json"
DIAGNOSTICS_FRAME_FILE = RUNTIME_DIR / "diagnostics" / "last" / "frame.bmp"


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


def _run_headless_capture(*args: str) -> dict[str, object]:
    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)

    result = subprocess.run(
        list(args),
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"
    assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing diagnostics frame file: {DIAGNOSTICS_FRAME_FILE}"
    frame_bytes = DIAGNOSTICS_FRAME_FILE.read_bytes()

    return {
        "state": json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8")),
        "frame_hash": hashlib.sha256(frame_bytes).hexdigest(),
        "frame_bytes": frame_bytes,
    }


def _bmp_pixel_bytes(frame_bytes: bytes) -> bytes:
    if len(frame_bytes) < 14:
        raise AssertionError("frame bytes are too short to be a BMP")
    pixel_offset = int.from_bytes(frame_bytes[10:14], byteorder="little", signed=False)
    if pixel_offset <= 0 or pixel_offset > len(frame_bytes):
        raise AssertionError(f"invalid BMP pixel offset: {pixel_offset}")
    return frame_bytes[pixel_offset:]


def _mean_absolute_frame_delta(left_frame_bytes: bytes, right_frame_bytes: bytes) -> float:
    left_pixels = _bmp_pixel_bytes(left_frame_bytes)
    right_pixels = _bmp_pixel_bytes(right_frame_bytes)
    assert len(left_pixels) == len(right_pixels), "captured frames must have identical pixel payload size"
    if not left_pixels:
        return 0.0
    total_delta = sum(abs(left_byte - right_byte) for left_byte, right_byte in zip(left_pixels, right_pixels))
    return total_delta / float(len(left_pixels))


def _distinct_rgb_triplet_count(frame_bytes: bytes) -> int:
    pixels = _bmp_pixel_bytes(frame_bytes)
    return len({pixels[index:index + 3] for index in range(0, len(pixels), 4)})


def _non_black_rgb_triplet_count(frame_bytes: bytes) -> int:
    pixels = _bmp_pixel_bytes(frame_bytes)
    return sum(1 for index in range(0, len(pixels), 4) if pixels[index:index + 3] != b"\x00\x00\x00")


def _write_state_bundle(tmp_path: Path, state: dict[str, object]) -> Path:
    state_path = tmp_path / "state.json"
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
    return state_path


def _run_exploration_advisor(*args: str, report_path: Path) -> dict[str, object]:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.unlink(missing_ok=True)

    result = subprocess.run(
        list(args),
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert report_path.exists(), f"missing advisor report: {report_path}"
    return json.loads(report_path.read_text(encoding="utf-8"))


def _run_exploration_advisor_stdout(*args: str) -> dict[str, object]:
    result = subprocess.run(
        list(args),
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert result.stdout.strip(), "missing advisor stdout payload"
    return json.loads(result.stdout)


def _run_exploration_advisor_failure(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(args),
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )


def _configure_sidecar_policy(state: dict[str, object], **updates: object) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    policy = configured_state["sidecar_auto_demo_policy"]
    assert isinstance(policy, dict)
    policy.update(updates)
    return configured_state


def _with_sidecar_mutation_history(
    state: dict[str, object], history: list[dict[str, object]], **param_updates: object
) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    configured_state["sidecar_mutation_history"] = history
    params = configured_state["params"]
    assert isinstance(params, dict)
    params.update(param_updates)
    return configured_state


def _with_explaino_programmable_color_state(
    state: dict[str, object], *, palette: str, **param_updates: object
) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    params = configured_state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "coloring_mode": "smooth_escape",
            "color_signal": "smooth_escape",
            "color_shape": "identity",
            "color_palette": palette,
            "color_grading": "escape_default",
            "color_smooth_escape_scale": 1.0,
            "color_smooth_escape_bias": 0.0,
            "color_heatmap_cycle_scale": 1.0,
            "color_heatmap_saturation": 1.0,
            "color_contrast_lift_exposure": 1.0,
            "color_contrast_lift_saturation": 1.0,
            "color_explaino_palette_seed_scale": 1.0,
            "color_explaino_palette_seed_phase": 0.0,
            "color_explaino_palette_colorfulness": 1.0,
        }
    )
    params.update(param_updates)
    return configured_state


def _with_explaino_root_proximity_color_state(
    state: dict[str, object], *, palette: str = "cyclic_escape", **param_updates: object
) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    params = configured_state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "coloring_mode": "smooth_escape",
            "color_signal": "root_proximity",
            "color_shape": "identity",
            "color_palette": palette,
            "color_grading": "escape_default",
            "color_root_proximity_scale": 0.5,
            "color_root_proximity_bias": 0.0,
            "color_explaino_palette_seed_scale": 1.0,
            "color_explaino_palette_seed_phase": 0.0,
            "color_explaino_palette_colorfulness": 1.0,
        }
    )
    params.update(param_updates)
    return configured_state


def _with_explaino_escape_magnitude_color_state(
    state: dict[str, object], *, palette: str = "cyclic_escape", **param_updates: object
) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    params = configured_state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "coloring_mode": "smooth_escape",
            "color_signal": "escape_magnitude",
            "color_shape": "identity",
            "color_palette": palette,
            "color_grading": "escape_default",
            "color_escape_magnitude_scale": 1.0,
            "color_escape_magnitude_bias": 0.0,
            "color_heatmap_cycle_scale": 1.0,
            "color_heatmap_saturation": 1.0,
            "color_contrast_lift_exposure": 1.0,
            "color_contrast_lift_saturation": 1.0,
            "color_explaino_palette_seed_scale": 1.0,
            "color_explaino_palette_seed_phase": 0.0,
            "color_explaino_palette_colorfulness": 1.0,
        }
    )
    params.update(param_updates)
    return configured_state


def _with_explaino_repeat_heatmap_draft_state(state: dict[str, object]) -> dict[str, object]:
    configured_state = _with_explaino_programmable_color_state(
        state,
        palette="cyclic_escape",
        color_shape="repeat",
        color_smooth_escape_scale=1.25,
        color_smooth_escape_bias=0.1,
        color_shape_repeat_frequency=4.0,
        color_shape_repeat_phase=0.2,
        color_heatmap_cycle_scale=0.5,
        color_heatmap_saturation=1.25,
    )
    configured_state["color_pipeline_draft"] = {
        "next_row_id": 14,
        "lanes": [
            {
                "lane_id": "source",
                "label": "Source",
                "rows": [
                    {
                        "ui_row_id": 11,
                        "enabled": True,
                        "function_id": "smooth_escape_ramp",
                        "parameter_values": [
                            {"path": "signal.scale", "type": "float", "number_value": 1.25},
                            {"path": "signal.bias", "type": "float", "number_value": 0.1},
                        ],
                    }
                ],
            },
            {
                "lane_id": "shape",
                "label": "Shape",
                "rows": [
                    {
                        "ui_row_id": 12,
                        "enabled": True,
                        "function_id": "repeat",
                        "parameter_values": [
                            {"path": "shape.frequency", "type": "float", "number_value": 4.0},
                            {"path": "shape.phase", "type": "float", "number_value": 0.2},
                        ],
                    }
                ],
            },
            {
                "lane_id": "palette",
                "label": "Palette",
                "rows": [
                    {
                        "ui_row_id": 13,
                        "enabled": True,
                        "function_id": "heatmap",
                        "parameter_values": [
                            {"path": "palette.cycle_scale", "type": "float", "number_value": 0.5},
                            {"path": "palette.saturation", "type": "float", "number_value": 1.25},
                        ],
                    }
                ],
            },
        ],
    }
    return configured_state


def _with_explaino_root_proximity_repeat_heatmap_draft_state(state: dict[str, object]) -> dict[str, object]:
    configured_state = _with_explaino_root_proximity_color_state(
        state,
        palette="cyclic_escape",
        color_shape="repeat",
        color_root_proximity_scale=0.75,
        color_root_proximity_bias=0.15,
        color_shape_repeat_frequency=4.0,
        color_shape_repeat_phase=0.2,
        color_heatmap_cycle_scale=0.5,
        color_heatmap_saturation=1.25,
    )
    configured_state["color_pipeline_draft"] = {
        "next_row_id": 24,
        "lanes": [
            {
                "lane_id": "source",
                "label": "Source",
                "rows": [
                    {
                        "ui_row_id": 21,
                        "enabled": True,
                        "function_id": "root_proximity",
                        "parameter_values": [
                            {"path": "signal.proximity_scale", "type": "float", "number_value": 0.75},
                            {"path": "signal.proximity_bias", "type": "float", "number_value": 0.15},
                        ],
                    }
                ],
            },
            {
                "lane_id": "shape",
                "label": "Shape",
                "rows": [
                    {
                        "ui_row_id": 22,
                        "enabled": True,
                        "function_id": "repeat",
                        "parameter_values": [
                            {"path": "shape.frequency", "type": "float", "number_value": 4.0},
                            {"path": "shape.phase", "type": "float", "number_value": 0.2},
                        ],
                    }
                ],
            },
            {
                "lane_id": "palette",
                "label": "Palette",
                "rows": [
                    {
                        "ui_row_id": 23,
                        "enabled": True,
                        "function_id": "heatmap",
                        "parameter_values": [
                            {"path": "palette.cycle_scale", "type": "float", "number_value": 0.5},
                            {"path": "palette.saturation", "type": "float", "number_value": 1.25},
                        ],
                    }
                ],
            },
        ],
    }
    return configured_state


def _color_pipeline_first_row(state: dict[str, object], lane_id: str) -> dict[str, object]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), "expected captured state to include color_pipeline_draft"
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), "expected color_pipeline_draft.lanes to be a list"
    for lane in lanes:
        if not isinstance(lane, dict):
            continue
        if lane.get("lane_id") != lane_id:
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list) and rows, f"expected lane {lane_id} to include at least one row"
        row = rows[0]
        assert isinstance(row, dict), f"expected lane {lane_id} row to be an object"
        return row
    raise AssertionError(f"missing color pipeline lane: {lane_id}")


def _color_pipeline_number_param(row: dict[str, object], path: str) -> float:
    parameter_values = row.get("parameter_values")
    assert isinstance(parameter_values, list), "expected parameter_values to be a list"
    for param in parameter_values:
        if not isinstance(param, dict):
            continue
        if param.get("path") != path:
            continue
        value = param.get("number_value")
        assert isinstance(value, (int, float)), f"expected numeric value for {path}"
        return float(value)
    raise AssertionError(f"missing color pipeline param: {path}")


def _with_view_camera(state: dict[str, object], *, center_x: float, center_y: float, zoom: float) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    view = configured_state["view"]
    assert isinstance(view, dict)
    safe_zoom = max(1.0e-30, zoom)
    view.update(
        {
            "center_x": center_x,
            "center_y": center_y,
            "zoom": safe_zoom,
            "center_hp_x": center_x,
            "center_hp_y": center_y,
            "log2_zoom": math.log2(safe_zoom),
            "auto_max_iter": False,
        }
    )
    return configured_state


def _with_explaino_root_basin_palette_state(
    state: dict[str, object], *, palette: str = "root_classic", **param_updates: object
) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    params = configured_state["params"]
    assert isinstance(params, dict)
    coloring_mode = "joy_basins" if palette == "joy" else "root_basin"
    params.update(
        {
            "coloring_mode": coloring_mode,
            "color_signal": "root_index",
            "color_shape": "identity",
            "color_palette": palette,
            "color_grading": "basin_default",
        }
    )
    params.update(param_updates)
    return configured_state


def _numeric_state_deltas(before: dict[str, object], after: dict[str, object], *, abs_tol: float = 1.0e-7) -> list[str]:
    changed: list[str] = []
    for section_name in ("view", "params"):
        before_section = before[section_name]
        after_section = after[section_name]
        assert isinstance(before_section, dict)
        assert isinstance(after_section, dict)
        for key, before_value in before_section.items():
            after_value = after_section.get(key)
            if not isinstance(before_value, (int, float)) or isinstance(before_value, bool):
                continue
            if not isinstance(after_value, (int, float)) or isinstance(after_value, bool):
                continue
            if abs(float(after_value) - float(before_value)) > abs_tol:
                changed.append(f"{section_name}.{key}")
    return changed


def test_explaino_lambda_cli_overrides_survive_headless_capture() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino-Lambda runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "explaino_lambda",
            "--lambda-real",
            "2.9685855",
            "--lambda-imag",
            "-0.27446103",
            "--explaino-seed",
            "3.25",
            "--explaino-warp-strength",
            "0.2",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"

    state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    assert state["fractal_type"] == "explaino_lambda"
    assert state["params"]["lambda_real"] == pytest.approx(2.9685855, abs=1e-5)
    assert state["params"]["lambda_imag"] == pytest.approx(-0.27446103, abs=1e-5)
    assert state["params"]["explaino_seed"] == 3
    assert state["view"]["explaino_seed_drift"] == pytest.approx(0.25, abs=1e-6)
    assert state["params"]["explaino_warp_strength"] == pytest.approx(0.2, abs=1e-6)
    assert state["params"]["coloring_mode"] == "smooth_escape"


def test_explaino_rational_escape_cli_overrides_survive_headless_capture() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino-Rational-Escape runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "explaino_rational_escape",
            "--explaino-seed",
            "4.2",
            "--explaino-warp-strength",
            "0.35",
            "--width",
            "640",
            "--height",
            "480",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"

    state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    assert state["fractal_type"] == "explaino_rational_escape"
    assert state["params"]["explaino_seed"] == 4
    assert state["view"]["explaino_seed_drift"] == pytest.approx(0.2, abs=1e-6)
    assert state["params"]["explaino_warp_strength"] == pytest.approx(0.35, abs=1e-6)
    assert state["params"]["coloring_mode"] == "smooth_escape"
    assert state["render"]["width"] == 640
    assert state["render"]["height"] == 480


def test_explaino_composed_variants_render_distinct_default_frames() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino composed runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    hashes: dict[str, str] = {}
    expected_strengths = {
        "explaino_ripple": {
            "ripple_amplitude": 0.15,
            "splice_offset": 0.0,
            "vortex_strength": 0.0,
            "tension_strength": 0.0,
        },
        "explaino_splice": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.5,
            "vortex_strength": 0.0,
            "tension_strength": 0.0,
        },
        "explaino_vortex": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.0,
            "vortex_strength": 0.3,
            "tension_strength": 0.0,
        },
        "explaino_tension": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.0,
            "vortex_strength": 0.0,
            "tension_strength": 0.02,
        },
    }
    for variant, expected_params in expected_strengths.items():
        capture = _run_headless_capture(
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            variant,
            "--width",
            "320",
            "--height",
            "240",
        )
        state = capture["state"]
        assert state["fractal_type"] == variant
        for param_name, expected_value in expected_params.items():
            assert state["params"][param_name] == pytest.approx(expected_value, abs=1e-6)
        hashes[variant] = str(capture["frame_hash"])

    assert len(set(hashes.values())) == len(hashes), f"expected distinct default frame hashes, got {hashes}"


def test_explaino_composed_variant_state_round_trips_through_load_state_json(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino composed state round-trip regression is Windows-only")

    exe_path = _active_runtime_exe()
    initial_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_vortex",
        "--width",
        "320",
        "--height",
        "240",
    )
    saved_state_path = tmp_path / "state.json"
    saved_state_path.write_bytes(DIAGNOSTICS_STATE_FILE.read_bytes())

    reloaded_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(saved_state_path),
        "--capture-diagnostic",
    )

    initial_state = initial_capture["state"]
    reloaded_state = reloaded_capture["state"]
    assert reloaded_state["fractal_type"] == "explaino_vortex"
    assert reloaded_state["params"]["vortex_strength"] == pytest.approx(0.3, abs=1e-6)
    assert reloaded_state["params"]["ripple_amplitude"] == pytest.approx(0.0, abs=1e-6)
    assert "sidecar_orientation" in initial_state
    assert reloaded_state["sidecar_orientation"] == initial_state["sidecar_orientation"]
    assert "sidecar_auto_demo_policy" in initial_state
    assert reloaded_state["sidecar_auto_demo_policy"] == initial_state["sidecar_auto_demo_policy"]
    assert reloaded_capture["frame_hash"] == initial_capture["frame_hash"]
    assert reloaded_state["render"]["width"] == initial_state["render"]["width"]
    assert reloaded_state["render"]["height"] == initial_state["render"]["height"]


def test_explaino_programmable_color_pipeline_changes_published_runtime_frame(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino programmable runtime color regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    heatmap_baseline_state = _with_explaino_programmable_color_state(
        baseline_capture["state"],
        palette="cyclic_escape",
    )
    heatmap_baseline_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "heatmap_baseline", heatmap_baseline_state)),
        "--capture-diagnostic",
    )

    heatmap_shifted_state = _with_explaino_programmable_color_state(
        baseline_capture["state"],
        palette="cyclic_escape",
        color_heatmap_cycle_scale=2.0,
    )
    heatmap_shifted_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "heatmap_shifted", heatmap_shifted_state)),
        "--capture-diagnostic",
    )

    heatmap_shifted_params = heatmap_shifted_capture["state"]["params"]
    assert isinstance(heatmap_shifted_params, dict)
    assert heatmap_shifted_params["coloring_mode"] == "smooth_escape"
    assert heatmap_shifted_params["color_palette"] == "cyclic_escape"
    assert heatmap_shifted_params["color_heatmap_cycle_scale"] == pytest.approx(2.0, abs=1e-6)
    assert heatmap_shifted_capture["frame_hash"] != heatmap_baseline_capture["frame_hash"], (
        "expected Explaino smooth_escape heatmap cycle scale to change the published runtime frame hash"
    )

    explaino_baseline_state = _with_explaino_programmable_color_state(
        baseline_capture["state"],
        palette="explaino_cmap",
    )
    explaino_baseline_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "explaino_palette_baseline", explaino_baseline_state)),
        "--capture-diagnostic",
    )

    explaino_shifted_state = _with_explaino_programmable_color_state(
        baseline_capture["state"],
        palette="explaino_cmap",
        color_explaino_palette_seed_phase=0.25,
    )
    explaino_shifted_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "explaino_palette_shifted", explaino_shifted_state)),
        "--capture-diagnostic",
    )

    explaino_shifted_params = explaino_shifted_capture["state"]["params"]
    assert isinstance(explaino_shifted_params, dict)
    assert explaino_shifted_params["coloring_mode"] == "smooth_escape"
    assert explaino_shifted_params["color_palette"] == "explaino_cmap"
    assert explaino_shifted_params["color_explaino_palette_seed_phase"] == pytest.approx(0.25, abs=1e-6)
    assert explaino_shifted_capture["frame_hash"] != explaino_baseline_capture["frame_hash"], (
        "expected Explaino smooth_escape explaino_cmap seed phase to change the published runtime frame hash"
    )


def test_explaino_root_proximity_programmable_color_pipeline_changes_published_runtime_frame(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino root-proximity runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    proximity_baseline_state = _with_explaino_root_proximity_color_state(
        baseline_capture["state"],
        palette="cyclic_escape",
        color_root_proximity_scale=0.5,
    )
    proximity_baseline_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "root_proximity_baseline", proximity_baseline_state)),
        "--capture-diagnostic",
    )

    proximity_shifted_state = _with_explaino_root_proximity_color_state(
        baseline_capture["state"],
        palette="cyclic_escape",
        color_root_proximity_scale=6.0,
    )
    proximity_shifted_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "root_proximity_shifted", proximity_shifted_state)),
        "--capture-diagnostic",
    )

    shifted_params = proximity_shifted_capture["state"]["params"]
    assert isinstance(shifted_params, dict)
    assert shifted_params["coloring_mode"] == "smooth_escape"
    assert shifted_params["color_signal"] == "root_proximity"
    assert shifted_params["color_palette"] == "cyclic_escape"
    assert shifted_params["color_root_proximity_scale"] == pytest.approx(6.0, abs=1e-6)
    assert _distinct_rgb_triplet_count(proximity_baseline_capture["frame_bytes"]) > 8, (
        "expected Explaino root_proximity baseline capture to contain visible color variation instead of collapsing to one flat color"
    )
    assert _distinct_rgb_triplet_count(proximity_shifted_capture["frame_bytes"]) > 8, (
        "expected Explaino root_proximity shifted capture to contain visible color variation instead of collapsing to one flat color"
    )
    assert _non_black_rgb_triplet_count(proximity_baseline_capture["frame_bytes"]) > 0, (
        "expected Explaino root_proximity baseline capture to retain non-black pixels"
    )
    assert _non_black_rgb_triplet_count(proximity_shifted_capture["frame_bytes"]) > 0, (
        "expected Explaino root_proximity shifted capture to retain non-black pixels"
    )
    assert proximity_shifted_capture["frame_hash"] != proximity_baseline_capture["frame_hash"], (
        "expected Explaino root_proximity scale to change the published runtime frame hash"
    )


def test_explaino_headless_color_pipeline_function_switch_changes_draft_state_and_frame(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino advanced-color runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    draft_state = _with_explaino_repeat_heatmap_draft_state(baseline_capture["state"])
    draft_state_path = _write_state_bundle(tmp_path / "headless_color_pipeline_switch", draft_state)

    baseline_draft_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_state_path),
        "--capture-diagnostic",
    )

    switched_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_state_path),
        "--color-pipeline-select-function",
        "source:0:root_proximity",
        "--capture-diagnostic",
    )

    switched_params = switched_capture["state"]["params"]
    assert isinstance(switched_params, dict)
    assert switched_params["color_signal"] == "root_proximity"
    assert switched_params["color_palette"] == "cyclic_escape"
    assert switched_params["color_shape"] == "repeat"
    assert switched_params["color_shape_repeat_frequency"] == pytest.approx(4.0, abs=1e-6)
    assert switched_params["color_shape_repeat_phase"] == pytest.approx(0.2, abs=1e-6)
    assert switched_params["color_heatmap_cycle_scale"] == pytest.approx(0.5, abs=1e-6)
    assert switched_params["color_heatmap_saturation"] == pytest.approx(1.25, abs=1e-6)

    switched_draft = switched_capture["state"].get("color_pipeline_draft")
    assert isinstance(switched_draft, dict)
    assert switched_draft["next_row_id"] == 14

    switched_source_row = _color_pipeline_first_row(switched_capture["state"], "source")
    assert switched_source_row["ui_row_id"] == 11
    assert switched_source_row["function_id"] == "root_proximity"

    switched_shape_row = _color_pipeline_first_row(switched_capture["state"], "shape")
    assert switched_shape_row["ui_row_id"] == 12
    assert switched_shape_row["function_id"] == "repeat"
    assert _color_pipeline_number_param(switched_shape_row, "shape.frequency") == pytest.approx(4.0, abs=1e-6)
    assert _color_pipeline_number_param(switched_shape_row, "shape.phase") == pytest.approx(0.2, abs=1e-6)

    switched_palette_row = _color_pipeline_first_row(switched_capture["state"], "palette")
    assert switched_palette_row["ui_row_id"] == 13
    assert switched_palette_row["function_id"] == "heatmap"
    assert _color_pipeline_number_param(switched_palette_row, "palette.cycle_scale") == pytest.approx(0.5, abs=1e-6)
    assert _color_pipeline_number_param(switched_palette_row, "palette.saturation") == pytest.approx(1.25, abs=1e-6)

    assert switched_capture["frame_hash"] != baseline_draft_capture["frame_hash"], (
        "expected a headless advanced-color function switch to change the published runtime frame while preserving the loaded repeat/heatmap draft rows"
    )


def test_loaded_color_pipeline_draft_resyncs_when_cli_fractal_override_invalidates_it(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino advanced-color runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    draft_state = _with_explaino_root_proximity_repeat_heatmap_draft_state(baseline_capture["state"])
    draft_state_path = _write_state_bundle(tmp_path / "override_invalidated_color_pipeline_draft", draft_state)

    override_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_state_path),
        "--fractal-type",
        "mandelbrot",
        "--capture-diagnostic",
    )

    assert override_capture["state"]["fractal_type"] == "mandelbrot"
    override_params = override_capture["state"]["params"]
    assert isinstance(override_params, dict)
    assert override_params["color_signal"] != "root_proximity"

    override_source_row = _color_pipeline_first_row(override_capture["state"], "source")
    assert override_source_row["function_id"] != "root_proximity", (
        "expected CLI fractal overrides that invalidate a saved advanced-color draft to resynchronize the captured draft away from disallowed root_proximity"
    )


def test_explaino_nearby_zoom_state_round_trips_and_stays_visible_in_published_runtime(tmp_path: Path) -> None:
    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "32",
        "--height",
        "32",
    )

    view = baseline_capture["state"]["view"]
    assert isinstance(view, dict)
    center_x = float(view["center_hp_x"])
    center_y = float(view["center_hp_y"])
    base_zoom = float(view["zoom"])
    nearby_zoom = base_zoom * 1.02

    base_state = _with_explaino_programmable_color_state(
        _with_view_camera(baseline_capture["state"], center_x=center_x, center_y=center_y, zoom=base_zoom),
        palette="cyclic_escape",
        color_heatmap_cycle_scale=0.25,
        max_iter=64,
    )
    base_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "zoom_continuity_base", base_state)),
        "--capture-diagnostic",
    )

    nearby_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(
            _write_state_bundle(
                tmp_path / "zoom_continuity_nearby",
                _with_view_camera(base_state, center_x=center_x, center_y=center_y, zoom=nearby_zoom),
            )
        ),
        "--capture-diagnostic",
    )

    for label, capture in {
        "base": base_capture,
        "nearby": nearby_capture,
    }.items():
        assert _non_black_rgb_triplet_count(capture["frame_bytes"]) > 0, (
            f"expected Explaino {label} zoom continuity witness to retain non-black pixels"
        )
        assert _distinct_rgb_triplet_count(capture["frame_bytes"]) > 8, (
            f"expected Explaino {label} zoom continuity witness to retain visible color structure"
        )

    nearby_view = nearby_capture["state"]["view"]
    assert isinstance(nearby_view, dict)
    assert nearby_view["zoom"] == pytest.approx(nearby_zoom, rel=1.0e-6, abs=1.0e-6)
    assert nearby_view["log2_zoom"] == pytest.approx(math.log2(nearby_zoom), rel=1.0e-6, abs=1.0e-6)
    assert nearby_capture["frame_hash"] != base_capture["frame_hash"], (
        "expected a nearby zoom edit to change the published runtime frame without collapsing the camera state"
    )


def test_explaino_escape_magnitude_ignores_residual_exit_threshold_in_published_runtime(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino escape_magnitude runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "16",
        "--height",
        "16",
    )

    camera_state = _with_view_camera(
        baseline_capture["state"],
        center_x=1.0,
        center_y=0.0,
        zoom=2048.0,
    )

    escape_base_state = _with_explaino_escape_magnitude_color_state(
        camera_state,
        palette="cyclic_escape",
        color_escape_magnitude_scale=0.25,
        color_heatmap_cycle_scale=0.25,
        max_iter=64,
    )
    escape_tight_state = _with_explaino_escape_magnitude_color_state(escape_base_state, epsilon=1.0e-6)
    escape_tight_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "escape_magnitude_epsilon_tight", escape_tight_state)),
        "--capture-diagnostic",
    )

    escape_loose_state = _with_explaino_escape_magnitude_color_state(escape_base_state, epsilon=1.0e-3)
    escape_loose_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "escape_magnitude_epsilon_loose", escape_loose_state)),
        "--capture-diagnostic",
    )

    smooth_base_state = _with_explaino_programmable_color_state(
        camera_state,
        palette="cyclic_escape",
        color_smooth_escape_scale=1.0,
        color_heatmap_cycle_scale=0.25,
        max_iter=64,
    )
    smooth_tight_state = _with_explaino_programmable_color_state(
        smooth_base_state,
        palette="cyclic_escape",
        epsilon=1.0e-6,
    )
    smooth_tight_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "smooth_escape_epsilon_tight", smooth_tight_state)),
        "--capture-diagnostic",
    )

    smooth_loose_state = _with_explaino_programmable_color_state(
        smooth_base_state,
        palette="cyclic_escape",
        epsilon=1.0e-3,
    )
    smooth_loose_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "smooth_escape_epsilon_loose", smooth_loose_state)),
        "--capture-diagnostic",
    )

    tight_params = escape_tight_capture["state"]["params"]
    loose_params = escape_loose_capture["state"]["params"]
    assert isinstance(tight_params, dict)
    assert isinstance(loose_params, dict)
    assert tight_params["color_signal"] == "escape_magnitude"
    assert loose_params["color_signal"] == "escape_magnitude"
    assert tight_params["epsilon"] == pytest.approx(1.0e-6, abs=1.0e-12)
    assert loose_params["epsilon"] == pytest.approx(1.0e-3, abs=1.0e-12)
    escape_delta = _mean_absolute_frame_delta(escape_tight_capture["frame_bytes"], escape_loose_capture["frame_bytes"])
    smooth_delta = _mean_absolute_frame_delta(smooth_tight_capture["frame_bytes"], smooth_loose_capture["frame_bytes"])
    assert smooth_delta > 0.0, "expected smooth_escape to remain sensitive to epsilon changes in the published runtime witness"
    assert escape_delta < smooth_delta * 0.25, (
        "expected Explaino escape_magnitude to react far less than smooth_escape to residual-sensitive exit-threshold changes in the published runtime witness; "
        f"escape_delta={escape_delta:.6f}, smooth_delta={smooth_delta:.6f}"
    )


def test_explaino_root_classic_and_joy_palettes_render_distinct_published_runtime_frames(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino basin palette runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    root_classic_state = _with_explaino_root_basin_palette_state(
        baseline_capture["state"],
        palette="root_classic",
    )
    root_classic_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "root_classic_palette", root_classic_state)),
        "--capture-diagnostic",
    )

    joy_state = _with_explaino_root_basin_palette_state(
        baseline_capture["state"],
        palette="joy",
    )
    joy_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "joy_palette", joy_state)),
        "--capture-diagnostic",
    )

    root_params = root_classic_capture["state"]["params"]
    joy_params = joy_capture["state"]["params"]
    assert isinstance(root_params, dict)
    assert isinstance(joy_params, dict)
    assert root_params["coloring_mode"] == "root_basin"
    assert root_params["color_signal"] == "root_index"
    assert root_params["color_palette"] == "root_classic"
    assert joy_params["coloring_mode"] == "joy_basins"
    assert joy_params["color_signal"] == "root_index"
    assert joy_params["color_palette"] == "joy"
    assert root_classic_capture["frame_hash"] != joy_capture["frame_hash"], (
        "expected Explaino root_classic and joy basin palettes to produce distinct published runtime frame hashes"
    )


def test_explaino_root_palette_shape_changes_published_runtime_frames(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino root-palette shape runtime regression is Windows-only")

    exe_path = _active_runtime_exe()

    explaino_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )
    root_classic_state = _with_explaino_root_basin_palette_state(
        explaino_capture["state"],
        palette="root_classic",
    )
    root_classic_shifted_state = _with_explaino_root_basin_palette_state(
        explaino_capture["state"],
        palette="root_classic",
        color_shape="offset_scale",
        color_shape_offset=0.4,
        color_shape_scale=1.0,
    )

    root_classic_baseline_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "root_classic_shape_baseline", root_classic_state)),
        "--capture-diagnostic",
    )
    root_classic_shifted_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "root_classic_shape_shifted", root_classic_shifted_state)),
        "--capture-diagnostic",
    )

    explaino_joy_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_joy",
        "--width",
        "320",
        "--height",
        "240",
    )
    joy_state = _with_explaino_root_basin_palette_state(
        explaino_joy_capture["state"],
        palette="joy",
    )
    joy_shifted_state = _with_explaino_root_basin_palette_state(
        explaino_joy_capture["state"],
        palette="joy",
        color_shape="offset_scale",
        color_shape_offset=0.4,
        color_shape_scale=1.0,
    )

    joy_baseline_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "joy_shape_baseline", joy_state)),
        "--capture-diagnostic",
    )
    joy_shifted_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "joy_shape_shifted", joy_shifted_state)),
        "--capture-diagnostic",
    )

    assert root_classic_baseline_capture["frame_hash"] != root_classic_shifted_capture["frame_hash"], (
        "expected Explaino root_classic Shape edits to change the published runtime frame hash"
    )
    assert joy_baseline_capture["frame_hash"] != joy_shifted_capture["frame_hash"], (
        "expected Explaino Joy root-palette Shape edits to change the published runtime frame hash"
    )


def test_explaino_sidecar_headless_apply_step_changes_state_and_frame(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino sidecar runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    configured_state = _configure_sidecar_policy(
        baseline_capture["state"],
        enabled=True,
        allow_runtime_mutation=True,
        run_paced_loop=False,
        paced_loop_interval_seconds=0.1,
        stop_demonstrated_fraction=1.0,
        stop_uncertain_count=0,
    )
    state_path = _write_state_bundle(tmp_path, configured_state)

    applied_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--sidecar-apply-armed-step-count",
        "1",
        "--capture-diagnostic",
    )

    changed_fields = _numeric_state_deltas(baseline_capture["state"], applied_capture["state"])
    assert changed_fields, "expected headless armed-step proof to mutate at least one numeric state field"
    assert applied_capture["frame_hash"] != baseline_capture["frame_hash"], (
        "expected headless armed-step proof to change the rendered frame hash"
    )


def test_explaino_sidecar_headless_apply_step_persists_mutation_history(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino sidecar runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    configured_state = _configure_sidecar_policy(
        baseline_capture["state"],
        enabled=True,
        allow_runtime_mutation=True,
        run_paced_loop=False,
        paced_loop_interval_seconds=0.1,
        stop_demonstrated_fraction=1.0,
        stop_uncertain_count=0,
    )
    state_path = _write_state_bundle(tmp_path / "apply", configured_state)

    applied_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--sidecar-apply-armed-step-count",
        "1",
        "--capture-diagnostic",
    )

    mutation_history = applied_capture["state"].get("sidecar_mutation_history")
    assert isinstance(mutation_history, list) and mutation_history, (
        "expected headless armed-step proof to persist at least one sidecar mutation-history record"
    )
    first_record = mutation_history[0]
    assert first_record["path"], "expected persisted mutation-history record to include a bound path"
    assert first_record["type"], "expected persisted mutation-history record to include a bound type"
    assert isinstance(first_record["target_value"], (int, float))

    reloaded_state_path = _write_state_bundle(tmp_path / "reload", applied_capture["state"])
    reloaded_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(reloaded_state_path),
        "--capture-diagnostic",
    )

    assert reloaded_capture["state"].get("sidecar_mutation_history") == mutation_history
    assert reloaded_capture["frame_hash"] == applied_capture["frame_hash"]


def test_explaino_sidecar_headless_replay_mutation_history_count_replays_ordered_targets(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino sidecar runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    mutation_history = [
        {
            "label": "replay-step-1",
            "path": "fractal.params.ripple_amplitude",
            "type": "float",
            "target_value": 0.12,
            "utility": 1.0,
        },
        {
            "label": "replay-step-2",
            "path": "fractal.params.ripple_amplitude",
            "type": "float",
            "target_value": 0.24,
            "utility": 1.0,
        },
    ]
    replay_state_path = _write_state_bundle(
        tmp_path / "replay",
        _with_sidecar_mutation_history(
            baseline_capture["state"],
            mutation_history,
            ripple_amplitude=0.5,
        ),
    )

    replay_one_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_state_path),
        "--sidecar-replay-mutation-history-count",
        "1",
        "--capture-diagnostic",
    )
    assert replay_one_capture["state"]["params"]["ripple_amplitude"] == pytest.approx(0.12, abs=1e-6)
    assert replay_one_capture["state"].get("sidecar_mutation_history") == mutation_history

    replay_two_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_state_path),
        "--sidecar-replay-mutation-history-count",
        "2",
        "--capture-diagnostic",
    )
    assert replay_two_capture["state"]["params"]["ripple_amplitude"] == pytest.approx(0.24, abs=1e-6)
    assert replay_two_capture["state"].get("sidecar_mutation_history") == mutation_history


def test_explaino_exploration_advisor_report_is_deterministic_for_loaded_state(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino advisor runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )
    state_path = _write_state_bundle(tmp_path / "advisor_state", baseline_capture["state"])

    report_one = _run_exploration_advisor(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--explore-recommend-json",
        str(tmp_path / "advisor_one.json"),
        report_path=tmp_path / "advisor_one.json",
    )
    report_two = _run_exploration_advisor(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--explore-recommend-json",
        str(tmp_path / "advisor_two.json"),
        report_path=tmp_path / "advisor_two.json",
    )

    assert report_one == report_two
    assert report_one["report_version"] == 1
    assert report_one["fractal_type"] == "explaino"
    assert report_one["recommendations"]
    assert report_one["recommended_observation"]["rank"] == 1
    assert report_one["recommended_observation"]["path"] == report_one["recommendations"][0]["path"]


def test_explaino_exploration_advisor_stdout_matches_json_output_for_loaded_state(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino advisor runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )
    state_path = _write_state_bundle(tmp_path / "advisor_stdout_state", baseline_capture["state"])

    file_report = _run_exploration_advisor(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--explore-recommend-json",
        str(tmp_path / "advisor_stdout_file.json"),
        report_path=tmp_path / "advisor_stdout_file.json",
    )
    stdout_report = _run_exploration_advisor_stdout(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--explore-recommend",
    )

    assert stdout_report == file_report


def test_explaino_exploration_advisor_rejects_non_explaino_loaded_state(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino advisor runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "mandelbrot",
        "--width",
        "320",
        "--height",
        "240",
    )
    state_path = _write_state_bundle(tmp_path / "advisor_non_explaino_state", baseline_capture["state"])
    report_path = tmp_path / "advisor_non_explaino.json"

    result = _run_exploration_advisor_failure(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--explore-recommend-json",
        str(report_path),
    )

    assert result.returncode != 0
    assert "Explaino" in (result.stderr or result.stdout)
    assert not report_path.exists()


def test_explaino_sidecar_headless_paced_loop_respects_stop_threshold(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino sidecar runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    moving_state_path = _write_state_bundle(
        tmp_path / "moving",
        _configure_sidecar_policy(
            baseline_capture["state"],
            enabled=True,
            allow_runtime_mutation=True,
            run_paced_loop=True,
            paced_loop_interval_seconds=0.05,
            stop_demonstrated_fraction=1.0,
            stop_uncertain_count=0,
        ),
    )
    stopped_state_path = _write_state_bundle(
        tmp_path / "stopped",
        _configure_sidecar_policy(
            baseline_capture["state"],
            enabled=True,
            allow_runtime_mutation=True,
            run_paced_loop=True,
            paced_loop_interval_seconds=0.05,
            stop_demonstrated_fraction=0.0,
            stop_uncertain_count=0,
        ),
    )

    moving_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(moving_state_path),
        "--sidecar-pump-paced-loop-seconds",
        "0.20",
        "--capture-diagnostic",
    )
    stopped_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(stopped_state_path),
        "--sidecar-pump-paced-loop-seconds",
        "0.20",
        "--capture-diagnostic",
    )

    moving_fields = _numeric_state_deltas(baseline_capture["state"], moving_capture["state"])
    stopped_fields = _numeric_state_deltas(baseline_capture["state"], stopped_capture["state"])

    assert moving_fields, "expected paced-loop proof to mutate at least one numeric state field"
    assert moving_capture["frame_hash"] != baseline_capture["frame_hash"], (
        "expected paced-loop proof to change the rendered frame hash"
    )
    assert not stopped_fields, (
        "expected zero-coverage stop threshold to halt the paced loop before mutating state"
    )
    assert stopped_capture["frame_hash"] == baseline_capture["frame_hash"], (
        "expected zero-coverage stop threshold to leave the rendered frame unchanged"
    )