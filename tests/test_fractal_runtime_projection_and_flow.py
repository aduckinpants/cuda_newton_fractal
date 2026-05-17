from __future__ import annotations

import copy

import pytest

from tests.runtime_harness import (
    active_runtime_exe as _active_runtime_exe,
    run_headless_capture as _run_headless_capture,
    write_state_bundle as _write_state_bundle,
)


def test_projection_and_flow_headless_capture_is_runtime_visible() -> None:
    exe_path = _active_runtime_exe()

    projection_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "projection_and_flow",
        "--width",
        "320",
        "--height",
        "240",
    )

    state = projection_capture["state"]
    assert state["fractal_type"] == "projection_and_flow"

    replay_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "projection_and_flow",
        "--width",
        "320",
        "--height",
        "240",
    )
    assert replay_capture["frame_hash"] == projection_capture["frame_hash"]

    newton_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "newton",
        "--width",
        "320",
        "--height",
        "240",
    )
    assert projection_capture["frame_hash"] != newton_capture["frame_hash"]


def test_projection_and_flow_headless_capture_persists_hardening_controls() -> None:
    exe_path = _active_runtime_exe()

    projection_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "projection_and_flow",
        "--width",
        "320",
        "--height",
        "240",
    )

    params = projection_capture["state"]["params"]
    assert params["projection_and_flow_root_family"] == "cubic_unit_roots"
    assert params["projection_and_flow_target_radius"] == pytest.approx(1.0)
    assert params["projection_and_flow_pressure_threshold"] == pytest.approx(1.0)


def test_projection_and_flow_loaded_state_reads_root_family_and_target_radius(tmp_path) -> None:
    exe_path = _active_runtime_exe()

    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "projection_and_flow",
        "--width",
        "320",
        "--height",
        "240",
    )

    quartic_state = copy.deepcopy(baseline_capture["state"])
    quartic_state["params"]["projection_and_flow_root_family"] = "quartic_unit_roots"
    quartic_state["params"]["poly_kind"] = 1

    radius_state = copy.deepcopy(baseline_capture["state"])
    radius_state["params"]["projection_and_flow_target_radius"] = 1.75
    radius_state["params"]["projection_and_flow_pressure_threshold"] = 0.5

    quartic_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "quartic_family", quartic_state)),
        "--capture-diagnostic",
    )
    radius_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "radius_variant", radius_state)),
        "--capture-diagnostic",
    )

    assert quartic_capture["state"]["params"]["projection_and_flow_root_family"] == "quartic_unit_roots"
    assert radius_capture["state"]["params"]["projection_and_flow_target_radius"] == pytest.approx(1.75)
    assert radius_capture["state"]["params"]["projection_and_flow_pressure_threshold"] == pytest.approx(0.5)
    assert quartic_capture["frame_hash"] != baseline_capture["frame_hash"]
    assert radius_capture["frame_hash"] != baseline_capture["frame_hash"]
