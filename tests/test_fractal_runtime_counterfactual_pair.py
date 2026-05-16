from __future__ import annotations

from tests.runtime_harness import active_runtime_exe as _active_runtime_exe, run_headless_capture as _run_headless_capture


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
