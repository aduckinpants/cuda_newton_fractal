from __future__ import annotations

import copy

import pytest

from tests.runtime_harness import (
    active_runtime_exe as _active_runtime_exe,
    run_headless_capture as _run_headless_capture,
    write_state_bundle as _write_state_bundle,
)


def _bmp_non_black_pixel_count(frame_bytes: bytes) -> int:
    data_offset = int.from_bytes(frame_bytes[10:14], "little")
    width = int.from_bytes(frame_bytes[18:22], "little", signed=True)
    height = int.from_bytes(frame_bytes[22:26], "little", signed=True)
    bits_per_pixel = int.from_bytes(frame_bytes[28:30], "little")
    assert bits_per_pixel == 24
    row_stride = ((abs(width) * 3 + 3) // 4) * 4
    pixel_data = frame_bytes[data_offset:]
    non_black = 0
    for row in range(abs(height)):
        row_base = row * row_stride
        for col in range(abs(width)):
            pixel_base = row_base + col * 3
            if pixel_data[pixel_base : pixel_base + 3] != b"\x00\x00\x00":
                non_black += 1
    return non_black


def _with_projection_flow_smooth_escape_state(state: dict[str, object], *, radius: float) -> dict[str, object]:
    configured_state = copy.deepcopy(state)
    params = configured_state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "projection_and_flow_target_radius": radius,
            "coloring_mode": "smooth_escape",
            "color_signal": "smooth_escape",
            "color_shape": "identity",
            "color_palette": "cyclic_escape",
            "color_grading": "escape_default",
            "color_smooth_escape_scale": 1.0,
            "color_smooth_escape_bias": 0.0,
        }
    )
    return configured_state


def _bmp_unique_color_count(frame_bytes: bytes) -> int:
    data_offset = int.from_bytes(frame_bytes[10:14], "little")
    width = int.from_bytes(frame_bytes[18:22], "little", signed=True)
    height = int.from_bytes(frame_bytes[22:26], "little", signed=True)
    bits_per_pixel = int.from_bytes(frame_bytes[28:30], "little")
    assert bits_per_pixel == 24
    row_stride = ((abs(width) * 3 + 3) // 4) * 4
    pixel_data = frame_bytes[data_offset:]
    colors: set[bytes] = set()
    for row in range(abs(height)):
        row_base = row * row_stride
        for col in range(abs(width)):
            pixel_base = row_base + col * 3
            colors.add(pixel_data[pixel_base : pixel_base + 3])
    return len(colors)


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


@pytest.mark.parametrize("radius", [0.75, 1.75])
def test_projection_and_flow_nonunit_radius_capture_still_has_multiple_render_classes(tmp_path, radius: float) -> None:
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

    varied_state = copy.deepcopy(baseline_capture["state"])
    varied_state["params"]["projection_and_flow_target_radius"] = radius

    varied_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / f"radius_{radius}", varied_state)),
        "--capture-diagnostic",
    )

    assert varied_capture["state"]["params"]["projection_and_flow_target_radius"] == pytest.approx(radius)
    assert _bmp_unique_color_count(varied_capture["frame_bytes"]) >= 6


def test_projection_and_flow_distinct_nonunit_radii_do_not_collapse_to_the_same_public_render(tmp_path) -> None:
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

    variant_hashes: dict[float, str] = {}
    for radius in (1.25, 1.75):
        varied_state = copy.deepcopy(baseline_capture["state"])
        varied_state["params"]["projection_and_flow_target_radius"] = radius
        varied_capture = _run_headless_capture(
            str(exe_path),
            "--load-state-json",
            str(_write_state_bundle(tmp_path / f"radius_hash_{radius}", varied_state)),
            "--capture-diagnostic",
        )
        variant_hashes[radius] = varied_capture["frame_hash"]

    assert variant_hashes[1.25] != variant_hashes[1.75]


def test_projection_and_flow_smooth_escape_runtime_capture_keeps_stable_classes_visible(tmp_path) -> None:
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

    smooth_escape_state = _with_projection_flow_smooth_escape_state(
        baseline_capture["state"],
        radius=1.75,
    )
    smooth_escape_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "projection_flow_smooth_escape", smooth_escape_state)),
        "--capture-diagnostic",
    )

    params = smooth_escape_capture["state"]["params"]
    assert params["projection_and_flow_target_radius"] == pytest.approx(1.75)
    assert params["coloring_mode"] == "smooth_escape"
    assert params["color_signal"] == "smooth_escape"
    assert params["color_palette"] == "cyclic_escape"
    assert _bmp_unique_color_count(smooth_escape_capture["frame_bytes"]) > 8
    assert _bmp_non_black_pixel_count(smooth_escape_capture["frame_bytes"]) > 0
    assert smooth_escape_capture["frame_hash"] != baseline_capture["frame_hash"]


def test_projection_and_flow_smooth_escape_runtime_capture_responds_to_pressure_threshold(tmp_path) -> None:
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

    tight_threshold_state = _with_projection_flow_smooth_escape_state(
        baseline_capture["state"],
        radius=1.75,
    )
    tight_threshold_state["params"]["projection_and_flow_pressure_threshold"] = 0.25
    loose_threshold_state = _with_projection_flow_smooth_escape_state(
        baseline_capture["state"],
        radius=1.75,
    )
    loose_threshold_state["params"]["projection_and_flow_pressure_threshold"] = 2.0

    tight_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "projection_flow_threshold_tight", tight_threshold_state)),
        "--capture-diagnostic",
    )
    loose_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(_write_state_bundle(tmp_path / "projection_flow_threshold_loose", loose_threshold_state)),
        "--capture-diagnostic",
    )

    assert tight_capture["state"]["params"]["projection_and_flow_pressure_threshold"] == pytest.approx(0.25)
    assert loose_capture["state"]["params"]["projection_and_flow_pressure_threshold"] == pytest.approx(2.0)
    assert tight_capture["frame_hash"] != loose_capture["frame_hash"]
