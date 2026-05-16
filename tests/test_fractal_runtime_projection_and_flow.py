from __future__ import annotations

from tests.runtime_harness import (
    active_runtime_exe as _active_runtime_exe,
    run_headless_capture as _run_headless_capture,
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
