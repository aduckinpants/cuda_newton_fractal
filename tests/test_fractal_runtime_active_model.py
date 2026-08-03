from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import RUNTIME_DIR, active_runtime_exe


REPO_ROOT = Path(__file__).resolve().parents[1]
FIXTURE_STATE = REPO_ROOT / "tests" / "fixtures" / "rational_escape_numeric_truth_v1" / "state.json"


def _describe(state_path: Path, *extra: str) -> subprocess.CompletedProcess[bytes]:
    return subprocess.run(
        [
            str(active_runtime_exe()),
            "--load-state-json",
            str(state_path),
            "--describe-active-fractal-model",
            *extra,
        ],
        cwd=str(RUNTIME_DIR),
        capture_output=True,
        timeout=30,
    )


def test_active_model_receipt_is_deterministic_exactly_bound_and_file_equivalent(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("active-model runtime qualification is Windows-only")

    first = _describe(FIXTURE_STATE)
    second = _describe(FIXTURE_STATE)
    assert first.returncode == 0, first.stderr.decode(errors="replace")
    assert second.returncode == 0, second.stderr.decode(errors="replace")
    assert first.stdout == second.stdout
    assert first.stderr == second.stderr == b""

    receipt = json.loads(first.stdout)
    exe = active_runtime_exe()
    assert receipt["state_binding"] == {
        "state_json_sha256": hashlib.sha256(FIXTURE_STATE.read_bytes()).hexdigest(),
        "runtime_executable_sha256": hashlib.sha256(exe.read_bytes()).hexdigest(),
    }
    assert receipt["selected_fractal_type"] == "explaino_rational_escape"
    assert receipt["resolved_runtime_fractal_type"] == "explaino_rational_escape"
    assert receipt["provider"] == {
        "status": "available",
        "provider_id": "polynomial_over_power_escape.v1",
        "provider_version": 1,
        "unavailable_reason": None,
    }
    assert receipt["numeric_authority"]["resolved_backend"] == "float64"
    assert receipt["evaluation_authority"]["evaluation_surface"] == "fractal.sample"
    assert receipt["model"]["denominator_power"] == 3
    assert "epsilon" not in json.dumps(receipt)

    target = tmp_path / "active-model.json"
    target.write_bytes(b"old")
    file_result = subprocess.run(
        [
            str(exe),
            "--load-state-json",
            str(FIXTURE_STATE),
            "--describe-active-fractal-model-json",
            str(target),
        ],
        cwd=str(RUNTIME_DIR),
        capture_output=True,
        timeout=30,
    )
    assert file_result.returncode == 0, file_result.stderr.decode(errors="replace")
    assert file_result.stdout == file_result.stderr == b""
    assert target.read_bytes() == first.stdout
    assert not target.with_name(target.name + ".tmp").exists()


def test_active_model_receipt_and_canonical_sample_share_state_and_runtime_authority() -> None:
    if sys.platform != "win32":
        pytest.skip("active-model runtime qualification is Windows-only")

    described = _describe(FIXTURE_STATE)
    assert described.returncode == 0, described.stderr.decode(errors="replace")
    receipt = json.loads(described.stdout)
    request = {
        "request_version": 1,
        "request_id": "active-model-binding-e2",
        "function_id": "fractal.sample",
        "mode": "point_set",
        "base_state": {"load_state_json": str(FIXTURE_STATE)},
        "points": [{"x": 0.217977, "y": 0.0}],
        "metrics": ["iterations", "status", "final_z", "final_abs2"],
    }
    sampled = subprocess.run(
        [str(active_runtime_exe()), "--sample-request-stdin", "--sample-response-stdout"],
        cwd=str(RUNTIME_DIR),
        input=json.dumps(request),
        text=True,
        capture_output=True,
        timeout=30,
    )
    assert sampled.returncode == 0, sampled.stderr or sampled.stdout
    response = json.loads(sampled.stdout)
    assert response["ok"] is True
    assert response["runtime"]["backend_used"] == "cuda"
    assert response["runtime"]["fractal_type"] == receipt["resolved_runtime_fractal_type"]
    assert response["runtime"]["iteration_arithmetic"] == receipt["numeric_authority"]["resolved_backend"]
    assert receipt["state_binding"]["state_json_sha256"] == hashlib.sha256(FIXTURE_STATE.read_bytes()).hexdigest()
    assert receipt["state_binding"]["runtime_executable_sha256"] == hashlib.sha256(active_runtime_exe().read_bytes()).hexdigest()


def test_active_model_unavailable_states_and_cli_failures_are_explicit(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("active-model runtime qualification is Windows-only")

    state = json.loads(FIXTURE_STATE.read_text(encoding="utf-8"))
    state["params"]["explaino_warp_strength"] = 0.25
    warped = tmp_path / "warped.json"
    warped.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    result = _describe(warped)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    receipt = json.loads(result.stdout)
    assert receipt["provider"]["status"] == "unavailable"
    assert receipt["provider"]["unavailable_reason"] == "nonzero_warp_unsupported"
    assert receipt["model"] is None

    state["fractal_type"] = "mandelbrot"
    state["params"]["explaino_warp_strength"] = 0.0
    unsupported = tmp_path / "unsupported.json"
    unsupported.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    result = _describe(unsupported)
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    receipt = json.loads(result.stdout)
    assert receipt["provider"]["status"] == "unavailable"
    assert receipt["provider"]["provider_id"] is None
    assert receipt["provider"]["unavailable_reason"] == "unsupported_fractal_type"

    missing_state = subprocess.run(
        [str(active_runtime_exe()), "--describe-active-fractal-model"],
        cwd=str(RUNTIME_DIR),
        capture_output=True,
        timeout=15,
    )
    assert missing_state.returncode != 0

    missing_parent = tmp_path / "missing" / "receipt.json"
    file_failure = subprocess.run(
        [
            str(active_runtime_exe()),
            "--load-state-json",
            str(FIXTURE_STATE),
            "--describe-active-fractal-model-json",
            str(missing_parent),
        ],
        cwd=str(RUNTIME_DIR),
        capture_output=True,
        timeout=30,
    )
    assert file_failure.returncode != 0
    assert file_failure.stderr
    assert not missing_parent.with_name(missing_parent.name + ".tmp").exists()
