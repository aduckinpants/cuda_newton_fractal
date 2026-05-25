from __future__ import annotations

import copy
import json
from dataclasses import dataclass
from pathlib import Path

from runtime_harness import active_runtime_exe, run_headless_capture, runtime_automation_lock


@dataclass(frozen=True)
class PhaseSignalMatrixRow:
    name: str
    color_signal: str
    color_palette: str
    color_grading: str
    source_stack: list[dict[str, object]] | None
    lens_downsample: int = 4


SDF_PHASE_MATRIX_ROWS = (
    PhaseSignalMatrixRow(
        name="sdf_normal_angle_phase_base",
        color_signal="sdf_normal_angle",
        color_palette="phase_wheel",
        color_grading="phase_default",
        source_stack=[{"signal": "sdf_normal_angle", "scale": 1.0, "bias": 0.0, "blend_weight": 1.0}],
    ),
    PhaseSignalMatrixRow(
        name="sdf_normal_angle_phase_offset",
        color_signal="sdf_normal_angle",
        color_palette="phase_wheel",
        color_grading="phase_default",
        source_stack=[{"signal": "sdf_normal_angle", "scale": 1.0, "bias": 0.25, "blend_weight": 1.0}],
    ),
    PhaseSignalMatrixRow(
        name="sdf_signed_distance_scalar",
        color_signal="sdf_signed_distance",
        color_palette="cyclic_escape",
        color_grading="escape_default",
        source_stack=[{"signal": "sdf_signed_distance", "scale": 0.05, "bias": 0.5, "blend_weight": 1.0}],
    ),
    PhaseSignalMatrixRow(
        name="sdf_boundary_band_scalar",
        color_signal="sdf_boundary_band",
        color_palette="cyclic_escape",
        color_grading="escape_default",
        source_stack=[
            {
                "signal": "sdf_boundary_band",
                "scale": 1.0,
                "bias": 0.0,
                "blend_weight": 1.0,
                "sdf_boundary_width_px": 3.0,
            }
        ],
    ),
    PhaseSignalMatrixRow(
        name="smooth_escape_scalar",
        color_signal="smooth_escape",
        color_palette="cyclic_escape",
        color_grading="escape_default",
        source_stack=None,
        lens_downsample=2,
    ),
)


def _base_multibrot_state(exe_path: Path) -> dict[str, object]:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "multibrot",
        "--width",
        "192",
        "--height",
        "144",
    )
    state = copy.deepcopy(capture["state"])
    state["render"]["width"] = 192
    state["render"]["height"] = 144
    state["params"]["max_iter"] = 96
    state["params"]["multibrot_power_float"] = 3.0
    state["params"]["multibrot_power_imag"] = 0.0
    return state


def _state_for_row(base_state: dict[str, object], row: PhaseSignalMatrixRow) -> dict[str, object]:
    state = copy.deepcopy(base_state)
    params = state["params"]
    params["coloring_mode"] = "phase" if row.color_palette == "phase_wheel" else "smooth_escape"
    params["color_signal"] = row.color_signal
    params["color_shape"] = "identity"
    params["color_palette"] = row.color_palette
    params["color_grading"] = row.color_grading
    params.pop("color_source_stack", None)
    if row.source_stack is not None:
        params["color_source_stack"] = row.source_stack
    state["lens"] = {
        "enabled": False,
        "downsample": row.lens_downsample,
        "sdf_overlay_mode": "off",
        "sdf_overlay_opacity": 0.55,
        "sdf_overlay_band_px": 1.5,
    }
    return state


def _write_json(path: Path, payload: dict[str, object]) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    return path


def _capture_row(exe_path: Path, tmp_path: Path, base_state: dict[str, object], row: PhaseSignalMatrixRow) -> dict[str, object]:
    state_path = _write_json(tmp_path / row.name / "input.custom-state.json", _state_for_row(base_state, row))
    capture = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--capture-diagnostic",
    )
    params = capture["state"]["params"]
    assert params["color_signal"] == row.color_signal
    assert params["color_palette"] == row.color_palette
    if row.source_stack is not None:
        source_stack = params.get("color_source_stack")
        assert isinstance(source_stack, list) and source_stack[0]["signal"] == row.source_stack[0]["signal"]
    assert capture["state"].get("lens", {}).get("downsample") == row.lens_downsample
    return capture


def test_sdf_normal_angle_phase_matrix_no_mouse(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    with runtime_automation_lock():
        base_state = _base_multibrot_state(exe_path)
        captures = {
            row.name: _capture_row(exe_path, tmp_path, base_state, row)
            for row in SDF_PHASE_MATRIX_ROWS
        }

        normal_hash = captures["sdf_normal_angle_phase_base"]["frame_hash"]
        offset_hash = captures["sdf_normal_angle_phase_offset"]["frame_hash"]
        assert offset_hash != normal_hash, "normal-angle phase bias should move/change the phase visualization"
        assert captures["sdf_signed_distance_scalar"]["frame_hash"] != normal_hash
        assert captures["sdf_boundary_band_scalar"]["frame_hash"] != normal_hash
        assert captures["smooth_escape_scalar"]["frame_hash"] != normal_hash

        root_capture = run_headless_capture(
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "newton",
            "--width",
            "192",
            "--height",
            "144",
        )
        root_params = root_capture["state"]["params"]
        assert root_params["coloring_mode"] in {"root_basin", "joy_basins"}
        assert root_capture["frame_hash"] != normal_hash
