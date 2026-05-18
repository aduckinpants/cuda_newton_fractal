from __future__ import annotations

import copy

import pytest

from tests.runtime_harness import (
    active_runtime_exe as _active_runtime_exe,
    run_headless_capture as _run_headless_capture,
    write_state_bundle as _write_state_bundle,
)


def test_counterfactual_pair_headless_capture_is_runtime_visible() -> None:
    exe_path = _active_runtime_exe()

    pair_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "counterfactual_pair",
        "--width",
        "320",
        "--height",
        "240",
    )

    state = pair_capture["state"]
    assert state["fractal_type"] == "counterfactual_pair"

    replay_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "counterfactual_pair",
        "--width",
        "320",
        "--height",
        "240",
    )
    assert replay_capture["frame_hash"] == pair_capture["frame_hash"]

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
    assert pair_capture["frame_hash"] != newton_capture["frame_hash"]


def test_counterfactual_pair_headless_capture_persists_hardening_controls() -> None:
    exe_path = _active_runtime_exe()

    pair_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "counterfactual_pair",
        "--width",
        "320",
        "--height",
        "240",
    )

    params = pair_capture["state"]["params"]
    assert params["counterfactual_pair_root_family"] == "cubic_unit_roots"
    assert params["counterfactual_pair_frame"] == "world_absolute"
    assert params["counterfactual_pair_offset_x"] == pytest.approx(0.16)
    assert params["counterfactual_pair_offset_y"] == pytest.approx(0.08)
    assert params["counterfactual_pair_reconvergence_ratio"] == pytest.approx(0.6)


def test_counterfactual_pair_loaded_state_reads_frame_and_root_family(tmp_path) -> None:
    exe_path = _active_runtime_exe()

    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "counterfactual_pair",
        "--width",
        "320",
        "--height",
        "240",
    )

    world_state = copy.deepcopy(baseline_capture["state"])
    world_state["view"]["zoom"] = 16.0
    world_state["view"]["log2_zoom"] = 4.0
    world_state["params"]["counterfactual_pair_root_family"] = "cubic_unit_roots"
    world_state["params"]["counterfactual_pair_frame"] = "world_absolute"

    relative_state = copy.deepcopy(world_state)
    relative_state["params"]["counterfactual_pair_frame"] = "view_relative"

    quartic_state = copy.deepcopy(world_state)
    quartic_state["params"]["counterfactual_pair_root_family"] = "quartic_unit_roots"

    world_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "world_absolute", world_state)),
        "--capture-diagnostic",
    )
    relative_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "view_relative", relative_state)),
        "--capture-diagnostic",
    )
    quartic_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "quartic_family", quartic_state)),
        "--capture-diagnostic",
    )

    assert world_capture["state"]["params"]["counterfactual_pair_frame"] == "world_absolute"
    assert relative_capture["state"]["params"]["counterfactual_pair_frame"] == "view_relative"
    assert quartic_capture["state"]["params"]["counterfactual_pair_root_family"] == "quartic_unit_roots"
    assert world_capture["frame_hash"] != relative_capture["frame_hash"]
    assert world_capture["frame_hash"] != quartic_capture["frame_hash"]
