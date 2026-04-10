"""Tests for generic.sample probe CLI contract via the published runtime."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest


RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"


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


def _run_probe(request: dict) -> dict:
    """Send a probe request via stdin/stdout and return the parsed response."""
    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [str(exe_path), "--sample-request-stdin", "--sample-response-stdout"],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(request),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    return json.loads(result.stdout)


# -- Happy path: Newton z^3-1 iterate via generic.sample --


def test_generic_sample_newton_z3m1_point_set() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    request = {
        "request_version": 1,
        "request_id": "generic-newton-z3m1",
        "function_id": "generic.sample",
        "mode": "point_set",
        "function": {
            "expression": "iterate(z - (z^3 - 1) / (3 * z^2), 200)",
            "epsilon": 1e-10,
            "escape_radius": 1000.0,
        },
        "points": [
            {"x": 1.1, "y": 0.05},
            {"x": -0.4, "y": 0.9},
        ],
        "metrics": ["iterations", "status", "value", "abs2", "derivative"],
    }

    response = _run_probe(request)
    assert response["ok"] is True
    assert response["request_id"] == "generic-newton-z3m1"
    assert response["function_id"] == "generic.sample"
    assert response["summary"]["sample_count"] == 2
    assert len(response["samples"]) == 2

    # First sample should converge near root z=1.
    s0 = response["samples"][0]
    assert s0["status"] == "converged"
    assert abs(s0["value_x"] - 1.0) < 1e-3
    assert abs(s0["value_y"]) < 1e-3

    # Second sample should converge near z=(-0.5, sqrt(3)/2).
    s1 = response["samples"][1]
    assert s1["status"] == "converged"
    assert abs(s1["value_x"] - (-0.5)) < 1e-3
    assert abs(s1["value_y"] - 0.866025) < 1e-2


# -- Happy path: grid sampling --


def test_generic_sample_grid_mandelbrot_like() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    request = {
        "request_version": 1,
        "request_id": "generic-grid-mandel",
        "function_id": "generic.sample",
        "mode": "grid",
        "function": {
            "expression": "iterate(z^2 + c, 50)",
            "params": {"c_real": -0.75, "c_imag": 0.0},
            "epsilon": 1e-8,
            "escape_radius": 4.0,
        },
        "region": {
            "center_x": 0.0,
            "center_y": 0.0,
            "span_x": 2.0,
            "span_y": 2.0,
            "grid_width": 4,
            "grid_height": 4,
        },
        "metrics": ["iterations", "status", "summary_mean_iterations"],
    }

    response = _run_probe(request)
    assert response["ok"] is True
    assert response["summary"]["sample_count"] == 16
    assert len(response["samples"]) == 16

    statuses = [s["status"] for s in response["samples"]]
    assert "escaped" in statuses, "expected some escaped points in 4x4 grid"


# -- Happy path: direct evaluation (non-iterate) --


def test_generic_sample_direct_eval() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    request = {
        "request_version": 1,
        "request_id": "generic-direct-eval",
        "function_id": "generic.sample",
        "mode": "point_set",
        "function": {
            "expression": "z^2 + z + 1",
        },
        "points": [
            {"x": 1.0, "y": 0.0},
        ],
        "metrics": ["value", "abs2"],
    }

    response = _run_probe(request)
    assert response["ok"] is True
    s = response["samples"][0]
    # f(1) = 1 + 1 + 1 = 3
    assert abs(s["value_x"] - 3.0) < 1e-9
    assert abs(s["value_y"]) < 1e-9


# -- Error cases --


def test_generic_sample_malformed_expression() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    request = {
        "request_version": 1,
        "request_id": "generic-bad-expr",
        "function_id": "generic.sample",
        "mode": "point_set",
        "function": {
            "expression": "z^2 +* 1",
        },
        "points": [{"x": 0.0, "y": 0.0}],
    }

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [str(exe_path), "--sample-request-stdin", "--sample-response-stdout"],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(request),
        text=True,
        capture_output=True,
        check=False,
    )
    # The runtime should still return exit 0 with an error JSON payload,
    # or a non-zero exit with a diagnostic. Either way it should not crash.
    if result.returncode == 0:
        response = json.loads(result.stdout)
        assert response["ok"] is False or "error" in response
    # Non-zero exit is also acceptable — what matters is no crash / no hang.


def test_generic_sample_unknown_function_id() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    request = {
        "request_version": 1,
        "request_id": "generic-bad-id",
        "function_id": "bogus.thing",
        "mode": "point_set",
        "points": [{"x": 0.0, "y": 0.0}],
    }

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [str(exe_path), "--sample-request-stdin", "--sample-response-stdout"],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(request),
        text=True,
        capture_output=True,
        check=False,
    )
    # Should fail with a descriptive error mentioning valid IDs.
    if result.returncode == 0:
        response = json.loads(result.stdout)
        assert response["ok"] is False


# -- Describe-functions: generic.sample should appear in catalog --


def test_describe_functions_includes_generic_sample() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [str(exe_path), "--describe-functions"],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    catalog = json.loads(result.stdout)
    function_ids = [f["id"] for f in catalog["functions"]]
    assert "fractal.sample" in function_ids, "fractal.sample should still be in catalog"
    assert "generic.sample" in function_ids, "generic.sample should appear in catalog"

    # Spot-check generic.sample descriptor.
    gf_desc = next(f for f in catalog["functions"] if f["id"] == "generic.sample")
    output_names = [o["name"] for o in gf_desc.get("outputs", [])]
    assert "value_x" in output_names
    assert "abs2" in output_names
