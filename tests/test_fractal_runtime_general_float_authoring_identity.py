from __future__ import annotations

import math
import struct
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


def _binary32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
def test_published_viewer_preserves_general_float_and_camera_authoring_identity(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_rational_escape",
        "--width",
        "320",
        "--height",
        "240",
    )
    state = capture["state"]
    state["params"]["explaino_seed"] = 0.0
    state["view"]["explaino_seed_drift"] = 0.0
    state["view"]["auto_increment_seed"] = False
    state_path = write_state_bundle(tmp_path, state)

    adjacent_float = 0.500000059604644775390625
    adjacent_binary32 = _binary32(adjacent_float)
    center_x = 0.123456789012345
    zoom = 12345.6789012345

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "ui-report.json",
        command_path=tmp_path / "ui-command.json",
    ) as viewer:
        viewer.wait_for_report(timeout_seconds=30.0)
        viewer.wait_for_control("fractal_control.explaino_seed_drift.primary", timeout_seconds=30.0)
        float_report = viewer.set_control_value(
            "fractal_control.explaino_seed_drift.primary",
            adjacent_float,
            timeout_seconds=30.0,
        )
        center_report = viewer.set_control_value(
            "fractal_control.center_x.primary",
            center_x,
            timeout_seconds=30.0,
        )
        zoom_report = viewer.set_control_value(
            "fractal_control.zoom.primary",
            zoom,
            timeout_seconds=30.0,
        )

    assert float_report["set_value_consumed"] is True
    assert float_report["explaino_seed_combined"] == adjacent_binary32
    assert center_report["set_value_consumed"] is True
    assert center_report["view_center_hp_x"] == center_x
    assert zoom_report["set_value_consumed"] is True
    assert zoom_report["view_log2_zoom"] == math.log2(zoom)
