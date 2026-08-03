from __future__ import annotations

import json
from pathlib import Path

from tests.runtime_harness import (
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


REPO_ROOT = Path(__file__).resolve().parents[1]
FIXTURE_STATE = REPO_ROOT / "tests" / "fixtures" / "rational_escape_numeric_truth_v1" / "state.json"


def _controlled_rational_escape_state(base_state: dict[str, object], *, tier: str) -> dict[str, object]:
    state = json.loads(json.dumps(base_state))
    state["fractal_type"] = "explaino_rational_escape"

    view = state["view"]
    assert isinstance(view, dict)
    view.update(
        {
            "center_x": 0.4671657681465149,
            "center_y": -0.8586328029632568,
            "center_hp_x": 0.4671657644151865,
            "center_hp_y": -0.8586328106310411,
            "zoom": 65536.0,
            "log2_zoom": 16.0,
            "explaino_phase": -0.8337900042533875,
            "explaino_seed_drift": 0.21797676384449005,
            "auto_max_iter": False,
        }
    )

    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "max_iter": 1200,
            "explaino_seed": 0,
            "explaino_warp_strength": 0,
            "explaino_rational_escape_denominator_power": 3,
            "poly_kind": 2,
            "poly_coeffs": [
                0.6917520761489868,
                -1.4607863426208496,
                2.08960223197937,
                -1.035009503364563,
                1.0,
            ],
        }
    )

    render = state["render"]
    assert isinstance(render, dict)
    render.update({"width": 160, "height": 100, "aa_mode": "off", "sample_tier": tier})
    return state


def test_rational_escape_published_runtime_uses_distinct_deterministic_numeric_tiers(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    base_state = json.loads(FIXTURE_STATE.read_text(encoding="utf-8"))

    fast_state = _controlled_rational_escape_state(base_state, tier="fast")
    standard_state = _controlled_rational_escape_state(base_state, tier="standard")
    fast_state_path = write_state_bundle(tmp_path / "fast", fast_state)
    standard_state_path = write_state_bundle(tmp_path / "standard", standard_state)

    fast_capture = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(fast_state_path),
        "--capture-diagnostic",
    )
    standard_capture_one = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(standard_state_path),
        "--capture-diagnostic",
    )
    standard_capture_two = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(standard_state_path),
        "--capture-diagnostic",
    )

    assert fast_capture["state"]["render"]["sample_tier"] == "fast"
    assert fast_capture["state"]["stats"]["resolved_backend"] == "float32"
    assert standard_capture_one["state"]["render"]["sample_tier"] == "standard"
    assert standard_capture_one["state"]["stats"]["resolved_backend"] == "float64"
    assert standard_capture_one["frame_hash"] == standard_capture_two["frame_hash"]
    assert fast_capture["frame_hash"] != standard_capture_one["frame_hash"]
