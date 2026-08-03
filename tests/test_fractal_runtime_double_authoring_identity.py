from __future__ import annotations

import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


CONTROL_ID = "fractal_control.explaino_seed.primary"
RATIONAL_ESCAPE_SEED = 0.21797676384449005


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
def test_published_viewer_preserves_exact_combined_seed_authoring_identity(tmp_path: Path) -> None:
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
    assert state["fractal_type"] == "explaino_rational_escape"
    state["params"]["explaino_seed"] = 0.0
    state["view"]["explaino_seed_drift"] = 0.0
    state["view"]["auto_increment_seed"] = False
    state_path = write_state_bundle(tmp_path, state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "ui-report.json",
        command_path=tmp_path / "ui-command.json",
    ) as viewer:
        viewer.wait_for_report(timeout_seconds=30.0)
        viewer.wait_for_control(CONTROL_ID, timeout_seconds=30.0)
        report = viewer.set_control_value(CONTROL_ID, RATIONAL_ESCAPE_SEED, timeout_seconds=30.0)

    assert report["requested_set_control_id"] == CONTROL_ID
    assert report["set_value_consumed"] is True
    assert report["explaino_seed_combined"] == RATIONAL_ESCAPE_SEED
