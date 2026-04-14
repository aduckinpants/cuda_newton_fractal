"""
test_callable_engine_adversarial_cli.py — Runtime adversarial tests for the callable engine surface.
Proves: empty function_id rejection, unknown function_id rejection, and
type coverage sync through the published runtime CLI.
"""
import json
import os
import subprocess
import pytest

RUNTIME_EXE = os.path.join(
    os.environ.get("SALT_FRACTAL_ROOT", r"D:\salt-fractal"),
    "cuda_newton_fractal_clone", "runtime", "fractal_ui.exe",
)
RUNTIME_DIR = os.path.dirname(RUNTIME_EXE)


def _run_sample(request_json: dict) -> tuple[bool, dict | None, str]:
    """Submit a probe via --sample-request-stdin and return (ok, parsed_response, stderr)."""
    proc = subprocess.run(
        [RUNTIME_EXE, "--sample-request-stdin", "--sample-response-stdout"],
        cwd=RUNTIME_DIR,
        input=json.dumps(request_json),
        capture_output=True, text=True, timeout=30,
    )
    stderr = proc.stderr.strip()
    # Runtime emits JSON error payloads to stdout even on non-zero exit.
    try:
        resp = json.loads(proc.stdout)
        return resp.get("ok", False), resp, stderr
    except (json.JSONDecodeError, ValueError):
        return False, None, stderr


def _describe_functions() -> dict:
    proc = subprocess.run(
        [RUNTIME_EXE, "--describe-functions"],
        capture_output=True, text=True, timeout=30,
    )
    assert proc.returncode == 0, f"describe-functions failed: {proc.stderr}"
    return json.loads(proc.stdout)


class TestEmptyFunctionId:
    """Empty function_id must be rejected, not silently routed to fractal.sample."""

    def test_empty_function_id_rejected(self):
        req = {
            "request_version": 1,
            "request_id": "adversarial-empty-fid",
            "function_id": "",
            "mode": "point_set",
            "points": [{"x": 0.5, "y": 0.5}],
        }
        ok, resp, stderr = _run_sample(req)
        assert not ok, "empty function_id must not succeed silently"
        # Error should mention function_id
        error_text = ""
        if resp and "error" in resp:
            error_text = resp["error"]
        elif stderr:
            error_text = stderr
        assert "function_id" in error_text.lower() or "function_id" in error_text, (
            f"error should mention function_id: {error_text}"
        )


class TestUnknownFunctionId:
    """Unknown function_id must fail fast with valid-id listing."""

    def test_unknown_function_id_rejected(self):
        req = {
            "request_version": 1,
            "request_id": "adversarial-unknown-fid",
            "function_id": "evil.compute",
            "mode": "point_set",
            "points": [{"x": 0.0, "y": 0.0}],
        }
        ok, resp, stderr = _run_sample(req)
        assert not ok, "unknown function_id must fail"
        error_text = resp.get("error", "") if resp else stderr
        assert "fractal.sample" in error_text, f"error should list valid ids: {error_text}"
        assert "generic.sample" in error_text, f"error should list valid ids: {error_text}"


class TestDescriptorTypeCoverage:
    """Every fractal_type advertised in describe-functions must be sampleable."""

    def test_all_advertised_types_sampleable(self):
        catalog = _describe_functions()
        functions = catalog.get("functions", [])
        fractal_desc = next((f for f in functions if f["id"] == "fractal.sample"), None)
        assert fractal_desc is not None, "fractal.sample not in catalog"

        ft_param = next(
            (p for p in fractal_desc["parameters"] if p["path"] == "fractal.view.fractal_type"),
            None,
        )
        assert ft_param is not None, "fractal.view.fractal_type not in descriptor"

        options = ft_param.get("options", [])
        assert len(options) >= 20, f"expected >= 20 advertised types, got {len(options)}"

        failures = []
        for opt in options:
            ftype = opt["id"]
            req = {
                "request_version": 1,
                "request_id": f"adversarial-type-{ftype}",
                "function_id": "fractal.sample",
                "mode": "point_set",
                "points": [{"x": 0.5, "y": 0.5}],
                "overrides": [{"path": "fractal.view.fractal_type", "value": ftype}],
            }
            ok, resp, stderr = _run_sample(req)
            error_text = resp.get("error", "") if resp else stderr
            if not ok and "not yet implemented" in error_text:
                failures.append(ftype)

        assert not failures, (
            f"advertised but not implemented: {failures}"
        )
