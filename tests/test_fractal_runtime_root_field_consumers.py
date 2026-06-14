from __future__ import annotations

import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    RUNTIME_DIR,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _require_frame_hash(payload: dict[str, Any]) -> str:
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), payload
    return frame_hash


def _require_root_hash(payload: dict[str, Any]) -> str:
    root_hash = payload.get("root_field_consumer_base_root_hash")
    assert isinstance(root_hash, str) and root_hash.startswith("fnv1a64:"), payload
    return root_hash


def _state_for_lane(exe_path: Path, lane_id: str) -> dict[str, Any]:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        lane_id,
        "--width",
        "256",
        "--height",
        "192",
    )
    state = json.loads(json.dumps(capture["state"]))
    assert state.get("fractal_type") == lane_id, state
    return state


def _color_pipeline_row(state: dict[str, Any], lane_id: str, row_index: int) -> dict[str, Any]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), "expected captured state to include color_pipeline_draft"
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), "expected color_pipeline_draft.lanes to be a list"
    for lane in lanes:
        if isinstance(lane, dict) and lane.get("lane_id") == lane_id:
            rows = lane.get("rows")
            assert isinstance(rows, list), f"expected {lane_id} rows to be a list"
            row = rows[row_index]
            assert isinstance(row, dict), f"expected {lane_id} row {row_index} to be an object"
            return row
    raise AssertionError(f"missing color pipeline lane {lane_id!r}")


def _color_pipeline_number_param(row: dict[str, Any], path: str) -> float:
    values = row.get("parameter_values")
    assert isinstance(values, list), row
    for value in values:
        if isinstance(value, dict) and value.get("path") == path:
            number = value.get("number_value")
            assert isinstance(number, (int, float)), value
            return float(number)
    raise AssertionError(f"missing color pipeline param {path!r} in row {row!r}")


def _root_pattern(report: dict[str, Any], ref: str) -> dict[str, Any]:
    patterns = report.get("root_patterns")
    assert isinstance(patterns, list), report
    for pattern in patterns:
        if isinstance(pattern, dict) and pattern.get("ref") == ref:
            return pattern
    raise AssertionError(f"missing root pattern ref {ref!r}: {report!r}")


def _has_root_pattern_consumer(
    report: dict[str, Any],
    *,
    consumer_kind: str,
    consumer_id: str,
    pattern_ref: str,
) -> bool:
    consumers = report.get("root_pattern_consumers")
    assert isinstance(consumers, list), report
    return any(
        isinstance(consumer, dict)
        and consumer.get("consumer_kind") == consumer_kind
        and consumer.get("consumer_id") == consumer_id
        and consumer.get("pattern_ref") == pattern_ref
        for consumer in consumers
    )


def _visible_control_ids(payload: dict[str, Any]) -> set[str]:
    controls = payload.get("controls")
    assert isinstance(controls, list), payload
    ids: set[str] = set()
    for control in controls:
        assert isinstance(control, dict), control
        control_id = control.get("control_id")
        assert isinstance(control_id, str), control
        ids.add(control_id)
    return ids


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
@pytest.mark.parametrize(
    ("lane_id", "base_lane"),
    [
        ("explaino_mandelbrot_root_trap", "mandelbrot"),
        ("explaino_magnet_root_well", "magnet"),
    ],
)
def test_root_field_consumer_lanes_report_and_mutate_no_mouse(
    tmp_path: Path,
    lane_id: str,
    base_lane: str,
) -> None:
    exe_path = active_runtime_exe()
    state = _state_for_lane(exe_path, lane_id)
    state_path = write_state_bundle(tmp_path / lane_id, state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / f"{lane_id}_report.json",
        command_path=tmp_path / f"{lane_id}_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.explaino_root_field_trap_strength.primary", timeout_seconds=20.0)
        viewer.wait_for_control("fractal_control.explaino_root_field_trap_scale.primary", timeout_seconds=20.0)

        baseline = viewer.wait_for_report(timeout_seconds=60.0)
        assert baseline.get("current_fractal_type") == lane_id, baseline
        assert baseline.get("root_field_consumer_active") is True, baseline
        assert baseline.get("root_field_consumer_kind") == lane_id, baseline
        assert baseline.get("root_field_consumer_base_fractal_type") == base_lane, baseline
        assert baseline.get("root_field_consumer_root_count") == 4, baseline
        assert baseline.get("root_field_consumer_fail_closed_reason") is None, baseline
        baseline_hash = _require_frame_hash(baseline)
        baseline_root_hash = _require_root_hash(baseline)

        neutral = viewer.set_control_value(
            "fractal_control.explaino_root_field_trap_strength.primary",
            0.0,
            timeout_seconds=20.0,
        )
        assert neutral.get("current_fractal_type") == lane_id, neutral
        assert neutral.get("root_field_consumer_active") is True, neutral
        neutral_hash = _require_frame_hash(neutral)

        restored = viewer.set_control_value(
            "fractal_control.explaino_root_field_trap_strength.primary",
            1.0,
            timeout_seconds=20.0,
        )
        assert restored.get("current_fractal_type") == lane_id, restored
        assert _require_frame_hash(restored) != neutral_hash, restored

        scaled = viewer.set_control_value(
            "fractal_control.explaino_root_field_trap_scale.primary",
            2.0,
            timeout_seconds=20.0,
        )
        assert scaled.get("current_fractal_type") == lane_id, scaled
        assert _require_frame_hash(scaled) != neutral_hash, scaled

        regular = viewer.set_enum_id(
            "fractal.root_pattern.dynamics.generated_layout",
            "regular_ngon_v1",
            expected_fractal_type=lane_id,
            timeout_seconds=20.0,
        )
        assert regular.get("root_field_consumer_root_layout_kind") == "regular_ngon_v1", regular
        viewer.wait_for_control("fractal_control.dynamics_root_field_generated_root_count.primary", timeout_seconds=20.0)
        root_count = viewer.set_control_value(
            "fractal_control.dynamics_root_field_generated_root_count.primary",
            5.0,
            timeout_seconds=20.0,
        )
        assert root_count.get("root_field_consumer_root_count") == 5, root_count
        assert _require_root_hash(root_count) != baseline_root_hash, root_count
        assert _require_frame_hash(root_count) != baseline_hash, root_count


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
def test_explaino_magnet_root_well_base_magnet_controls_are_visible_and_active(
    tmp_path: Path,
) -> None:
    exe_path = active_runtime_exe()
    lane_id = "explaino_magnet_root_well"
    state = _state_for_lane(exe_path, lane_id)
    state_path = write_state_bundle(tmp_path / "magnet_root_well_base_controls", state)

    expected_controls = {
        "fractal_control.magnet_seed_real.primary",
        "fractal_control.magnet_seed_imag.primary",
        "fractal_control.magnet_relaxation.primary",
        "fractal_control.magnet_bailout.primary",
        "fractal_control.explaino_root_field_trap_strength.primary",
        "fractal_control.dynamics_root_field_seed.primary",
        "dynamics_root_field_prev_seed",
        "fractal_control.dynamics_root_field_generated_layout.primary",
    }

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "magnet_root_well_base_controls_report.json",
        command_path=tmp_path / "magnet_root_well_base_controls_command.json",
    ) as viewer:
        for control_id in sorted(expected_controls):
            viewer.wait_for_control(control_id, timeout_seconds=30.0)

        baseline = viewer.wait_for_report(timeout_seconds=60.0)
        assert baseline.get("current_fractal_type") == lane_id, baseline
        visible_controls = _visible_control_ids(baseline)
        assert expected_controls.issubset(visible_controls), baseline
        assert "fractal_control.explaino_root_field_pattern_ref.primary" not in visible_controls, baseline
        assert "fractal_control.explaino_seed.primary" not in visible_controls, baseline
        assert "fractal_control.prev_seed.primary" not in visible_controls, baseline
        assert "fractal_control.next_seed.primary" not in visible_controls, baseline
        assert "fractal_control.explaino_root_spread.primary" not in visible_controls, baseline
        assert "fractal_control.explaino_generated_root_layout.primary" not in visible_controls, baseline
        assert "fractal_control.explaino_secondary_root_pattern_layout.primary" not in visible_controls, baseline
        assert "fractal_control.explaino_secondary_root_pattern_count.primary" not in visible_controls, baseline
        baseline_hash = _require_frame_hash(baseline)
        baseline_root_hash = _require_root_hash(baseline)

        next_seed = viewer.click_control("dynamics_root_field_next_seed", timeout_seconds=20.0)
        assert next_seed.get("current_fractal_type") == lane_id, next_seed
        assert _require_root_hash(next_seed) != baseline_root_hash, next_seed
        assert _require_frame_hash(next_seed) != baseline_hash, next_seed

        prev_seed = viewer.click_control("dynamics_root_field_prev_seed", timeout_seconds=20.0)
        assert prev_seed.get("current_fractal_type") == lane_id, prev_seed
        assert _require_root_hash(prev_seed) == baseline_root_hash, prev_seed

        edits = [
            ("fractal_control.magnet_seed_real.primary", 0.35),
            ("fractal_control.magnet_seed_imag.primary", -0.25),
            ("fractal_control.magnet_relaxation.primary", 0.65),
            ("fractal_control.magnet_bailout.primary", 24.0),
        ]
        edited = baseline
        for control_id, value in edits:
            edited = viewer.set_control_value(control_id, value, timeout_seconds=30.0)
            assert edited.get("current_fractal_type") == lane_id, edited
            assert edited.get("requested_set_control_id") == control_id, edited
            assert edited.get("set_value_consumed") is True, edited
            assert edited.get("set_value_error") is None, edited

        assert _require_frame_hash(edited) != baseline_hash, edited


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
def test_mandelbrot_root_trap_root_phase_source_runtime_actions_change_frame(
    tmp_path: Path,
) -> None:
    exe_path = active_runtime_exe()
    lane_id = "explaino_mandelbrot_root_trap"
    state = _state_for_lane(exe_path, lane_id)
    state_path = write_state_bundle(tmp_path / "root_phase_source", state)

    baseline = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--capture-diagnostic",
    )
    root_phase = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        "select_function:source:0:root_phase",
        "--color-pipeline-action",
        "select_function:palette:0:phase_wheel_palette",
        "--capture-diagnostic",
    )
    shifted = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        "select_function:source:0:root_phase",
        "--color-pipeline-action",
        "select_function:palette:0:phase_wheel_palette",
        "--color-pipeline-action",
        "set_param:source:0:signal.phase_offset:number:1.57079632679",
        "--capture-diagnostic",
    )

    root_phase_state = root_phase["state"]
    params = root_phase_state.get("params")
    assert isinstance(params, dict), root_phase_state
    assert root_phase_state.get("fractal_type") == lane_id
    assert params.get("color_signal") == "root_phase", params
    assert params.get("color_palette") == "phase_wheel", params
    assert params.get("coloring_mode") == "phase", params

    root_phase_source_row = _color_pipeline_row(root_phase_state, "source", 0)
    assert root_phase_source_row.get("function_id") == "root_phase", root_phase_source_row
    root_phase_palette_row = _color_pipeline_row(root_phase_state, "palette", 0)
    assert root_phase_palette_row.get("function_id") == "phase_wheel_palette", root_phase_palette_row

    shifted_source_row = _color_pipeline_row(shifted["state"], "source", 0)
    assert shifted_source_row.get("function_id") == "root_phase", shifted_source_row
    assert _color_pipeline_number_param(shifted_source_row, "signal.phase_offset") == pytest.approx(
        1.57079632679,
        abs=1.0e-6,
    )

    assert root_phase["frame_hash"] != baseline["frame_hash"]
    assert shifted["frame_hash"] != root_phase["frame_hash"]


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
def test_explaino_magnet_root_well_scoped_root_controls_and_color_refs(
    tmp_path: Path,
) -> None:
    exe_path = active_runtime_exe()
    lane_id = "explaino_magnet_root_well"
    state = _state_for_lane(exe_path, lane_id)
    params = state["params"]
    assert isinstance(params, dict), state
    params["explaino_generated_root_layout"] = "regular_ngon_v1"
    params["explaino_generated_root_count"] = 11
    params["explaino_secondary_root_pattern_layout"] = "legacy_quartic_v1"
    params["explaino_secondary_root_pattern_count"] = 4
    params["explaino_root_field_pattern_ref"] = "dynamics_root_field"
    params["explaino_root_field_trap_strength"] = 1.0
    params["explaino_root_field_trap_scale"] = 1.5
    params["coloring_mode"] = "smooth_escape"
    params["color_signal"] = "root_proximity"
    params["color_shape"] = "identity"
    params["color_palette"] = "explaino_cmap"
    params["color_grading"] = "escape_default"
    params["color_source_stack"] = [
        {
            "signal": "root_proximity",
            "proximity_scale": 1.0,
            "proximity_bias": 0.0,
            "root_pattern_ref": "color_root_field",
            "blend_weight": 1.0,
        }
    ]
    secondary_state_path = write_state_bundle(tmp_path / "magnet_root_patterns_secondary", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=secondary_state_path,
        report_path=tmp_path / "magnet_root_patterns_report.json",
        command_path=tmp_path / "magnet_root_patterns_command.json",
        open_color_pipeline=True,
    ) as viewer:
        secondary = viewer.wait_for_report(timeout_seconds=60.0)
        visible_controls = {
            str(control.get("control_id", ""))
            for control in secondary.get("controls", [])
            if isinstance(control, dict)
        }
        assert "fractal_control.explaino_root_field_pattern_ref.primary" not in visible_controls, secondary
        assert "fractal_control.explaino_secondary_root_pattern_layout.primary" not in visible_controls, secondary
        assert "fractal_control.explaino_secondary_root_pattern_count.primary" not in visible_controls, secondary
        assert secondary.get("current_fractal_type") == lane_id, secondary
        assert secondary.get("root_field_consumer_kind") == lane_id, secondary
        assert secondary.get("root_field_consumer_root_count") == 11, secondary
        assert secondary.get("root_field_consumer_root_layout_kind") == "regular_ngon_v1", secondary
        active_root_field = secondary.get("active_root_field")
        assert isinstance(active_root_field, dict), secondary
        assert active_root_field.get("ref") == "dynamics_root_field", active_root_field
        assert active_root_field.get("label") == "Dynamics Root Field", active_root_field
        assert active_root_field.get("root_count") == 11, active_root_field
        assert active_root_field.get("layout_kind") == "regular_ngon_v1", active_root_field
        assert _root_pattern(secondary, "dynamics_root_field").get("root_count") == 11, secondary
        assert _root_pattern(secondary, "dynamics_root_field").get("layout_kind") == "regular_ngon_v1", secondary
        assert _root_pattern(secondary, "color_root_field").get("root_count") == 4, secondary
        assert _root_pattern(secondary, "color_root_field").get("layout_kind") == "legacy_quartic_v1", secondary
        assert _has_root_pattern_consumer(
            secondary,
            consumer_kind="root_field_consumer",
            consumer_id=lane_id,
            pattern_ref="dynamics_root_field",
        ), secondary
        assert _has_root_pattern_consumer(
            secondary,
            consumer_kind="color_source_row",
            consumer_id="root_proximity",
            pattern_ref="color_root_field",
        ), secondary
        secondary_hash = _require_frame_hash(secondary)

        primary_state = json.loads(json.dumps(state))
        primary_params = primary_state["params"]
        assert isinstance(primary_params, dict), primary_state
        source_rows = primary_params["color_source_stack"]
        assert isinstance(source_rows, list), primary_params
        source_rows[0]["root_pattern_ref"] = "dynamics_root_field"
        primary_state_path = write_state_bundle(tmp_path / "magnet_root_patterns_primary", primary_state)
        primary = viewer.load_state_json(
            primary_state_path,
            expected_fractal_type=lane_id,
            timeout_seconds=60.0,
        )
        assert primary.get("root_field_consumer_root_count") == 11, primary
        assert _has_root_pattern_consumer(
            primary,
            consumer_kind="root_field_consumer",
            consumer_id=lane_id,
            pattern_ref="dynamics_root_field",
        ), primary
        assert _has_root_pattern_consumer(
            primary,
            consumer_kind="color_source_row",
            consumer_id="root_proximity",
            pattern_ref="dynamics_root_field",
        ), primary
        assert _require_frame_hash(primary) != secondary_hash, primary


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
def test_magnet_root_well_primary_ngon_variation_controls_change_frame_no_mouse(
    tmp_path: Path,
) -> None:
    exe_path = active_runtime_exe()
    lane_id = "explaino_magnet_root_well"
    state = _state_for_lane(exe_path, lane_id)

    def configure_state(
        source: dict[str, Any],
        *,
        seed: float,
        phase: float,
        spread: float,
        source_pattern_ref: str,
    ) -> dict[str, Any]:
        configured = json.loads(json.dumps(source))
        configured["fractal_type"] = lane_id
        view = configured.setdefault("view", {})
        assert isinstance(view, dict), configured
        view["explaino_phase"] = phase
        configured["explaino_phase"] = phase
        params = configured.setdefault("params", {})
        assert isinstance(params, dict), configured
        params["explaino_root_authority"] = "generated"
        params["explaino_generated_root_layout"] = "regular_ngon_v1"
        params["explaino_generated_root_count"] = 11
        params["explaino_secondary_root_pattern_layout"] = "legacy_quartic_v1"
        params["explaino_secondary_root_pattern_count"] = 4
        params["explaino_root_field_pattern_ref"] = "dynamics_root_field"
        params["explaino_root_field_trap_strength"] = 1.0
        params["explaino_root_field_trap_scale"] = 1.5
        params["explaino_seed"] = seed
        params["explaino_root_spread"] = spread
        params["coloring_mode"] = "smooth_escape"
        params["color_signal"] = "root_proximity"
        params["color_shape"] = "identity"
        params["color_palette"] = "explaino_cmap"
        params["color_grading"] = "escape_default"
        params["color_source_stack"] = [
            {
                "signal": "root_proximity",
                "proximity_scale": 1.0,
                "proximity_bias": 0.0,
                "root_pattern_ref": source_pattern_ref,
                "blend_weight": 1.0,
            }
        ]
        return configured

    primary_a_path = write_state_bundle(
        tmp_path / "magnet_primary_ngon_a",
        configure_state(state, seed=0.13, phase=0.08, spread=0.45, source_pattern_ref="dynamics_root_field"),
    )
    primary_b_path = write_state_bundle(
        tmp_path / "magnet_primary_ngon_b",
        configure_state(state, seed=0.57, phase=0.42, spread=0.92, source_pattern_ref="dynamics_root_field"),
    )
    secondary_a_path = write_state_bundle(
        tmp_path / "magnet_secondary_legacy_a",
        configure_state(state, seed=0.13, phase=0.08, spread=0.45, source_pattern_ref="color_root_field"),
    )
    secondary_b_path = write_state_bundle(
        tmp_path / "magnet_secondary_legacy_b",
        configure_state(state, seed=0.57, phase=0.42, spread=0.92, source_pattern_ref="color_root_field"),
    )

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=primary_a_path,
        report_path=tmp_path / "magnet_primary_ngon_report.json",
        command_path=tmp_path / "magnet_primary_ngon_command.json",
        open_color_pipeline=True,
    ) as viewer:
        viewer.wait_for_control("fractal_control.dynamics_root_field_generated_layout.primary", timeout_seconds=30.0)
        primary_a = viewer.wait_for_report(timeout_seconds=60.0)
        assert primary_a.get("current_fractal_type") == lane_id, primary_a
        assert primary_a.get("root_field_consumer_root_layout_kind") == "regular_ngon_v1", primary_a
        assert _has_root_pattern_consumer(
            primary_a,
            consumer_kind="color_source_row",
            consumer_id="root_proximity",
            pattern_ref="dynamics_root_field",
        ), primary_a
        primary_a_frame = _require_frame_hash(primary_a)
        primary_a_root = _require_root_hash(primary_a)

        primary_b = viewer.load_state_json(
            primary_b_path,
            expected_fractal_type=lane_id,
            timeout_seconds=60.0,
        )
        assert _has_root_pattern_consumer(
            primary_b,
            consumer_kind="color_source_row",
            consumer_id="root_proximity",
            pattern_ref="dynamics_root_field",
        ), primary_b
        assert _require_root_hash(primary_b) != primary_a_root, primary_b
        assert _require_frame_hash(primary_b) != primary_a_frame, primary_b

        secondary_a = viewer.load_state_json(
            secondary_a_path,
            expected_fractal_type=lane_id,
            timeout_seconds=60.0,
        )
        assert _has_root_pattern_consumer(
            secondary_a,
            consumer_kind="color_source_row",
            consumer_id="root_proximity",
            pattern_ref="color_root_field",
        ), secondary_a
        secondary_a_frame = _require_frame_hash(secondary_a)

        secondary_b = viewer.load_state_json(
            secondary_b_path,
            expected_fractal_type=lane_id,
            timeout_seconds=60.0,
        )
        assert _has_root_pattern_consumer(
            secondary_b,
            consumer_kind="color_source_row",
            consumer_id="root_proximity",
            pattern_ref="color_root_field",
        ), secondary_b
        assert _require_root_hash(secondary_b) != _require_root_hash(secondary_a), secondary_b
        assert _has_root_pattern_consumer(
            secondary_b,
            consumer_kind="root_field_consumer",
            consumer_id=lane_id,
            pattern_ref="dynamics_root_field",
        ), secondary_b
        assert _require_frame_hash(secondary_b) != secondary_a_frame, secondary_b


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
@pytest.mark.parametrize(
    ("lane_id", "base_lane"),
    [
        ("explaino_mandelbrot_root_trap", "mandelbrot"),
        ("explaino_magnet_root_well", "magnet"),
    ],
)
def test_root_field_consumer_capture_finding_sidecar_and_replay(
    tmp_path: Path,
    lane_id: str,
    base_lane: str,
) -> None:
    exe_path = active_runtime_exe()
    state = _state_for_lane(exe_path, lane_id)
    state["params"]["explaino_generated_root_layout"] = "regular_ngon_v1"
    state["params"]["explaino_generated_root_count"] = 5
    state["params"]["explaino_secondary_root_pattern_layout"] = "legacy_quartic_v1"
    state["params"]["explaino_secondary_root_pattern_count"] = 4
    state["params"]["explaino_root_field_pattern_ref"] = "dynamics_root_field"
    state["params"]["explaino_root_field_trap_strength"] = 1.0
    state["params"]["explaino_root_field_trap_scale"] = 1.75
    state["params"]["coloring_mode"] = "smooth_escape"
    state["params"]["color_signal"] = "root_proximity"
    state["params"]["color_shape"] = "identity"
    state["params"]["color_palette"] = "explaino_cmap"
    state["params"]["color_source_stack"] = [
        {
            "signal": "root_proximity",
            "proximity_scale": 1.0,
            "proximity_bias": 0.0,
            "root_pattern_ref": "color_root_field",
            "blend_weight": 1.0,
        }
    ]
    state_path = write_state_bundle(tmp_path / f"{lane_id}_capture", state)

    group = f"pytest_root_field_consumer_{lane_id}_{tmp_path.name}"
    group_root = RUNTIME_DIR.parent / "findings" / group
    if group_root.exists():
        shutil.rmtree(group_root)
    result = subprocess.run(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--capture-finding",
            "--finding-group",
            group,
            "--finding-why",
            "root-field consumer runtime sidecar proof",
        ],
        cwd=str(exe_path.parent),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    state_paths = sorted(group_root.rglob("state.json"))
    assert len(state_paths) == 1, state_paths
    finding_dir = state_paths[0].parent
    finding_state_path = finding_dir / "state.json"
    fractal_state_path = finding_dir / "fractal-state.json"
    finding_json_path = finding_dir / "finding.json"
    assert finding_state_path.exists(), finding_dir
    assert fractal_state_path.exists(), finding_dir
    assert finding_json_path.exists(), finding_dir

    finding_json = json.loads(finding_json_path.read_text(encoding="utf-8"))
    assert finding_json.get("fractal_state_file") == "fractal-state.json", finding_json
    sidecar = json.loads(fractal_state_path.read_text(encoding="utf-8"))
    capture_context = sidecar.get("capture_context")
    assert isinstance(capture_context, dict), sidecar
    assert capture_context.get("fractal_type") == lane_id, capture_context
    assert capture_context.get("selected_fractal_type") == lane_id, capture_context
    active_controls = sidecar.get("active_fractal_controls")
    assert isinstance(active_controls, dict), sidecar
    assert active_controls.get("explaino_generated_root_layout") == "regular_ngon_v1", active_controls
    assert active_controls.get("explaino_generated_root_count") == 5, active_controls
    assert active_controls.get("explaino_root_field_trap_strength") == pytest.approx(1.0), active_controls
    assert active_controls.get("explaino_root_field_trap_scale") == pytest.approx(1.75), active_controls
    derived = sidecar.get("derived_runtime_values")
    assert isinstance(derived, dict), sidecar
    consumer = derived.get("root_field_consumer")
    assert isinstance(consumer, dict), derived
    assert consumer.get("consumer_kind") == lane_id, consumer
    assert consumer.get("base_fractal_type") == base_lane, consumer
    assert consumer.get("root_layout_kind") == "regular_ngon_v1", consumer
    assert consumer.get("requested_generated_root_count") == 5, consumer
    assert consumer.get("root_count") == 5, consumer
    assert consumer.get("trap_strength") == pytest.approx(1.0), consumer
    assert consumer.get("trap_scale") == pytest.approx(1.75), consumer
    assert isinstance(consumer.get("base_root_hash"), str), consumer
    assert isinstance(consumer.get("effective_root_hash"), str), consumer
    patterns = derived.get("root_patterns")
    active_root_field = derived.get("active_root_field")
    assert isinstance(active_root_field, dict), derived
    assert active_root_field.get("ref") == "dynamics_root_field", active_root_field
    assert active_root_field.get("label") == "Dynamics Root Field", active_root_field
    assert active_root_field.get("root_count") == 5, active_root_field
    assert active_root_field.get("layout_kind") == "regular_ngon_v1", active_root_field
    assert isinstance(patterns, list), derived
    pattern_by_ref = {pattern.get("ref"): pattern for pattern in patterns if isinstance(pattern, dict)}
    assert pattern_by_ref["dynamics_root_field"].get("root_count") == 5, patterns
    assert pattern_by_ref["dynamics_root_field"].get("layout_kind") == "regular_ngon_v1", patterns
    assert pattern_by_ref["color_root_field"].get("root_count") == 4, patterns
    assert pattern_by_ref["color_root_field"].get("layout_kind") == "legacy_quartic_v1", patterns
    pattern_consumers = derived.get("root_pattern_consumers")
    assert isinstance(pattern_consumers, list), derived
    assert any(
        isinstance(item, dict)
        and item.get("consumer_kind") == "root_field_consumer"
        and item.get("pattern_ref") == "dynamics_root_field"
        for item in pattern_consumers
    ), pattern_consumers
    assert any(
        isinstance(item, dict)
        and item.get("consumer_kind") == "color_source_row"
        and item.get("consumer_id") == "root_proximity"
        and item.get("pattern_ref") == "color_root_field"
        for item in pattern_consumers
    ), pattern_consumers

    replay_a = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(finding_state_path),
        "--capture-diagnostic",
    )
    replay_b = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(finding_state_path),
        "--capture-diagnostic",
    )
    assert replay_b["frame_hash"] == replay_a["frame_hash"]
