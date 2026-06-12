from __future__ import annotations

import json
import sys
from pathlib import Path

import pytest

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "tools" / "reality_toolkit"))

from fractal_explorer import explaino_slime_trace_runner as trace_runner


REQUIRED_ARTIFACTS = {
    "slime_trace_manifest.json",
    "initial_state.json",
    "final_state.json",
    "mutation_trace.jsonl",
    "root_samples.jsonl",
    "measurement_samples.jsonl",
    "trace_summary.json",
}

REQUIRED_STEP_FIELDS = {
    "step_index",
    "path",
    "type",
    "previous_value",
    "target_value",
    "applied_value",
    "utility",
    "selection_reason",
    "pre_state_hash",
    "post_state_hash",
    "measurement_hash",
    "root_authority",
    "roots_at_step",
    "scene_id",
    "rng_seed",
    "policy_id",
}


def _write_explaino_state(path: Path) -> dict[str, object]:
    state: dict[str, object] = {
        "state_version": 3,
        "fractal_type": "explaino_all",
        "view": {"center_x": 0.0, "center_y": 0.0, "zoom": 1.0},
        "params": {
            "poly_coeffs": [-1.0, 0.0, 0.0, 1.0, 0.0],
            "explaino_root_authority": "generated",
            "explaino_root_count": 2,
            "explaino_roots": [
                {"x": 0.25, "y": 0.5},
                {"x": 0.25, "y": -0.5},
            ],
            "ripple_amplitude": 0.15,
            "splice_offset": 0.5,
            "vortex_strength": 0.3,
            "tension_strength": 0.02,
            "non_numeric_note": "ignore me",
        },
        "render": {"width": 128, "height": 96},
    }
    path.write_text(json.dumps(state, indent=2), encoding="utf-8")
    return state


def _read_jsonl(path: Path) -> list[dict[str, object]]:
    return [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines() if line.strip()]


def test_run_explaino_slime_trace_writes_required_v1_artifacts(tmp_path: Path) -> None:
    state_path = tmp_path / "state.json"
    original_state = _write_explaino_state(state_path)
    out_dir = tmp_path / "trace"

    summary = trace_runner.run_explaino_slime_trace(
        state_path=state_path,
        out_dir=out_dir,
        steps=3,
        policy_id="unit_policy_v1",
        rng_seed=None,
    )

    assert REQUIRED_ARTIFACTS <= {path.name for path in out_dir.iterdir()}
    assert summary["steps_applied"] == 3
    assert json.loads((out_dir / "initial_state.json").read_text(encoding="utf-8")) == original_state

    manifest = json.loads((out_dir / "slime_trace_manifest.json").read_text(encoding="utf-8"))
    assert manifest["schema_id"] == "viewer.explaino_slime_trace.v1"
    assert manifest["policy_id"] == "unit_policy_v1"
    assert set(manifest["artifacts"].values()) == REQUIRED_ARTIFACTS

    mutation_rows = _read_jsonl(out_dir / "mutation_trace.jsonl")
    root_rows = _read_jsonl(out_dir / "root_samples.jsonl")
    measurement_rows = _read_jsonl(out_dir / "measurement_samples.jsonl")
    assert len(mutation_rows) == 3
    assert len(root_rows) == 3
    assert len(measurement_rows) == 3

    for index, row in enumerate(mutation_rows, start=1):
        assert REQUIRED_STEP_FIELDS <= set(row)
        assert row["step_index"] == index
        assert row["policy_id"] == "unit_policy_v1"
        assert row["root_authority"] == "generated"
        assert row["roots_at_step"] == original_state["params"]["explaino_roots"]
        assert row["measurement_hash"] == measurement_rows[index - 1]["measurement_hash"]
        assert row["pre_state_hash"] != row["post_state_hash"]

    assert [row["path"] for row in mutation_rows] == [
        "params.ripple_amplitude",
        "params.splice_offset",
        "params.vortex_strength",
    ]
    assert mutation_rows[0]["previous_value"] == pytest.approx(0.15)
    assert mutation_rows[0]["applied_value"] != pytest.approx(0.15)
    assert root_rows[0]["root_source"] == "captured_runtime"
    assert root_rows[0]["root_authority"] == "generated"

    final_state = json.loads((out_dir / "final_state.json").read_text(encoding="utf-8"))
    assert final_state["params"]["ripple_amplitude"] == pytest.approx(mutation_rows[0]["applied_value"])
    assert final_state["params"]["splice_offset"] == pytest.approx(mutation_rows[1]["applied_value"])
    assert final_state["params"]["vortex_strength"] == pytest.approx(mutation_rows[2]["applied_value"])


def test_run_explaino_slime_trace_is_deterministic(tmp_path: Path) -> None:
    state_path = tmp_path / "state.json"
    _write_explaino_state(state_path)

    trace_runner.run_explaino_slime_trace(
        state_path=state_path,
        out_dir=tmp_path / "trace_a",
        steps=4,
        policy_id="unit_policy_v1",
        rng_seed=1234,
    )
    trace_runner.run_explaino_slime_trace(
        state_path=state_path,
        out_dir=tmp_path / "trace_b",
        steps=4,
        policy_id="unit_policy_v1",
        rng_seed=1234,
    )

    for name in [
        "slime_trace_manifest.json",
        "mutation_trace.jsonl",
        "root_samples.jsonl",
        "measurement_samples.jsonl",
        "trace_summary.json",
    ]:
        assert (tmp_path / "trace_a" / name).read_text(encoding="utf-8") == (
            tmp_path / "trace_b" / name
        ).read_text(encoding="utf-8")


def test_run_explaino_slime_trace_rejects_malformed_captured_roots(tmp_path: Path) -> None:
    state_path = tmp_path / "state.json"
    state = _write_explaino_state(state_path)
    state["params"]["explaino_root_count"] = 2
    state["params"]["explaino_roots"] = [{"x": 0.25, "y": 0.5}]
    state_path.write_text(json.dumps(state), encoding="utf-8")

    with pytest.raises(ValueError, match="explaino_roots length"):
        trace_runner.run_explaino_slime_trace(
            state_path=state_path,
            out_dir=tmp_path / "trace",
            steps=1,
            policy_id="unit_policy_v1",
        )


def test_run_explaino_slime_trace_rejects_non_explaino_state(tmp_path: Path) -> None:
    state_path = tmp_path / "state.json"
    state = _write_explaino_state(state_path)
    state["fractal_type"] = "multibrot"
    state_path.write_text(json.dumps(state), encoding="utf-8")

    with pytest.raises(ValueError, match="requires an ExplainO fractal_type"):
        trace_runner.run_explaino_slime_trace(
            state_path=state_path,
            out_dir=tmp_path / "trace",
            steps=1,
            policy_id="unit_policy_v1",
        )
