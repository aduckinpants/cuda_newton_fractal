from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
DIAGNOSTICS_STATE_FILE = RUNTIME_DIR / "diagnostics" / "last" / "state.json"
DIAGNOSTICS_FRAME_FILE = RUNTIME_DIR / "diagnostics" / "last" / "frame.bmp"


def _active_runtime_exe() -> Path:
    if not ACTIVE_RUNTIME_FILE.exists():
        pytest.skip(f"missing active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    active_name = ACTIVE_RUNTIME_FILE.read_text(encoding="utf-8").strip()
    if not active_name:
        pytest.skip(f"empty active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    exe_path = RUNTIME_DIR / active_name
    if not exe_path.exists():
        pytest.skip(f"active runtime missing: {exe_path}")
    return exe_path


def _run_headless_capture(*args: str) -> dict[str, object]:
    DIAGNOSTICS_STATE_FILE.unlink(missing_ok=True)
    DIAGNOSTICS_FRAME_FILE.unlink(missing_ok=True)

    result = subprocess.run(
        list(args),
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"
    assert DIAGNOSTICS_FRAME_FILE.exists(), f"missing diagnostics frame file: {DIAGNOSTICS_FRAME_FILE}"

    return {
        "state": json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8")),
        "frame_hash": hashlib.sha256(DIAGNOSTICS_FRAME_FILE.read_bytes()).hexdigest(),
    }


def _write_state_bundle(tmp_path: Path, state: dict[str, object]) -> Path:
    state_path = tmp_path / "state.json"
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
    return state_path


def _configure_sidecar_policy(state: dict[str, object], **updates: object) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    policy = configured_state["sidecar_auto_demo_policy"]
    assert isinstance(policy, dict)
    policy.update(updates)
    return configured_state


def _with_sidecar_mutation_history(
    state: dict[str, object], history: list[dict[str, object]], **param_updates: object
) -> dict[str, object]:
    configured_state = json.loads(json.dumps(state))
    configured_state["sidecar_mutation_history"] = history
    params = configured_state["params"]
    assert isinstance(params, dict)
    params.update(param_updates)
    return configured_state


def _numeric_state_deltas(before: dict[str, object], after: dict[str, object], *, abs_tol: float = 1.0e-7) -> list[str]:
    changed: list[str] = []
    for section_name in ("view", "params"):
        before_section = before[section_name]
        after_section = after[section_name]
        assert isinstance(before_section, dict)
        assert isinstance(after_section, dict)
        for key, before_value in before_section.items():
            after_value = after_section.get(key)
            if not isinstance(before_value, (int, float)) or isinstance(before_value, bool):
                continue
            if not isinstance(after_value, (int, float)) or isinstance(after_value, bool):
                continue
            if abs(float(after_value) - float(before_value)) > abs_tol:
                changed.append(f"{section_name}.{key}")
    return changed


def test_explaino_lambda_cli_overrides_survive_headless_capture() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino-Lambda runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "explaino_lambda",
            "--lambda-real",
            "2.9685855",
            "--lambda-imag",
            "-0.27446103",
            "--explaino-seed",
            "3.25",
            "--explaino-warp-strength",
            "0.2",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"

    state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    assert state["fractal_type"] == "explaino_lambda"
    assert state["params"]["lambda_real"] == pytest.approx(2.9685855, abs=1e-5)
    assert state["params"]["lambda_imag"] == pytest.approx(-0.27446103, abs=1e-5)
    assert state["params"]["explaino_seed"] == 3
    assert state["view"]["explaino_seed_drift"] == pytest.approx(0.25, abs=1e-6)
    assert state["params"]["explaino_warp_strength"] == pytest.approx(0.2, abs=1e-6)
    assert state["params"]["coloring_mode"] == "smooth_escape"


def test_explaino_rational_escape_cli_overrides_survive_headless_capture() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino-Rational-Escape runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "explaino_rational_escape",
            "--explaino-seed",
            "4.2",
            "--explaino-warp-strength",
            "0.35",
            "--width",
            "640",
            "--height",
            "480",
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert DIAGNOSTICS_STATE_FILE.exists(), f"missing diagnostics state file: {DIAGNOSTICS_STATE_FILE}"

    state = json.loads(DIAGNOSTICS_STATE_FILE.read_text(encoding="utf-8"))
    assert state["fractal_type"] == "explaino_rational_escape"
    assert state["params"]["explaino_seed"] == 4
    assert state["view"]["explaino_seed_drift"] == pytest.approx(0.2, abs=1e-6)
    assert state["params"]["explaino_warp_strength"] == pytest.approx(0.35, abs=1e-6)
    assert state["params"]["coloring_mode"] == "smooth_escape"
    assert state["render"]["width"] == 640
    assert state["render"]["height"] == 480


def test_explaino_composed_variants_render_distinct_default_frames() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino composed runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    hashes: dict[str, str] = {}
    expected_strengths = {
        "explaino_ripple": {
            "ripple_amplitude": 0.15,
            "splice_offset": 0.0,
            "vortex_strength": 0.0,
            "tension_strength": 0.0,
        },
        "explaino_splice": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.5,
            "vortex_strength": 0.0,
            "tension_strength": 0.0,
        },
        "explaino_vortex": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.0,
            "vortex_strength": 0.3,
            "tension_strength": 0.0,
        },
        "explaino_tension": {
            "ripple_amplitude": 0.0,
            "splice_offset": 0.0,
            "vortex_strength": 0.0,
            "tension_strength": 0.02,
        },
    }
    for variant, expected_params in expected_strengths.items():
        capture = _run_headless_capture(
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            variant,
            "--width",
            "320",
            "--height",
            "240",
        )
        state = capture["state"]
        assert state["fractal_type"] == variant
        for param_name, expected_value in expected_params.items():
            assert state["params"][param_name] == pytest.approx(expected_value, abs=1e-6)
        hashes[variant] = str(capture["frame_hash"])

    assert len(set(hashes.values())) == len(hashes), f"expected distinct default frame hashes, got {hashes}"


def test_explaino_composed_variant_state_round_trips_through_load_state_json(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino composed state round-trip regression is Windows-only")

    exe_path = _active_runtime_exe()
    initial_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_vortex",
        "--width",
        "320",
        "--height",
        "240",
    )
    saved_state_path = tmp_path / "state.json"
    saved_state_path.write_bytes(DIAGNOSTICS_STATE_FILE.read_bytes())

    reloaded_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(saved_state_path),
        "--capture-diagnostic",
    )

    initial_state = initial_capture["state"]
    reloaded_state = reloaded_capture["state"]
    assert reloaded_state["fractal_type"] == "explaino_vortex"
    assert reloaded_state["params"]["vortex_strength"] == pytest.approx(0.3, abs=1e-6)
    assert reloaded_state["params"]["ripple_amplitude"] == pytest.approx(0.0, abs=1e-6)
    assert "sidecar_orientation" in initial_state
    assert reloaded_state["sidecar_orientation"] == initial_state["sidecar_orientation"]
    assert "sidecar_auto_demo_policy" in initial_state
    assert reloaded_state["sidecar_auto_demo_policy"] == initial_state["sidecar_auto_demo_policy"]
    assert reloaded_capture["frame_hash"] == initial_capture["frame_hash"]
    assert reloaded_state["render"]["width"] == initial_state["render"]["width"]
    assert reloaded_state["render"]["height"] == initial_state["render"]["height"]


def test_explaino_sidecar_headless_apply_step_changes_state_and_frame(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino sidecar runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    configured_state = _configure_sidecar_policy(
        baseline_capture["state"],
        enabled=True,
        allow_runtime_mutation=True,
        run_paced_loop=False,
        paced_loop_interval_seconds=0.1,
        stop_demonstrated_fraction=1.0,
        stop_uncertain_count=0,
    )
    state_path = _write_state_bundle(tmp_path, configured_state)

    applied_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--sidecar-apply-armed-step-count",
        "1",
        "--capture-diagnostic",
    )

    changed_fields = _numeric_state_deltas(baseline_capture["state"], applied_capture["state"])
    assert changed_fields, "expected headless armed-step proof to mutate at least one numeric state field"
    assert applied_capture["frame_hash"] != baseline_capture["frame_hash"], (
        "expected headless armed-step proof to change the rendered frame hash"
    )


def test_explaino_sidecar_headless_apply_step_persists_mutation_history(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino sidecar runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    configured_state = _configure_sidecar_policy(
        baseline_capture["state"],
        enabled=True,
        allow_runtime_mutation=True,
        run_paced_loop=False,
        paced_loop_interval_seconds=0.1,
        stop_demonstrated_fraction=1.0,
        stop_uncertain_count=0,
    )
    state_path = _write_state_bundle(tmp_path / "apply", configured_state)

    applied_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--sidecar-apply-armed-step-count",
        "1",
        "--capture-diagnostic",
    )

    mutation_history = applied_capture["state"].get("sidecar_mutation_history")
    assert isinstance(mutation_history, list) and mutation_history, (
        "expected headless armed-step proof to persist at least one sidecar mutation-history record"
    )
    first_record = mutation_history[0]
    assert first_record["path"], "expected persisted mutation-history record to include a bound path"
    assert first_record["type"], "expected persisted mutation-history record to include a bound type"
    assert isinstance(first_record["target_value"], (int, float))

    reloaded_state_path = _write_state_bundle(tmp_path / "reload", applied_capture["state"])
    reloaded_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(reloaded_state_path),
        "--capture-diagnostic",
    )

    assert reloaded_capture["state"].get("sidecar_mutation_history") == mutation_history
    assert reloaded_capture["frame_hash"] == applied_capture["frame_hash"]


def test_explaino_sidecar_headless_replay_mutation_history_count_replays_ordered_targets(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino sidecar runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    mutation_history = [
        {
            "label": "replay-step-1",
            "path": "fractal.params.ripple_amplitude",
            "type": "float",
            "target_value": 0.12,
            "utility": 1.0,
        },
        {
            "label": "replay-step-2",
            "path": "fractal.params.ripple_amplitude",
            "type": "float",
            "target_value": 0.24,
            "utility": 1.0,
        },
    ]
    replay_state_path = _write_state_bundle(
        tmp_path / "replay",
        _with_sidecar_mutation_history(
            baseline_capture["state"],
            mutation_history,
            ripple_amplitude=0.5,
        ),
    )

    replay_one_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_state_path),
        "--sidecar-replay-mutation-history-count",
        "1",
        "--capture-diagnostic",
    )
    assert replay_one_capture["state"]["params"]["ripple_amplitude"] == pytest.approx(0.12, abs=1e-6)
    assert replay_one_capture["state"].get("sidecar_mutation_history") == mutation_history

    replay_two_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_state_path),
        "--sidecar-replay-mutation-history-count",
        "2",
        "--capture-diagnostic",
    )
    assert replay_two_capture["state"]["params"]["ripple_amplitude"] == pytest.approx(0.24, abs=1e-6)
    assert replay_two_capture["state"].get("sidecar_mutation_history") == mutation_history


def test_explaino_sidecar_headless_paced_loop_respects_stop_threshold(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino sidecar runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    baseline_capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino",
        "--width",
        "320",
        "--height",
        "240",
    )

    moving_state_path = _write_state_bundle(
        tmp_path / "moving",
        _configure_sidecar_policy(
            baseline_capture["state"],
            enabled=True,
            allow_runtime_mutation=True,
            run_paced_loop=True,
            paced_loop_interval_seconds=0.05,
            stop_demonstrated_fraction=1.0,
            stop_uncertain_count=0,
        ),
    )
    stopped_state_path = _write_state_bundle(
        tmp_path / "stopped",
        _configure_sidecar_policy(
            baseline_capture["state"],
            enabled=True,
            allow_runtime_mutation=True,
            run_paced_loop=True,
            paced_loop_interval_seconds=0.05,
            stop_demonstrated_fraction=0.0,
            stop_uncertain_count=0,
        ),
    )

    moving_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(moving_state_path),
        "--sidecar-pump-paced-loop-seconds",
        "0.20",
        "--capture-diagnostic",
    )
    stopped_capture = _run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(stopped_state_path),
        "--sidecar-pump-paced-loop-seconds",
        "0.20",
        "--capture-diagnostic",
    )

    moving_fields = _numeric_state_deltas(baseline_capture["state"], moving_capture["state"])
    stopped_fields = _numeric_state_deltas(baseline_capture["state"], stopped_capture["state"])

    assert moving_fields, "expected paced-loop proof to mutate at least one numeric state field"
    assert moving_capture["frame_hash"] != baseline_capture["frame_hash"], (
        "expected paced-loop proof to change the rendered frame hash"
    )
    assert not stopped_fields, (
        "expected zero-coverage stop threshold to halt the paced loop before mutating state"
    )
    assert stopped_capture["frame_hash"] == baseline_capture["frame_hash"], (
        "expected zero-coverage stop threshold to leave the rendered frame unchanged"
    )