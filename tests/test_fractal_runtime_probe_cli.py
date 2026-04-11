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
        "metrics": ["iterations", "status", "final_z", "final_abs2", "summary_mean_iterations", "summary_best_sequence_index"],
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
    assert response["cost"]["sample_count"] == 2
    assert response["cost"]["gpu_ms"] >= 0.0
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
        "metrics": [
            "iterations",
            "status",
            "final_z",
            "final_abs2",
            "summary_mean_iterations",
            "summary_best_sequence_index",
        ],
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


def test_probe_cli_supports_variant_crossfade_sequence_mode() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-variant-crossfade",
        "mode": "sequence_grid",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "explaino"},
            {"path": "fractal.params.explaino_seed", "value": 3.0},
            {"path": "fractal.params.explaino_warp_strength", "value": 0.25},
            {"path": "fractal.view.explaino_seed_drift", "value": 0.1},
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
            "mode": "variant_crossfade",
            "from_variant": "explaino_ripple",
            "to_variant": "explaino_splice",
            "steps": 5,
        },
        "metrics": ["iterations", "status", "summary_mean_iterations", "summary_best_sequence_index"],
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
    assert response["runtime"]["fractal_type"] == "explaino_splice"
    assert response["summary"]["sample_count"] == 20
    assert len(response["sequence_results"]) == 5
    assert len(response["samples"]) == 20

    assert response["sequence_results"][0]["applied"] == {
        "fractal.view.fractal_type": "explaino_ripple",
        "fractal.params.ripple_amplitude": pytest.approx(0.15),
    }
    assert response["sequence_results"][1]["applied"] == {
        "fractal.view.fractal_type": "explaino_ripple",
        "fractal.params.ripple_amplitude": pytest.approx(0.075),
    }
    assert response["sequence_results"][2]["applied"] == {
        "fractal.view.fractal_type": "explaino",
    }
    assert response["sequence_results"][3]["applied"] == {
        "fractal.view.fractal_type": "explaino_splice",
        "fractal.params.splice_offset": pytest.approx(0.25),
    }
    assert response["sequence_results"][4]["applied"] == {
        "fractal.view.fractal_type": "explaino_splice",
        "fractal.params.splice_offset": pytest.approx(0.5),
    }


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
    assert response["cost"]["sample_count"] == 0
    assert response["cost"]["gpu_ms"] == 0.0
    assert "mystery_field" in response["error"]


def test_probe_cli_omits_samples_for_summary_only_metrics() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-summary-only-metrics",
        "mode": "point_set",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "mandelbrot"},
        ],
        "points": [
            {"x": -0.75, "y": 0.0},
            {"x": 0.25, "y": 0.0},
        ],
        "metrics": ["summary_mean_iterations", "summary_escape_fraction"],
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
    assert response["summary"]["sample_count"] == 2
    assert "mean_iterations" in response["summary"]
    assert "escape_fraction" in response["summary"]
    assert "converged_fraction" not in response["summary"]
    assert "best_sequence_index" not in response["summary"]
    assert response["samples"] == []
    assert len(response["sequence_results"]) == 1
    assert "mean_iterations" in response["sequence_results"][0]["summary"]
    assert "pole_fraction" not in response["sequence_results"][0]["summary"]


def test_probe_cli_omits_unrequested_sample_metrics() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-sample-metric-subset",
        "mode": "point_set",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "mandelbrot"},
        ],
        "points": [
            {"x": 0.0, "y": 0.0},
        ],
        "metrics": ["iterations", "status", "summary_mean_iterations"],
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
    assert len(response["samples"]) == 1
    sample = response["samples"][0]
    assert "coord_x" in sample and "coord_y" in sample
    assert "iterations" in sample and "status" in sample
    assert "final_abs2" not in sample
    assert "final_z_x" not in sample and "final_z_y" not in sample
    assert "residual" not in sample and "root_index" not in sample


def test_probe_cli_supports_ndjson_output_mode() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-ndjson-grid",
        "mode": "grid",
        "output_mode": "ndjson",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "mandelbrot"},
        ],
        "region": {
            "center_x": -0.75,
            "center_y": 0.0,
            "span_x": 0.5,
            "span_y": 0.5,
            "grid_width": 2,
            "grid_height": 2,
        },
        "metrics": ["iterations", "status", "summary_mean_iterations"],
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

    lines = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    assert len(lines) == 3
    assert lines[0]["type"] == "sample_batch"
    assert lines[1]["type"] == "sample_batch"
    assert lines[2]["type"] == "summary"
    assert lines[2]["request_id"] == request["request_id"]
    assert lines[2]["cost"]["sample_count"] == 4


def test_probe_cli_point_set_ndjson_single_batch_and_metric_filtering() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-ndjson-point-set",
        "mode": "point_set",
        "output_mode": "ndjson",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "newton"},
        ],
        "points": [
            {"x": 0.5, "y": 0.3},
            {"x": -0.5, "y": 0.0},
        ],
        "metrics": ["iterations", "status", "summary_mean_iterations"],
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

    lines = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    assert len(lines) == 2
    assert lines[0]["type"] == "sample_batch"
    assert lines[0]["request_id"] == request["request_id"]
    assert lines[0]["function_id"] == "fractal.sample"
    assert "row_index" not in lines[0]
    assert len(lines[0]["samples"]) == 2
    sample = lines[0]["samples"][0]
    assert "iterations" in sample and "status" in sample
    assert "final_abs2" not in sample
    assert "final_z_x" not in sample and "final_z_y" not in sample
    assert "residual" not in sample and "root_index" not in sample
    assert lines[1]["type"] == "summary"


def test_probe_cli_ndjson_summary_only_emits_summary_line() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-ndjson-summary-only",
        "mode": "point_set",
        "output_mode": "ndjson",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "mandelbrot"},
        ],
        "points": [
            {"x": -0.75, "y": 0.0},
            {"x": 0.25, "y": 0.0},
        ],
        "metrics": ["summary_mean_iterations", "summary_escape_fraction"],
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

    lines = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    assert len(lines) == 1
    assert lines[0]["type"] == "summary"
    assert lines[0]["summary"]["sample_count"] == 2
    assert lines[0]["cost"]["sample_count"] == 2


def test_probe_cli_generic_sample_ndjson_includes_function_id() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-ndjson-generic",
        "function_id": "generic.sample",
        "mode": "point_set",
        "output_mode": "ndjson",
        "function": {
            "expression": "z^2 + z + 1",
        },
        "points": [
            {"x": 1.0, "y": 0.0},
        ],
        "metrics": ["value", "abs2"],
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

    lines = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    assert len(lines) == 2
    assert lines[0]["type"] == "sample_batch"
    assert lines[0]["function_id"] == "generic.sample"
    assert lines[1]["type"] == "summary"
    assert lines[1]["function_id"] == "generic.sample"


def test_probe_cli_sequence_point_set_ndjson_batches_per_sequence_step() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-ndjson-sequence-point-set",
        "mode": "sequence_point_set",
        "output_mode": "ndjson",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "newton"},
        ],
        "points": [
            {"x": 0.5, "y": 0.3},
            {"x": -0.5, "y": 0.0},
        ],
        "sequence": {
            "zip_paths": True,
            "vary": [
                {"path": "fractal.view.zoom", "values": [1.0, 2.0]},
            ],
        },
        "metrics": ["iterations", "status", "summary_mean_iterations"],
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

    lines = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    assert len(lines) == 3
    assert lines[0]["type"] == "sample_batch"
    assert lines[1]["type"] == "sample_batch"
    assert lines[0]["sequence_index"] == 0
    assert lines[1]["sequence_index"] == 1
    assert "row_index" not in lines[0]
    assert "row_index" not in lines[1]
    assert len(lines[0]["samples"]) == 2
    assert len(lines[1]["samples"]) == 2
    assert lines[2]["type"] == "summary"


def test_probe_cli_batch_rejects_ndjson_output_mode(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request_path = tmp_path / "ndjson_batch_request.json"
    response_path = tmp_path / "ndjson_batch_response.json"
    batch = [
        {
            "request_version": 1,
            "request_id": "ndjson-bad",
            "mode": "point_set",
            "output_mode": "ndjson",
            "overrides": [{"path": "fractal.view.fractal_type", "value": "newton"}],
            "points": [{"x": 0.5, "y": 0.3}],
        }
    ]
    request_path.write_text(json.dumps(batch), encoding="utf-8")

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
    assert result.returncode != 0
    response = json.loads(response_path.read_text(encoding="utf-8"))
    assert isinstance(response, list)
    assert response[0]["ok"] is False
    assert "ndjson" in response[0]["error"]


def test_probe_cli_sequence_grid_ndjson_batches_per_sequence_step() -> None:
    if sys.platform != "win32":
        pytest.skip("probe CLI runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "probe-ndjson-sequence-grid",
        "mode": "sequence_grid",
        "output_mode": "ndjson",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "mandelbrot"},
        ],
        "region": {
            "center_x": -0.75,
            "center_y": 0.0,
            "span_x": 0.5,
            "span_y": 0.5,
            "grid_width": 2,
            "grid_height": 2,
        },
        "sequence": {
            "zip_paths": True,
            "vary": [
                {"path": "fractal.view.zoom", "values": [1.0, 2.0]},
            ],
        },
        "metrics": ["iterations", "status", "summary_mean_iterations"],
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

    lines = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    assert len(lines) == 3
    assert lines[0]["type"] == "sample_batch"
    assert lines[1]["type"] == "sample_batch"
    assert lines[0]["sequence_index"] == 0
    assert lines[1]["sequence_index"] == 1
    assert "row_index" not in lines[0]
    assert "row_index" not in lines[1]
    assert len(lines[0]["samples"]) == 4
    assert len(lines[1]["samples"]) == 4
    assert lines[2]["type"] == "summary"
    assert lines[2]["cost"]["sample_count"] == 8