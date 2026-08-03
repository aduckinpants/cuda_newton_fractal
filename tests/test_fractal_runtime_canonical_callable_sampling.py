from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import RUNTIME_DIR, active_runtime_exe


REPO_ROOT = Path(__file__).resolve().parents[1]
FIXTURE_STATE = REPO_ROOT / "tests" / "fixtures" / "rational_escape_numeric_truth_v1" / "state.json"


def _run_probe(state_path: Path, denominator_power: int) -> dict[str, object]:
    request = {
        "request_version": 1,
        "request_id": f"canonical-rational-escape-power-{denominator_power}",
        "function_id": "fractal.sample",
        "mode": "point_set",
        "base_state": {"load_state_json": str(state_path)},
        "overrides": [
            {
                "path": "fractal.params.explaino_rational_escape_denominator_power",
                "value": denominator_power,
            },
            {"path": "fractal.params.explaino_warp_strength", "value": 0.0},
            {"path": "fractal.params.max_iter", "value": 1},
        ],
        "points": [{"x": 0.5, "y": 0.2}],
        "metrics": ["iterations", "status", "final_z", "final_abs2"],
    }
    result = subprocess.run(
        [str(active_runtime_exe()), "--sample-request-stdin", "--sample-response-stdout"],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(request),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    response = json.loads(result.stdout)
    assert response["ok"] is True
    assert response["runtime"]["backend_used"] == "cuda"
    assert response["runtime"]["fractal_type"] == "explaino_rational_escape"
    assert len(response["samples"]) == 1
    return response


def _final_z(response: dict[str, object]) -> tuple[float, float]:
    sample = response["samples"][0]
    return (float(sample["final_z_x"]), float(sample["final_z_y"]))


def test_published_callable_uses_canonical_denominator_power_and_reports_actual_arithmetic(
    tmp_path: Path,
) -> None:
    if sys.platform != "win32":
        pytest.skip("canonical CUDA callable regression is Windows-only")

    base_state = json.loads(FIXTURE_STATE.read_text(encoding="utf-8"))
    states: dict[str, Path] = {}
    for tier in ("fast", "standard"):
        state = json.loads(json.dumps(base_state))
        state["render"]["sample_tier"] = tier
        path = tmp_path / tier / "state.json"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
        states[tier] = path

    fast_one = _run_probe(states["fast"], 1)
    fast_six = _run_probe(states["fast"], 6)
    standard_one = _run_probe(states["standard"], 1)
    standard_six = _run_probe(states["standard"], 6)

    assert fast_one["runtime"]["iteration_arithmetic"] == "float32"
    assert fast_six["runtime"]["iteration_arithmetic"] == "float32"
    assert standard_one["runtime"]["iteration_arithmetic"] == "float64"
    assert standard_six["runtime"]["iteration_arithmetic"] == "float64"

    assert _final_z(fast_one) != _final_z(fast_six)
    assert _final_z(standard_one) != _final_z(standard_six)

    repeat = _run_probe(states["standard"], 6)
    assert _final_z(repeat) == _final_z(standard_six)
    assert repeat["samples"][0]["status"] == standard_six["samples"][0]["status"]
