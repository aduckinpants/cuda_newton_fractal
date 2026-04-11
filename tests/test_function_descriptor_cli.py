"""Runtime regression tests for --describe-functions and function_id support."""
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


def test_describe_functions_emits_valid_catalog() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    exe = _active_runtime_exe()

    result = subprocess.run(
        [str(exe), "--describe-functions"],
        capture_output=True,
        text=True,
        timeout=10,
    )
    assert result.returncode == 0, f"stderr: {result.stderr}"

    catalog = json.loads(result.stdout)
    assert catalog["engine_version"] == 1
    assert isinstance(catalog["functions"], list)
    assert len(catalog["functions"]) >= 1

    func = catalog["functions"][0]
    assert func["id"] == "fractal.sample"
    assert func["name"] == "Fractal Point Sampler"
    assert isinstance(func["parameters"], list)
    assert len(func["parameters"]) > 0
    assert isinstance(func["outputs"], list)
    assert len(func["outputs"]) == 7
    assert isinstance(func["summary_metrics"], list)
    assert len(func["summary_metrics"]) == 6
    assert "best_sequence_index" in func["summary_metrics"]

    # fractal_type param should be required with enum options
    ft_param = next(p for p in func["parameters"] if p["path"] == "fractal.view.fractal_type")
    assert ft_param["type"] == "enum"
    assert ft_param.get("required") is True
    option_ids = [option["id"] for option in ft_param["options"]]
    assert set(option_ids) == {
        "newton",
        "nova",
        "mandelbrot",
        "julia",
        "burning_ship",
        "multibrot",
        "phoenix",
        "explaino",
        "explaino_y",
        "explaino_fp",
        "explaino_nova",
        "explaino_halley",
        "explaino_dual",
        "explaino_fold",
        "explaino_joy",
        "explaino_bell",
        "explaino_mult",
        "explaino_phoenix",
        "explaino_transcendental",
        "explaino_inertial",
        "explaino_julia",
        "explaino_rational",
        "explaino_ripple",
        "explaino_splice",
        "explaino_vortex",
        "explaino_tension",
        "multicorn",
        "halley",
        "collatz",
        "explaino_collatz",
        "mcmullen",
        "lambda",
        "explaino_lambda",
        "explaino_rational_escape",
        "spider",
        "celtic_mandelbrot",
        "perpendicular_burning_ship",
    }

    # epsilon param should have applicable_when
    eps_param = next(p for p in func["parameters"] if p["path"] == "fractal.params.epsilon")
    assert eps_param["type"] == "float"
    assert "applicable_when" in eps_param
    assert eps_param["applicable_when"]["op"] == "in"

    ripple_param = next(p for p in func["parameters"] if p["path"] == "fractal.params.ripple_amplitude")
    assert ripple_param["type"] == "float"
    assert ripple_param["cost_hint"] == pytest.approx(2.55)
    assert ripple_param["sensitivity"]["zero_case_id"] == "explaino_ripple_zero"
    assert ripple_param["sensitivity"]["default_case_id"] == "explaino_ripple_default"
    assert len(ripple_param["sensitivity"]["points"]) == 5
    assert ripple_param["sensitivity"]["points"][0]["param_value"] == pytest.approx(0.0)
    assert ripple_param["sensitivity"]["points"][-1]["param_value"] == pytest.approx(0.15)

    # status output should be enum with option list
    status_out = next(o for o in func["outputs"] if o["name"] == "status")
    assert status_out["type"] == "enum"
    assert len(status_out["options"]) == 6

    # residual output should be nullable
    residual_out = next(o for o in func["outputs"] if o["name"] == "residual")
    assert residual_out.get("nullable") is True


def test_describe_functions_json_file_output() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    exe = _active_runtime_exe()

    import tempfile, os
    fd, tmppath = tempfile.mkstemp(suffix=".json")
    os.close(fd)
    try:
        result = subprocess.run(
            [str(exe), "--describe-functions-json", tmppath],
            capture_output=True,
            text=True,
            timeout=10,
        )
        assert result.returncode == 0, f"stderr: {result.stderr}"
        with open(tmppath, "r", encoding="utf-8") as f:
            catalog = json.load(f)
        assert catalog["engine_version"] == 1
        assert catalog["functions"][0]["id"] == "fractal.sample"
    finally:
        if os.path.exists(tmppath):
            os.remove(tmppath)


def test_probe_with_explicit_function_id() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    exe = _active_runtime_exe()

    import tempfile, os

    request = {
        "request_version": 1,
        "request_id": "fid-explicit-test",
        "function_id": "fractal.sample",
        "mode": "point_set",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "newton"},
        ],
        "points": [{"x": 1.0, "y": 0.0}],
        "metrics": ["iterations", "status"],
    }

    fd_req, req_path = tempfile.mkstemp(suffix=".json")
    os.close(fd_req)
    fd_resp, resp_path = tempfile.mkstemp(suffix=".json")
    os.close(fd_resp)
    try:
        with open(req_path, "w", encoding="utf-8") as f:
            json.dump(request, f)

        result = subprocess.run(
            [str(exe), "--sample-request-json", req_path, "--sample-response-json", resp_path],
            capture_output=True,
            text=True,
            timeout=10,
        )
        assert result.returncode == 0, f"stderr: {result.stderr}"

        with open(resp_path, "r", encoding="utf-8") as f:
            response = json.load(f)

        assert response["ok"] is True
        assert response["function_id"] == "fractal.sample"
        assert response["summary"]["sample_count"] == 1
    finally:
        for p in (req_path, resp_path):
            if os.path.exists(p):
                os.remove(p)


def test_probe_with_unknown_function_id_fails() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    exe = _active_runtime_exe()

    import tempfile, os

    request = {
        "request_version": 1,
        "request_id": "bad-fid-test",
        "function_id": "nonexistent.kernel",
        "mode": "point_set",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "newton"},
        ],
        "points": [{"x": 0.0, "y": 0.0}],
    }

    fd_req, req_path = tempfile.mkstemp(suffix=".json")
    os.close(fd_req)
    fd_resp, resp_path = tempfile.mkstemp(suffix=".json")
    os.close(fd_resp)
    try:
        with open(req_path, "w", encoding="utf-8") as f:
            json.dump(request, f)

        result = subprocess.run(
            [str(exe), "--sample-request-json", req_path, "--sample-response-json", resp_path],
            capture_output=True,
            text=True,
            timeout=10,
        )
        # Should fail
        assert result.returncode != 0

        # Should emit error JSON
        with open(resp_path, "r", encoding="utf-8") as f:
            response = json.load(f)
        assert response["ok"] is False
        assert "nonexistent.kernel" in response.get("error", "")
    finally:
        for p in (req_path, resp_path):
            if os.path.exists(p):
                os.remove(p)
