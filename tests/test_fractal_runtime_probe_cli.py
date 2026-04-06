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


def test_probe_cli_supports_stdin_stdout_json() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-stdin-stdout",
        "mode": "point_set",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "explaino_lambda"},
            {"path": "fractal.params.lambda_real", "value": 2.9685855},
            {"path": "fractal.params.lambda_imag", "value": -0.27446103},
            {"path": "fractal.params.explaino_seed", "value": 3.0},
            {"path": "fractal.view.explaino_seed_drift", "value": 0.25},
            {"path": "fractal.params.explaino_warp_strength", "value": 0.2},
        ],
        "points": [
            {"x": 0.48, "y": -0.04},
            {"x": 0.52, "y": 0.04},
        ],
        "metrics": ["iterations", "status", "final_z", "final_abs2", "summary_mean_iterations"],
        "operator_context": {
            "source": "salticid",
            "operator": "parameter_probe_scout",
            "why": "verify stdin/stdout sampling contract",
        },
    }

    result = subprocess.run(
        [
            str(exe_path),
            "--sample-request-stdin",
            "--sample-response-stdout",
        ],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(request),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout

    response = json.loads(result.stdout)
    assert response["ok"] is True
    assert response["request_id"] == request["request_id"]
    assert response["runtime"]["fractal_type"] == "explaino_lambda"
    assert response["summary"]["sample_count"] == 2
    assert len(response["sequence_results"]) == 1
    assert len(response["samples"]) == 2
    assert response["operator_context"]["source"] == "salticid"
    assert response["operator_context"]["operator"] == "parameter_probe_scout"


def test_probe_cli_supports_request_response_files(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request_path = tmp_path / "probe_request.json"
    response_path = tmp_path / "probe_response.json"

    request = {
        "request_version": 1,
        "request_id": "probe-file-io",
        "mode": "sequence_grid",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "explaino_rational_escape"},
            {"path": "fractal.params.explaino_warp_strength", "value": 0.35},
        ],
        "region": {
            "center_x": 0.0,
            "center_y": 0.0,
            "span_x": 0.2,
            "span_y": 0.2,
            "grid_width": 2,
            "grid_height": 2,
        },
        "sequence": {
            "zip_paths": True,
            "vary": [
                {
                    "path": "fractal.params.explaino_seed",
                    "values": [4.0, 5.0],
                },
                {
                    "path": "fractal.view.explaino_seed_drift",
                    "values": [0.1, 0.2],
                },
            ],
        },
        "metrics": ["iterations", "status", "final_z", "final_abs2", "summary_mean_iterations"],
        "operator_context": {
            "source": "salticid",
            "operator": "parameter_probe_scout",
            "why": "verify file-based sampling contract",
        },
    }
    request_path.write_text(json.dumps(request), encoding="utf-8")

    result = subprocess.run(
        [
            str(exe_path),
            "--sample-request-json",
            str(request_path),
            "--sample-response-json",
            str(response_path),
        ],
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout
    assert response_path.exists(), f"missing probe response file: {response_path}"

    response = json.loads(response_path.read_text(encoding="utf-8"))
    assert response["ok"] is True
    assert response["request_id"] == request["request_id"]
    assert response["runtime"]["fractal_type"] == "explaino_rational_escape"
    assert response["summary"]["sample_count"] == 8
    assert response["summary"]["best_sequence_index"] in (0, 1)
    assert len(response["sequence_results"]) == 2
    assert len(response["samples"]) == 8


def test_probe_cli_emits_json_error_payload_for_invalid_request() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    bad_request = {
        "request_version": 1,
        "request_id": "probe-invalid-request",
        "mode": "point_set",
        "points": [{"x": 0.0, "y": 0.0}],
        "mystery_field": 123,
    }

    result = subprocess.run(
        [
            str(exe_path),
            "--sample-request-stdin",
            "--sample-response-stdout",
        ],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(bad_request),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode != 0

    response = json.loads(result.stdout)
    assert response["ok"] is False
    assert response["request_id"] == ""
    assert "mystery_field" in response["error"]