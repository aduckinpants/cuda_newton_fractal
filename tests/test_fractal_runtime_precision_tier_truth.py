from __future__ import annotations

import json
from pathlib import Path

import pytest

from tests.runtime_harness import (
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


REPO_ROOT = Path(__file__).resolve().parents[1]
FIXTURE_STATE = REPO_ROOT / "tests" / "fixtures" / "rational_escape_numeric_truth_v1" / "state.json"
REPAIRED_SELECTORS = (
    "explaino_y",
    "explaino_julia",
    "explaino_lambda",
    "multicorn",
    "mcmullen",
    "collatz",
)


def _controlled_state(base_state: dict[str, object], *, selector: str, tier: str) -> dict[str, object]:
    state = json.loads(json.dumps(base_state))
    state["fractal_type"] = selector

    view = state["view"]
    assert isinstance(view, dict)
    view.update(
        {
            "center_x": 0.0,
            "center_y": 0.0,
            "center_hp_x": 0.0,
            "center_hp_y": 0.0,
            "zoom": 1.0,
            "log2_zoom": 0.0,
            "auto_max_iter": False,
        }
    )

    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "max_iter": 48,
            "explaino_warp_strength": 0,
            "explaino_damping": 1,
            "mcmullen_m": 3,
            "mcmullen_n": 3,
            "mcmullen_lambda": -0.125,
            "collatz_transition_strength": 1,
        }
    )

    render = state["render"]
    assert isinstance(render, dict)
    render.update({"width": 64, "height": 40, "aa_mode": "off", "sample_tier": tier})
    return state


@pytest.mark.parametrize("selector", REPAIRED_SELECTORS)
def test_published_runtime_resolves_repaired_selectors_and_replays_standard_deterministically(
    tmp_path: Path,
    selector: str,
) -> None:
    exe_path = active_runtime_exe()
    base_state = json.loads(FIXTURE_STATE.read_text(encoding="utf-8"))
    fast_path = write_state_bundle(
        tmp_path / selector / "fast",
        _controlled_state(base_state, selector=selector, tier="fast"),
    )
    standard_path = write_state_bundle(
        tmp_path / selector / "standard",
        _controlled_state(base_state, selector=selector, tier="standard"),
    )

    fast = run_headless_capture(str(exe_path), "--load-state-json", str(fast_path), "--capture-diagnostic")
    standard_one = run_headless_capture(
        str(exe_path), "--load-state-json", str(standard_path), "--capture-diagnostic"
    )
    standard_two = run_headless_capture(
        str(exe_path), "--load-state-json", str(standard_path), "--capture-diagnostic"
    )

    assert fast["state"]["fractal_type"] == selector
    assert fast["state"]["render"]["sample_tier"] == "fast"
    assert fast["state"]["stats"]["resolved_backend"] == "float32"
    assert standard_one["state"]["fractal_type"] == selector
    assert standard_one["state"]["render"]["sample_tier"] == "standard"
    assert standard_one["state"]["stats"]["resolved_backend"] == "float64"
    assert standard_one["frame_hash"] == standard_two["frame_hash"]
