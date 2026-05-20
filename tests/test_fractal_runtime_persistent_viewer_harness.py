from __future__ import annotations

import json
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


def _require_rendered_frame_hash(payload: dict[str, object]) -> str:
    assert payload.get("rendered_frame_ready") is True, (
        f"automation report did not include a ready rendered frame: {payload!r}"
    )
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), (
        f"automation report did not include a renderer-owned frame hash: {payload!r}"
    )
    return frame_hash


def test_persistent_runtime_viewer_batches_set_value_proofs_in_one_process(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "julia", "--width", "320", "--height", "240"
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    state_path = write_state_bundle(tmp_path / "julia_persistent", state)
    real_control = "fractal_control.julia_c_real.primary"
    imag_control = "fractal_control.julia_c_imag.primary"

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "persistent_report.json",
        command_path=tmp_path / "persistent_command.json",
    ) as viewer:
        viewer.wait_for_control(real_control)
        viewer.wait_for_control(imag_control)
        baseline_hash = _require_rendered_frame_hash(viewer.wait_for_report())

        real_payload = viewer.set_control_value(real_control, 0.285)
        real_hash = _require_rendered_frame_hash(real_payload)

        imag_payload = viewer.set_control_value(imag_control, 0.01)
        imag_hash = _require_rendered_frame_hash(imag_payload)

        assert viewer.launch_count == 1
        assert real_payload.get("ui_automation_command_sequence") == 1
        assert imag_payload.get("ui_automation_command_sequence") == 2

    assert real_hash != baseline_hash, "persistent viewer set-value for julia_c_real should change the rendered frame"
    assert imag_hash != real_hash, "persistent viewer second set-value should update the same running viewer"
