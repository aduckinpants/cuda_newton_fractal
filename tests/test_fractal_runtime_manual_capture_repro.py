from __future__ import annotations

from io import BytesIO
import json
from pathlib import Path
import shutil
import subprocess
import uuid

import pytest

from tests.runtime_harness import (
    DIAGNOSTICS_FRAME_FILE,
    DIAGNOSTICS_STATE_FILE,
    RUNTIME_DIR,
    active_runtime_exe as _active_runtime_exe,
)


MANUAL_CAPTURE_DIR = Path(
    r"D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-11\234919_563__explaino_inertial"
)
MANUAL_CAPTURE_STATE = MANUAL_CAPTURE_DIR / "state.json"
MANUAL_CAPTURE_FRAME = MANUAL_CAPTURE_DIR / "frame.png"


def _image_mean_abs_rgb(left, right) -> float:
    left_pixels = list(left.getdata())
    right_pixels = list(right.getdata())
    assert len(left_pixels) == len(right_pixels)
    total = 0
    for left_rgb, right_rgb in zip(left_pixels, right_pixels):
        total += abs(left_rgb[0] - right_rgb[0])
        total += abs(left_rgb[1] - right_rgb[1])
        total += abs(left_rgb[2] - right_rgb[2])
    return total / float(len(left_pixels) * 3)


def _resampling_lanczos():
    image_module = pytest.importorskip("PIL.Image")
    return getattr(getattr(image_module, "Resampling", image_module), "LANCZOS")


@pytest.mark.xfail(
    strict=True,
    reason=(
        "2026-05-11 ExplainO-Inertial archive is a known frame/state mismatch; "
        "keep this as the historical recovery tripwire"
    ),
)
def test_manual_explaino_inertial_capture_reloads_to_detailed_frame() -> None:
    image_module = pytest.importorskip("PIL.Image")
    if not MANUAL_CAPTURE_STATE.exists() or not MANUAL_CAPTURE_FRAME.exists():
        pytest.skip(f"missing manual Explaino-Inertial capture fixture: {MANUAL_CAPTURE_DIR}")

    exe_path = _active_runtime_exe()
    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)

    result = subprocess.run(
        [
            str(exe_path),
            "--load-state-json",
            str(MANUAL_CAPTURE_STATE),
            "--width",
            "256",
            "--height",
            "256",
            "--capture-diagnostic",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"
    assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing diagnostics frame file: {DIAGNOSTICS_FRAME_FILE}"

    loaded_state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    actual = image_module.open(BytesIO(DIAGNOSTICS_FRAME_FILE.read_bytes())).convert("RGB")
    reference = image_module.open(MANUAL_CAPTURE_FRAME).convert("RGB").resize(actual.size, _resampling_lanczos())
    actual_unique_colors = len(set(actual.getdata()))
    mean_abs_rgb = _image_mean_abs_rgb(reference, actual)

    failures = []
    if loaded_state["params"]["multibrot_power"] != 3:
        failures.append(f"multibrot_power reloaded as {loaded_state['params']['multibrot_power']}, expected 3")
    if actual_unique_colors <= 1000:
        failures.append(f"reloaded frame has only {actual_unique_colors} unique colors")
    if mean_abs_rgb >= 35.0:
        failures.append(f"reloaded frame mean_abs_rgb={mean_abs_rgb:.3f}, expected < 35.0")
    assert not failures, "\n".join(failures)


def test_capture_finding_preserves_wide_viewport_aspect_at_high_resolution() -> None:
    image_module = pytest.importorskip("PIL.Image")
    exe_path = _active_runtime_exe()
    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)
    seed = subprocess.run(
        [
            str(exe_path),
            "--width",
            "640",
            "--height",
            "360",
            "--capture-diagnostic",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert seed.returncode == 0, seed.stderr or seed.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing seed diagnostics state file: {DIAGNOSTICS_STATE_FILE}"

    seeded_state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    seeded_state["view"]["center_x"] = 0.125
    seeded_state["view"]["center_y"] = -0.25
    seeded_state["view"]["zoom"] = 8.0
    seeded_state["view"]["center_hp_x"] = 0.125
    seeded_state["view"]["center_hp_y"] = -0.25
    seeded_state["view"]["log2_zoom"] = 3.0
    source_state_dir = RUNTIME_DIR / f"wide_camera_state_{uuid.uuid4().hex}"
    source_state_dir.mkdir(parents=True, exist_ok=True)
    source_state_path = source_state_dir / "state.json"
    source_state_path.write_text(json.dumps(seeded_state), encoding="utf-8")

    finding_group = f"runtime_capture_aspect_{uuid.uuid4().hex}"
    finding_group_root = RUNTIME_DIR.parent / "findings" / finding_group
    shutil.rmtree(finding_group_root, ignore_errors=True)
    try:
        result = subprocess.run(
            [
                str(exe_path),
                "--load-state-json",
                str(source_state_path),
                "--width",
                "2048",
                "--height",
                "1152",
                "--capture-finding",
                "--finding-group",
                finding_group,
                "--finding-why",
                "runtime no-mouse capture aspect proof",
            ],
            cwd=str(RUNTIME_DIR),
            text=True,
            capture_output=True,
            check=False,
        )
        assert result.returncode == 0, result.stderr or result.stdout

        finding_dirs = sorted(path for path in finding_group_root.glob("*/*") if path.is_dir())
        assert len(finding_dirs) == 1
        finding_dir = finding_dirs[0]
        archived_state_path = finding_dir / "state.json"
        archived_frame_path = finding_dir / "frame.png"
        assert archived_state_path.exists()
        assert archived_frame_path.exists()

        archived_state = json.loads(archived_state_path.read_text(encoding="utf-8"))
        assert archived_state["render"]["width"] == 4096
        assert archived_state["render"]["height"] == 2304
        assert archived_state["view"]["center_hp_x"] == pytest.approx(0.125)
        assert archived_state["view"]["center_hp_y"] == pytest.approx(-0.25)
        assert archived_state["view"]["log2_zoom"] == pytest.approx(3.0)
        assert archived_state["view"]["center_x"] == pytest.approx(0.125)
        assert archived_state["view"]["center_y"] == pytest.approx(-0.25)
        assert archived_state["view"]["zoom"] == pytest.approx(8.0)

        with image_module.open(archived_frame_path) as archived_frame:
            assert archived_frame.size == (4096, 2304)
    finally:
        shutil.rmtree(source_state_dir, ignore_errors=True)
        shutil.rmtree(finding_group_root, ignore_errors=True)


def test_current_explaino_inertial_capture_finding_archive_replays_its_pixels() -> None:
    image_module = pytest.importorskip("PIL.Image")
    if not MANUAL_CAPTURE_STATE.exists():
        pytest.skip(f"missing manual ExplainO-Inertial seed fixture: {MANUAL_CAPTURE_STATE}")

    exe_path = _active_runtime_exe()
    finding_group = f"runtime_manual_explaino_repro_{uuid.uuid4().hex}"
    finding_group_root = RUNTIME_DIR.parent / "findings" / finding_group
    shutil.rmtree(finding_group_root, ignore_errors=True)
    try:
        result = subprocess.run(
            [
                str(exe_path),
                "--load-state-json",
                str(MANUAL_CAPTURE_STATE),
                "--capture-finding",
                "--finding-group",
                finding_group,
                "--finding-why",
                "runtime manual explaino replay proof",
            ],
            cwd=str(RUNTIME_DIR),
            text=True,
            capture_output=True,
            check=False,
        )
        assert result.returncode == 0, result.stderr or result.stdout

        finding_dirs = sorted(path for path in finding_group_root.glob("*/*") if path.is_dir())
        assert len(finding_dirs) == 1
        finding_dir = finding_dirs[0]
        archived_state_path = finding_dir / "state.json"
        archived_frame_path = finding_dir / "frame.png"
        assert archived_state_path.exists()
        assert archived_frame_path.exists()

        archived_state = json.loads(archived_state_path.read_text(encoding="utf-8"))
        assert len(archived_state["params"].get("explaino_roots", [])) == 4
        assert len(archived_state["params"].get("poly_coeffs_b", [])) == 5

        DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
        DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)
        replay = subprocess.run(
            [
                str(exe_path),
                "--load-state-json",
                str(archived_state_path),
                "--width",
                "256",
                "--height",
                "256",
                "--capture-diagnostic",
            ],
            cwd=str(RUNTIME_DIR),
            text=True,
            capture_output=True,
            check=False,
        )
        assert replay.returncode == 0, replay.stderr or replay.stdout
        assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing replay frame file: {DIAGNOSTICS_FRAME_FILE}"

        actual = image_module.open(BytesIO(DIAGNOSTICS_FRAME_FILE.read_bytes())).convert("RGB")
        reference = image_module.open(archived_frame_path).convert("RGB").resize(actual.size, _resampling_lanczos())
        assert _image_mean_abs_rgb(reference, actual) < 1.0
    finally:
        shutil.rmtree(finding_group_root, ignore_errors=True)


def test_current_explaino_inertial_capture_state_replays_its_pixels(tmp_path: Path) -> None:
    image_module = pytest.importorskip("PIL.Image")
    if not MANUAL_CAPTURE_STATE.exists():
        pytest.skip(f"missing manual ExplainO-Inertial seed fixture: {MANUAL_CAPTURE_STATE}")

    exe_path = _active_runtime_exe()
    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)

    first = subprocess.run(
        [
            str(exe_path),
            "--load-state-json",
            str(MANUAL_CAPTURE_STATE),
            "--width",
            "256",
            "--height",
            "256",
            "--capture-diagnostic",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert first.returncode == 0, first.stderr or first.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"
    assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing diagnostics frame file: {DIAGNOSTICS_FRAME_FILE}"

    emitted_state_path = tmp_path / "state.json"
    emitted_frame_path = tmp_path / "frame.bmp"
    emitted_state_path.write_bytes(DIAGNOSTICS_STATE_FILE.read_bytes())
    emitted_frame_path.write_bytes(DIAGNOSTICS_FRAME_FILE.read_bytes())

    emitted_state = json.loads(emitted_state_path.read_text(encoding="utf-8"))
    assert len(emitted_state["params"].get("explaino_roots", [])) == 4
    assert len(emitted_state["params"].get("poly_coeffs_b", [])) == 5

    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)
    replay = subprocess.run(
        [
            str(exe_path),
            "--load-state-json",
            str(emitted_state_path),
            "--width",
            "256",
            "--height",
            "256",
            "--capture-diagnostic",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert replay.returncode == 0, replay.stderr or replay.stdout
    assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing replay frame file: {DIAGNOSTICS_FRAME_FILE}"

    first_image = image_module.open(BytesIO(emitted_frame_path.read_bytes())).convert("RGB")
    replay_image = image_module.open(BytesIO(DIAGNOSTICS_FRAME_FILE.read_bytes())).convert("RGB")
    assert first_image.size == replay_image.size
    assert _image_mean_abs_rgb(first_image, replay_image) == 0.0
