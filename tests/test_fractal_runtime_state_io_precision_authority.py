from __future__ import annotations

import json
from pathlib import Path
import struct

from tests.runtime_harness import active_runtime_exe, run_headless_capture, write_state_bundle


REPO_ROOT = Path(__file__).resolve().parents[1]
FIXTURE_STATE = REPO_ROOT / "tests" / "fixtures" / "rational_escape_numeric_truth_v1" / "state.json"


def _binary32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def test_published_runtime_preserves_direct_double_state_and_reports_float_normalization(tmp_path: Path) -> None:
    state = json.loads(FIXTURE_STATE.read_text(encoding="utf-8"))
    state["fractal_type"] = "explaino_rational_escape"

    expected_double = {
        "center_hp_x": -0.30367326545671292,
        "center_hp_y": 0.20949006969551054,
        "log2_zoom": 12.345678901234567,
        "explaino_seed": -3.0,
        "explaino_seed_b": 1.2345678901234567,
        "explaino_secondary_root_pattern_seed": -2.345678901234567,
    }
    view = state["view"]
    assert isinstance(view, dict)
    view.update(
        {
            "center_x": _binary32(expected_double["center_hp_x"]),
            "center_y": _binary32(expected_double["center_hp_y"]),
            "zoom": _binary32(2.0 ** expected_double["log2_zoom"]),
            "center_hp_x": expected_double["center_hp_x"],
            "center_hp_y": expected_double["center_hp_y"],
            "log2_zoom": expected_double["log2_zoom"],
            "auto_max_iter": False,
        }
    )
    requested_float = 0.7300000000000001
    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "max_iter": 240,
            "explaino_seed": expected_double["explaino_seed"],
            "explaino_seed_b": expected_double["explaino_seed_b"],
            "explaino_secondary_root_pattern_seed": expected_double["explaino_secondary_root_pattern_seed"],
            "explaino_damping": requested_float,
            "explaino_warp_strength": 0,
        }
    )
    render = state["render"]
    assert isinstance(render, dict)
    render.update({"width": 96, "height": 60, "aa_mode": "off", "sample_tier": "standard"})

    state_path = write_state_bundle(tmp_path / "requested", state)
    materialized = run_headless_capture(
        str(active_runtime_exe()),
        "--load-state-json",
        str(state_path),
        "--capture-diagnostic",
    )
    emitted = materialized["state"]
    assert isinstance(emitted, dict)
    emitted_view = emitted["view"]
    emitted_params = emitted["params"]
    assert isinstance(emitted_view, dict)
    assert isinstance(emitted_params, dict)

    for key in ("center_hp_x", "center_hp_y", "log2_zoom"):
        assert emitted_view[key] == expected_double[key]
    for key in ("explaino_seed", "explaino_seed_b", "explaino_secondary_root_pattern_seed"):
        assert emitted_params[key] == expected_double[key]
    assert emitted_params["explaino_damping"] == _binary32(requested_float)
    assert emitted_params["explaino_damping"] != requested_float

    replay_path = write_state_bundle(tmp_path / "replay", emitted)
    replay = run_headless_capture(
        str(active_runtime_exe()),
        "--load-state-json",
        str(replay_path),
        "--capture-diagnostic",
    )
    replay_state = replay["state"]
    assert isinstance(replay_state, dict)
    assert replay_state["view"] == emitted_view
    assert replay_state["params"]["explaino_seed"] == expected_double["explaino_seed"]
    assert replay_state["params"]["explaino_seed_b"] == expected_double["explaino_seed_b"]
    assert replay_state["params"]["explaino_secondary_root_pattern_seed"] == expected_double[
        "explaino_secondary_root_pattern_seed"
    ]
    assert replay_state["params"]["explaino_damping"] == _binary32(requested_float)
    assert replay["frame_hash"] == materialized["frame_hash"]
