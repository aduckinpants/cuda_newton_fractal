from __future__ import annotations

import copy
import json
from dataclasses import dataclass
from pathlib import Path

from runtime_harness import active_runtime_exe, run_headless_capture, runtime_automation_lock


@dataclass(frozen=True)
class CaptureReplayMatrixRow:
    name: str
    source_stack: list[dict[str, object]] | None
    color_signal: str
    color_palette: str = "cyclic_escape"
    color_grading: str = "escape_default"
    lens_downsample: int = 2


MATRIX_ROWS = [
    CaptureReplayMatrixRow(
        name="plain_smooth_escape",
        source_stack=None,
        color_signal="smooth_escape",
    ),
    CaptureReplayMatrixRow(
        name="sdf_signed_distance",
        source_stack=[{"signal": "sdf_signed_distance", "scale": 0.05, "bias": 0.5, "blend_weight": 1.0}],
        color_signal="sdf_signed_distance",
        lens_downsample=4,
    ),
    CaptureReplayMatrixRow(
        name="sdf_normal_angle",
        source_stack=[{"signal": "sdf_normal_angle", "scale": 1.0, "bias": 0.0, "blend_weight": 1.0}],
        color_signal="sdf_normal_angle",
        color_palette="phase_wheel",
        color_grading="phase_default",
        lens_downsample=4,
    ),
    CaptureReplayMatrixRow(
        name="sdf_boundary_band",
        source_stack=[
            {
                "signal": "sdf_boundary_band",
                "scale": 1.0,
                "bias": 0.0,
                "blend_weight": 1.0,
                "sdf_boundary_width_px": 3.5,
            }
        ],
        color_signal="sdf_boundary_band",
        lens_downsample=4,
    ),
    CaptureReplayMatrixRow(
        name="sdf_two_row_stack_nondefault_downsample",
        source_stack=[
            {"signal": "sdf_normal_angle", "scale": -0.4, "bias": 0.18, "blend_weight": 1.0},
            {"signal": "sdf_signed_distance", "scale": 1.6, "bias": 0.97, "blend_weight": 0.25},
        ],
        color_signal="sdf_signed_distance",
        lens_downsample=8,
    ),
]


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


def _state_for_row(base_state: dict[str, object], row: CaptureReplayMatrixRow) -> dict[str, object]:
    state = copy.deepcopy(base_state)
    params = state["params"]
    params["coloring_mode"] = "smooth_escape"
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


def _assert_effective_source_summary(captured_state: dict[str, object], row: CaptureReplayMatrixRow) -> None:
    params = captured_state["params"]
    summary = params.get("color_effective_source")
    assert isinstance(summary, dict), f"{row.name}: missing color_effective_source summary"
    assert summary.get("legacy_flat_signal") == row.color_signal
    if row.source_stack is None:
        assert summary.get("authority") == "flat_signal"
        assert summary.get("source_stack_count") == 0
        assert summary.get("source_stack") == []
        return

    assert summary.get("authority") == "source_stack"
    assert summary.get("source_stack_count") == len(row.source_stack)
    summary_stack = summary.get("source_stack")
    assert isinstance(summary_stack, list)
    assert [entry["signal"] for entry in summary_stack] == [entry["signal"] for entry in row.source_stack]
    for entry in summary_stack:
        if entry["signal"] == "sdf_normal_angle":
            assert entry["kind"] == "phase"
        elif str(entry["signal"]).startswith("sdf_"):
            assert entry["kind"] in {"scalar", "categorical"}


def test_capture_state_replays_pixels_for_sdf_and_non_sdf_matrix(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    with runtime_automation_lock():
        base_state = _base_multibrot_state(exe_path)
        for row in MATRIX_ROWS:
            input_state = _state_for_row(base_state, row)
            input_path = _write_json(tmp_path / row.name / "input.custom-state.json", input_state)
            first_capture = run_headless_capture(
                str(exe_path),
                "--load-state-json",
                str(input_path),
                "--capture-diagnostic",
            )
            captured_state = first_capture["state"]
            assert captured_state.get("lens", {}).get("downsample") == row.lens_downsample
            _assert_effective_source_summary(captured_state, row)

            replay_path = _write_json(tmp_path / row.name / "replay-from-emitted-state.any-json-name", captured_state)
            replay_capture = run_headless_capture(
                str(exe_path),
                "--load-state-json",
                str(replay_path),
                "--capture-diagnostic",
            )
            assert first_capture["frame_hash"] == replay_capture["frame_hash"], row.name
