from __future__ import annotations

import sys

import pytest

from tests.runtime_harness import active_runtime_exe as _active_runtime_exe, run_headless_capture as _run_headless_capture

def test_dual_seed_cli_overrides_survive_headless_capture() -> None:
    if sys.platform != "win32":
        pytest.skip("Explaino-DualSeed runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    capture = _run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_dual",
        "--explaino-seed",
        "2.25",
        "--explaino-seed-b",
        "7.75",
        "--explaino-mix",
        "0.35",
    )

    state = capture["state"]
    assert state["fractal_type"] == "explaino_dual"
    assert state["params"]["explaino_seed"] == 2
    assert state["view"]["explaino_seed_drift"] == pytest.approx(0.25, abs=1e-6)
    assert state["params"]["explaino_seed_b"] == pytest.approx(7.75, abs=1e-6)
    assert state["params"]["explaino_mix"] == pytest.approx(0.35, abs=1e-6)