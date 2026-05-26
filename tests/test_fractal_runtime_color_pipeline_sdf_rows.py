from __future__ import annotations

import json
import shutil
import subprocess
import sys
import time
import uuid
from pathlib import Path

import pytest

from tests.runtime_harness import (
    DIAGNOSTICS_FRAME_FILE,
    PersistentRuntimeViewerAutomation,
    RUNTIME_DIR,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


SDF_SOURCE_ROWS = (
    ("sdf_signed_distance", "smooth_escape", "cyclic_escape", 0.05, 0.5, 0.65, 0.2),
    ("sdf_inside_outside", "smooth_escape", "cyclic_escape", 1.0, 0.0, -1.0, 0.5),
    ("sdf_boundary_band", "smooth_escape", "cyclic_escape", 1.0, 0.0, 1.75, -0.25),
    ("sdf_normal_angle", "phase", "phase_wheel", 1.0, 0.0, -0.75, 0.35),
    ("sdf_curvature", "smooth_escape", "cyclic_escape", 0.25, 0.5, 1.25, -0.35),
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _resampling_lanczos():
    image_module = pytest.importorskip("PIL.Image")
    try:
        return image_module.Resampling.LANCZOS
    except AttributeError:
        return image_module.LANCZOS


def _image_mean_abs_rgb(left, right) -> float:
    assert left.size == right.size
    left_bytes = left.tobytes()
    right_bytes = right.tobytes()
    assert len(left_bytes) == len(right_bytes)
    return sum(abs(a - b) for a, b in zip(left_bytes, right_bytes)) / len(left_bytes)


def _manual_capture_dirs() -> set[Path]:
    root = RUNTIME_DIR.parent / "findings" / "manual_capture"
    if not root.exists():
        return set()
    return {path.resolve() for path in root.glob("*/*") if path.is_dir()}


def _wait_for_new_manual_capture(before: set[Path], *, timeout_seconds: float = 120.0) -> Path:
    deadline = time.monotonic() + timeout_seconds
    last_seen: list[Path] = []
    while time.monotonic() < deadline:
        after = _manual_capture_dirs()
        new_dirs = sorted(after - before, key=lambda path: path.stat().st_mtime if path.exists() else 0.0)
        last_seen = new_dirs
        for finding_dir in reversed(new_dirs):
            if (finding_dir / "state.json").exists() and (finding_dir / "frame.png").exists():
                return finding_dir
        time.sleep(0.2)
    raise AssertionError(f"manual Capture Finding did not create a complete finding; last_seen={last_seen!r}")


def _finding_dirs_for_group(group: str) -> list[Path]:
    root = RUNTIME_DIR.parent / "findings" / group
    if not root.exists():
        return []
    return sorted(path for path in root.glob("*/*") if path.is_dir())


def _first_color_pipeline_row(state: dict[str, object], lane_id: str) -> dict[str, object]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), "expected captured state to include color_pipeline_draft"
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), "expected color_pipeline_draft.lanes to be a list"
    for lane in lanes:
        if not isinstance(lane, dict) or lane.get("lane_id") != lane_id:
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list) and rows, f"expected lane {lane_id} to include rows"
        row = rows[0]
        assert isinstance(row, dict), f"expected lane {lane_id} first row to be an object"
        return row
    raise AssertionError(f"missing color pipeline lane: {lane_id}")


def _row_number(row: dict[str, object], path: str) -> float:
    params = row.get("parameter_values")
    assert isinstance(params, list), "expected parameter_values to be a list"
    for param in params:
        if not isinstance(param, dict) or param.get("path") != path:
            continue
        value = param.get("number_value")
        assert isinstance(value, (int, float)), f"expected numeric value for {path}"
        return float(value)
    raise AssertionError(f"missing color pipeline row param: {path}")


def _capture_sdf_source_row(
    *,
    exe_path: Path,
    state_path: Path,
    function_id: str,
    scale: float,
    bias: float,
    sdf_gate: str | None = None,
    sdf_gate_width_px: float | None = None,
) -> dict[str, object]:
    args: list[str] = [
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        f"select_function:source:0:{function_id}",
        "--color-pipeline-action",
        f"set_param:source:0:signal.scale:number:{scale}",
        "--color-pipeline-action",
        f"set_param:source:0:signal.bias:number:{bias}",
    ]
    if sdf_gate is not None:
        args.extend(
            [
                "--color-pipeline-action",
                f"set_param:source:0:signal.sdf_gate:enum:{sdf_gate}",
            ]
        )
    if sdf_gate_width_px is not None:
        args.extend(
            [
                "--color-pipeline-action",
                f"set_param:source:0:signal.sdf_gate_width_px:number:{sdf_gate_width_px}",
            ]
        )
    args.append("--capture-diagnostic")
    return run_headless_capture(*args)


def _capture_all_sdf_source_rows(
    *,
    exe_path: Path,
    state_path: Path,
) -> dict[str, object]:
    args: list[str] = [
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        f"select_function:source:0:{SDF_SOURCE_ROWS[0][0]}",
    ]
    for function_id, *_ in SDF_SOURCE_ROWS[1:]:
        args.extend(["--color-pipeline-action", f"add_row:source:{function_id}"])
    args.append("--capture-diagnostic")
    return run_headless_capture(*args)


def _assert_sdf_capture_state(
    capture: dict[str, object],
    *,
    function_id: str,
    expected_mode: str,
    expected_palette: str,
    scale: float,
    bias: float,
) -> None:
    state = capture["state"]
    assert isinstance(state, dict)
    params = state.get("params")
    assert isinstance(params, dict)
    assert params.get("coloring_mode") == expected_mode
    assert params.get("color_signal") == function_id
    assert params.get("color_palette") == expected_palette

    source_stack = params.get("color_source_stack")
    assert isinstance(source_stack, list) and len(source_stack) == 1
    source_entry = source_stack[0]
    assert isinstance(source_entry, dict)
    assert source_entry.get("signal") == function_id
    assert source_entry.get("scale") == pytest.approx(scale, abs=1e-6)
    assert source_entry.get("bias") == pytest.approx(bias, abs=1e-6)
    assert source_entry.get("sdf_gate", "none") in {"none", "boundary_band"}

    source_row = _first_color_pipeline_row(state, "source")
    assert source_row.get("function_id") == function_id
    assert _row_number(source_row, "signal.scale") == pytest.approx(scale, abs=1e-6)
    assert _row_number(source_row, "signal.bias") == pytest.approx(bias, abs=1e-6)


def test_color_pipeline_sdf_source_rows_are_live_backed_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline SDF runtime regression is Windows-only")

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
    neutral_state = json.loads(json.dumps(neutral_capture["state"]))
    state_path = write_state_bundle(tmp_path / "color_pipeline_sdf_rows", neutral_state)

    for function_id, expected_mode, expected_palette, baseline_scale, baseline_bias, edited_scale, edited_bias in SDF_SOURCE_ROWS:
        baseline = _capture_sdf_source_row(
            exe_path=exe_path,
            state_path=state_path,
            function_id=function_id,
            scale=baseline_scale,
            bias=baseline_bias,
        )
        edited = _capture_sdf_source_row(
            exe_path=exe_path,
            state_path=state_path,
            function_id=function_id,
            scale=edited_scale,
            bias=edited_bias,
        )

        _assert_sdf_capture_state(
            baseline,
            function_id=function_id,
            expected_mode=expected_mode,
            expected_palette=expected_palette,
            scale=baseline_scale,
            bias=baseline_bias,
        )
        _assert_sdf_capture_state(
            edited,
            function_id=function_id,
            expected_mode=expected_mode,
            expected_palette=expected_palette,
            scale=edited_scale,
            bias=edited_bias,
        )
        assert baseline["frame_hash"] != neutral_capture["frame_hash"], (
            f"expected {function_id} to use the live Lens SDF field and change the base frame"
        )
        assert edited["frame_hash"] != baseline["frame_hash"], (
            f"expected {function_id} signal.scale/signal.bias edits to change the live rendered frame"
        )


def test_sdf_source_boundary_gate_changes_normal_angle_frame_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline SDF runtime regression is Windows-only")

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
    state_path = write_state_bundle(
        tmp_path / "sdf_source_boundary_gate_seed",
        json.loads(json.dumps(neutral_capture["state"])),
    )

    full_field = _capture_sdf_source_row(
        exe_path=exe_path,
        state_path=state_path,
        function_id="sdf_normal_angle",
        scale=1.0,
        bias=0.0,
        sdf_gate="none",
        sdf_gate_width_px=2.0,
    )
    gated = _capture_sdf_source_row(
        exe_path=exe_path,
        state_path=state_path,
        function_id="sdf_normal_angle",
        scale=1.0,
        bias=0.0,
        sdf_gate="boundary_band",
        sdf_gate_width_px=1.0,
    )
    wide_gate = _capture_sdf_source_row(
        exe_path=exe_path,
        state_path=state_path,
        function_id="sdf_normal_angle",
        scale=1.0,
        bias=0.0,
        sdf_gate="boundary_band",
        sdf_gate_width_px=6.0,
    )

    gated_stack = gated["state"]["params"]["color_source_stack"]
    assert isinstance(gated_stack, list) and gated_stack
    assert gated_stack[0].get("sdf_gate") == "boundary_band"
    assert gated_stack[0].get("sdf_gate_width_px") == pytest.approx(1.0)
    assert gated["frame_hash"] != full_field["frame_hash"], (
        "expected boundary-gating sdf_normal_angle to mask the full-field diagnostic output"
    )
    assert wide_gate["frame_hash"] != gated["frame_hash"], (
        "expected SDF gate width to be a live source-local control"
    )


def test_capture_finding_preserves_sdf_source_row_pixels_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline SDF Capture Finding regression is Windows-only")

    image_module = pytest.importorskip("PIL.Image")
    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "mandelbrot",
        "--width",
        "192",
        "--height",
        "144",
    )
    neutral_state = json.loads(json.dumps(neutral_capture["state"]))
    seed_state_path = write_state_bundle(tmp_path / "capture_finding_seed", neutral_state)
    sdf_capture = _capture_sdf_source_row(
        exe_path=exe_path,
        state_path=seed_state_path,
        function_id="sdf_signed_distance",
        scale=0.05,
        bias=0.5,
    )
    assert sdf_capture["frame_hash"] != neutral_capture["frame_hash"]
    sdf_state_path = write_state_bundle(tmp_path / "capture_finding_sdf", sdf_capture["state"])

    report_path = tmp_path / "viewer_report.json"
    command_path = tmp_path / "viewer_command.json"
    before_manual = _manual_capture_dirs()
    ui_finding_dir: Path | None = None
    headless_group = f"runtime_sdf_capture_reference_{uuid.uuid4().hex}"
    headless_group_root = RUNTIME_DIR.parent / "findings" / headless_group
    shutil.rmtree(headless_group_root, ignore_errors=True)

    try:
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=sdf_state_path,
            report_path=report_path,
            command_path=command_path,
        ) as viewer:
            ready_report = viewer.wait_for_report(timeout_seconds=30.0)
            assert ready_report.get("lens_sdf_valid") is True, ready_report
            viewer.wait_for_control("capture_finding", timeout_seconds=20.0)
            viewer.click_control("capture_finding", timeout_seconds=180.0)

        ui_finding_dir = _wait_for_new_manual_capture(before_manual, timeout_seconds=180.0)
        ui_state_path = ui_finding_dir / "state.json"
        ui_frame_path = ui_finding_dir / "frame.png"
        assert ui_state_path.exists()
        assert ui_frame_path.exists()

        headless = subprocess.run(
            [
                str(exe_path),
                "--load-state-json",
                str(ui_state_path),
                "--capture-finding",
                "--finding-group",
                headless_group,
                "--finding-why",
                "runtime no-mouse SDF Capture Finding reference",
            ],
            cwd=str(RUNTIME_DIR),
            text=True,
            capture_output=True,
            check=False,
        )
        assert headless.returncode == 0, headless.stderr or headless.stdout
        reference_dirs = _finding_dirs_for_group(headless_group)
        assert len(reference_dirs) == 1
        reference_frame_path = reference_dirs[0] / "frame.png"
        assert reference_frame_path.exists()

        ui_frame = image_module.open(ui_frame_path).convert("RGB")
        reference_frame = image_module.open(reference_frame_path).convert("RGB")
        if reference_frame.size != ui_frame.size:
            reference_frame = reference_frame.resize(ui_frame.size, _resampling_lanczos())
        mean_abs_rgb = _image_mean_abs_rgb(ui_frame, reference_frame)
        assert mean_abs_rgb < 1.0, (
            f"Capture Finding archived pixels diverged from the SDF Color Pipeline replay: "
            f"mean_abs_rgb={mean_abs_rgb:.3f}, ui_finding={ui_finding_dir}, reference={reference_dirs[0]}"
        )
    finally:
        if ui_finding_dir is not None:
            shutil.rmtree(ui_finding_dir, ignore_errors=True)
        shutil.rmtree(headless_group_root, ignore_errors=True)
        DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)

def test_color_pipeline_sdf_source_controls_are_visible_and_live_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline SDF runtime regression is Windows-only")

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
    neutral_state = json.loads(json.dumps(neutral_capture["state"]))
    seed_state_path = write_state_bundle(tmp_path / "sdf_source_visible_seed", neutral_state)
    all_rows_capture = _capture_all_sdf_source_rows(
        exe_path=exe_path,
        state_path=seed_state_path,
    )
    all_rows_state = json.loads(json.dumps(all_rows_capture["state"]))
    all_rows_state_path = write_state_bundle(tmp_path / "sdf_source_visible_all_rows", all_rows_state)
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=all_rows_state_path,
        report_path=tmp_path / "sdf_source_visible_all_rows_report.json",
        command_path=tmp_path / "sdf_source_visible_all_rows_command.json",
        open_color_pipeline=True,
    ) as viewer:
        viewer.wait_for_report(timeout_seconds=30.0)
        for function_id, *_ in SDF_SOURCE_ROWS:
            for path in ("signal.scale", "signal.bias", "signal.sdf_gate", "signal.sdf_gate_width_px", "signal.blend_weight"):
                viewer.wait_for_control(
                    f"color_pipeline.source.{function_id}.{path}.primary",
                    timeout_seconds=20.0,
                )
        viewer.wait_for_control(
            "color_pipeline.source.sdf_boundary_band.signal.boundary_width_px.primary",
            timeout_seconds=20.0,
        )
        viewer.wait_for_control("color_pipeline.source.sdf_field.downsample.primary", timeout_seconds=20.0)

    boundary_capture = _capture_sdf_source_row(
        exe_path=exe_path,
        state_path=seed_state_path,
        function_id="sdf_boundary_band",
        scale=1.0,
        bias=0.0,
    )
    sdf_state = json.loads(json.dumps(boundary_capture["state"]))
    lens = sdf_state.setdefault("lens", {})
    assert isinstance(lens, dict)
    lens["enabled"] = False
    lens["downsample"] = 2
    state_path = write_state_bundle(tmp_path / "sdf_source_visible", sdf_state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "sdf_source_visible_report.json",
        command_path=tmp_path / "sdf_source_visible_command.json",
        open_color_pipeline=True,
    ) as viewer:
        ready_report = viewer.wait_for_report(timeout_seconds=30.0)
        base_hash = ready_report.get("rendered_frame_hash")
        assert isinstance(base_hash, str), ready_report
        for path in ("signal.scale", "signal.bias", "signal.blend_weight", "signal.boundary_width_px"):
            viewer.wait_for_control(
                f"color_pipeline.source.sdf_boundary_band.{path}.primary",
                timeout_seconds=20.0,
            )
        viewer.wait_for_control("color_pipeline.source.sdf_field.downsample.primary", timeout_seconds=20.0)

        scale_report = viewer.set_control_value(
            "color_pipeline.source.sdf_boundary_band.signal.scale.primary",
            1.5,
            timeout_seconds=40.0,
        )
        assert scale_report.get("rendered_frame_hash") != base_hash, scale_report
        bias_report = viewer.set_control_value(
            "color_pipeline.source.sdf_boundary_band.signal.bias.primary",
            0.35,
            timeout_seconds=40.0,
        )
        assert bias_report.get("rendered_frame_hash") != scale_report.get("rendered_frame_hash"), bias_report
        width_report = viewer.set_control_value(
            "color_pipeline.source.sdf_boundary_band.signal.boundary_width_px.primary",
            6.0,
            timeout_seconds=40.0,
        )
        assert width_report.get("rendered_frame_hash") != bias_report.get("rendered_frame_hash"), width_report
        downsample_report = viewer.set_control_value(
            "color_pipeline.source.sdf_field.downsample.primary",
            4.0,
            timeout_seconds=40.0,
        )

    assert downsample_report.get("lens_sdf_enabled") is False, downsample_report
    assert downsample_report.get("lens_sdf_valid") is True, downsample_report
    assert downsample_report.get("lens_sdf_pixel_scale") == pytest.approx(4.0), downsample_report
    render_pixels = int(downsample_report["rendered_frame_width"]) * int(downsample_report["rendered_frame_height"])
    field_pixels = int(downsample_report["lens_sdf_width"]) * int(downsample_report["lens_sdf_height"])
    direct_samples = int(downsample_report["lens_sdf_postprocess_direct_sample_count"])
    neighborhood_samples = int(downsample_report["lens_sdf_postprocess_neighborhood_sample_count"])
    assert int(downsample_report["lens_sdf_postprocess_pixel_step"]) == 1, downsample_report
    assert int(downsample_report["lens_sdf_postprocess_filled_pixel_count"]) == render_pixels, downsample_report
    assert direct_samples == field_pixels, downsample_report
    assert neighborhood_samples == 0, downsample_report
    assert direct_samples < render_pixels, downsample_report


def test_lens_downsample_visible_and_authoritative_for_sdf_source_without_lens_visualization_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline SDF runtime regression is Windows-only")

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
    neutral_state = json.loads(json.dumps(neutral_capture["state"]))
    seed_state_path = write_state_bundle(tmp_path / "lens_downsample_sdf_seed", neutral_state)
    sdf_capture = _capture_sdf_source_row(
        exe_path=exe_path,
        state_path=seed_state_path,
        function_id="sdf_normal_angle",
        scale=1.0,
        bias=0.0,
    )
    sdf_state = json.loads(json.dumps(sdf_capture["state"]))
    lens = sdf_state.setdefault("lens", {})
    assert isinstance(lens, dict)
    lens["enabled"] = False
    lens["downsample"] = 2
    state_path = write_state_bundle(tmp_path / "lens_downsample_sdf_visible", sdf_state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "lens_downsample_sdf_report.json",
        command_path=tmp_path / "lens_downsample_sdf_command.json",
    ) as viewer:
        ready_report = viewer.wait_for_report(timeout_seconds=30.0)
        assert ready_report.get("lens_sdf_enabled") is False, ready_report
        assert ready_report.get("lens_sdf_valid") is True, ready_report
        viewer.wait_for_control("fractal_control.lens_downsample.primary", timeout_seconds=20.0)
        edited_report = viewer.set_control_value(
            "fractal_control.lens_downsample.primary",
            4.0,
            timeout_seconds=40.0,
        )

    assert edited_report.get("lens_sdf_enabled") is False, edited_report
    assert edited_report.get("lens_sdf_valid") is True, edited_report
    assert edited_report.get("lens_sdf_pixel_scale") == pytest.approx(4.0), edited_report
    render_width = edited_report.get("rendered_frame_width")
    render_height = edited_report.get("rendered_frame_height")
    assert isinstance(render_width, int) and render_width > 0, edited_report
    assert isinstance(render_height, int) and render_height > 0, edited_report
    assert edited_report.get("lens_sdf_width") == (render_width + 3) // 4, edited_report
    assert edited_report.get("lens_sdf_height") == (render_height + 3) // 4, edited_report
    render_pixels = render_width * render_height
    field_pixels = int(edited_report["lens_sdf_width"]) * int(edited_report["lens_sdf_height"])
    direct_samples = int(edited_report["lens_sdf_postprocess_direct_sample_count"])
    neighborhood_samples = int(edited_report["lens_sdf_postprocess_neighborhood_sample_count"])
    assert int(edited_report["lens_sdf_postprocess_pixel_step"]) == 1, edited_report
    assert int(edited_report["lens_sdf_postprocess_filled_pixel_count"]) == render_pixels, edited_report
    assert direct_samples == 0, edited_report
    assert neighborhood_samples == field_pixels, edited_report
    assert neighborhood_samples < render_pixels, edited_report
