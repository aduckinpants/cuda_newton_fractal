from __future__ import annotations

import json
import os
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)

_RUNTIME_VIEWER_E2E_OPT_IN_ENV = "VIEWER_HOST_ENABLE_RUNTIME_VIEWER_E2E"
pytestmark = pytest.mark.skipif(
    os.environ.get(_RUNTIME_VIEWER_E2E_OPT_IN_ENV) != "1",
    reason=(
        "runtime Lens SDF backend tests launch the viewer; set "
        "VIEWER_HOST_ENABLE_RUNTIME_VIEWER_E2E=1 for an explicit opt-in run"
    ),
)


@pytest.fixture(autouse=True)
def _serialize_runtime_viewer_automation():
    with runtime_automation_lock():
        yield


def _require_rendered_frame_hash(payload: dict[str, object]) -> str:
    assert payload.get("rendered_frame_ready") is True, payload
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), payload
    return frame_hash


def _expect_lens_report(payload: dict[str, object], *, base_hash: str) -> None:
    assert payload.get("lens_sdf_enabled") is True, payload
    assert payload.get("lens_sdf_valid") is True, payload
    assert payload.get("lens_sdf_backend_used") == "cuda_jfa", payload
    assert payload.get("lens_sdf_fallback_used") is False, payload
    assert payload.get("lens_sdf_pixel_scale") == pytest.approx(2.0), payload

    render_width = payload.get("rendered_frame_width")
    render_height = payload.get("rendered_frame_height")
    assert isinstance(render_width, int) and render_width > 0, payload
    assert isinstance(render_height, int) and render_height > 0, payload
    assert payload.get("lens_sdf_width") == (render_width + 1) // 2, payload
    assert payload.get("lens_sdf_height") == (render_height + 1) // 2, payload
    assert _require_rendered_frame_hash(payload) == base_hash, payload


def test_live_lens_sdf_auto_backend_reports_cuda_without_changing_base_frame(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "mandelbrot",
        "--width",
        "160",
        "--height",
        "120",
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    render = state["render"]
    assert isinstance(render, dict)
    render["width"] = 160
    render["height"] = 120
    state_path = write_state_bundle(tmp_path / "lens_sdf_backend", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "lens_sdf_backend_report.json",
        command_path=tmp_path / "lens_sdf_backend_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.lens_enabled.primary", timeout_seconds=15.0)
        base_hash = _require_rendered_frame_hash(viewer.wait_for_report(timeout_seconds=15.0))

        enabled_payload = viewer.set_control_value(
            "fractal_control.lens_enabled.primary",
            1.0,
            timeout_seconds=20.0,
        )
        _expect_lens_report(enabled_payload, base_hash=base_hash)
