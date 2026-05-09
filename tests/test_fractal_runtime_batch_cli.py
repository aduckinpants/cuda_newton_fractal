"""V2-A: Batch request array — runtime CLI regression tests.

Validates that the published fractal_ui runtime accepts a JSON array of
requests and returns a JSON array of responses, one per request, in order.
"""
from __future__ import annotations

import json
import subprocess
import sys

import pytest

from tests.runtime_harness import RUNTIME_DIR, active_runtime_exe as _active_runtime_exe

def _run_batch(requests: list[dict]) -> tuple[int, list[dict]]:
    """Send a batch request array to the runtime and parse the response array."""
    exe_path = _active_runtime_exe()
    result = subprocess.run(
        [str(exe_path), "--sample-request-stdin", "--sample-response-stdout"],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(requests),
        text=True,
        capture_output=True,
        check=False,
    )
    responses = json.loads(result.stdout)
    assert isinstance(responses, list), f"batch response must be an array, got {type(responses).__name__}"
    return result.returncode, responses


def test_batch_two_valid_requests() -> None:
    if sys.platform != "win32":
        pytest.skip("runtime regression is Windows-only")

    requests = [
        {
            "request_version": 1,
            "request_id": "batch-newton",
            "mode": "point_set",
            "overrides": [
                {"path": "fractal.view.fractal_type", "value": "newton"},
            ],
            "points": [{"x": 0.5, "y": 0.3}],
            "metrics": ["iterations", "status", "summary_mean_iterations"],
        },
        {
            "request_version": 1,
            "request_id": "batch-mandelbrot",
            "mode": "point_set",
            "overrides": [
                {"path": "fractal.view.fractal_type", "value": "mandelbrot"},
            ],
            "points": [{"x": -0.5, "y": 0.0}, {"x": -1.0, "y": 0.0}],
            "metrics": ["iterations", "status", "summary_mean_iterations"],
        },
    ]
    rc, responses = _run_batch(requests)
    assert rc == 0, f"batch of valid requests should exit 0, got {rc}"
    assert len(responses) == 2

    assert responses[0]["ok"] is True
    assert responses[0]["request_id"] == "batch-newton"
    assert responses[0]["summary"]["sample_count"] == 1

    assert responses[1]["ok"] is True
    assert responses[1]["request_id"] == "batch-mandelbrot"
    assert responses[1]["summary"]["sample_count"] == 2


def test_batch_error_isolation() -> None:
    if sys.platform != "win32":
        pytest.skip("runtime regression is Windows-only")

    requests = [
        {
            "request_version": 1,
            "request_id": "good-req",
            "mode": "point_set",
            "overrides": [
                {"path": "fractal.view.fractal_type", "value": "newton"},
            ],
            "points": [{"x": 0.5, "y": 0.3}],
            "metrics": ["iterations", "status"],
        },
        {
            "request_version": 1,
            "request_id": "bad-req",
            "mode": "point_set",
            "points": [{"x": 0.0, "y": 0.0}],
            "mystery_field": 123,
        },
    ]
    rc, responses = _run_batch(requests)
    assert rc == 1, "batch with a bad request should exit non-zero"
    assert len(responses) == 2

    # First request should succeed
    assert responses[0]["ok"] is True
    assert responses[0]["request_id"] == "good-req"

    # Second request should fail with error mentioning the unknown field
    assert responses[1]["ok"] is False
    assert responses[1]["request_id"] == "bad-req"
    assert "mystery_field" in responses[1]["error"]


def test_batch_empty_array() -> None:
    if sys.platform != "win32":
        pytest.skip("runtime regression is Windows-only")

    rc, responses = _run_batch([])
    assert rc == 0, "empty batch should exit 0"
    assert responses == []


def test_single_object_not_wrapped_in_array() -> None:
    """V1 backward compat: single object input -> single object output."""
    if sys.platform != "win32":
        pytest.skip("runtime regression is Windows-only")

    exe_path = _active_runtime_exe()
    request = {
        "request_version": 1,
        "request_id": "v1-compat",
        "mode": "point_set",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": "newton"},
        ],
        "points": [{"x": 0.5, "y": 0.3}],
        "metrics": ["iterations", "status"],
    }
    result = subprocess.run(
        [str(exe_path), "--sample-request-stdin", "--sample-response-stdout"],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(request),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0
    response = json.loads(result.stdout)
    # Must be a dict (object), not a list (array)
    assert isinstance(response, dict), f"V1 single-object response should be dict, got {type(response).__name__}"
    assert response["ok"] is True
    assert response["request_id"] == "v1-compat"
