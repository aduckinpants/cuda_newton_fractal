"""End-to-end tests for V2-B session mode (--sample-session)."""
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


def _run_session(lines: list[str], timeout: float = 30.0) -> subprocess.CompletedProcess[str]:
    """Spawn the runtime in --sample-session mode and feed lines via stdin."""
    exe_path = _active_runtime_exe()
    input_text = "\n".join(lines) + "\n"
    return subprocess.run(
        [str(exe_path), "--sample-session"],
        cwd=str(RUNTIME_DIR),
        input=input_text,
        text=True,
        capture_output=True,
        check=False,
        timeout=timeout,
    )


def _parse_output_lines(stdout: str) -> list[dict]:
    """Parse NDJSON output lines into a list of dicts."""
    results = []
    for line in stdout.strip().splitlines():
        line = line.strip()
        if line:
            results.append(json.loads(line))
    return results


class TestSessionOpenClose:
    """Basic session lifecycle: open → ready → close."""

    def test_open_close_returns_zero(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

    def test_ready_response_shape(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
            json.dumps({"session": "close"}),
        ])
        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 2, f"expected 2 output lines, got {len(lines)}"

        ready = lines[0]
        assert ready["session"] == "ready"
        assert "state_token" in ready
        assert ready["engine_version"] == 2

    def test_close_ack(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
            json.dumps({"session": "close"}),
        ])
        lines = _parse_output_lines(result.stdout)
        assert lines[-1]["session"] == "closed"


class TestSessionSampling:
    """Sample requests within a session."""

    @staticmethod
    def _make_newton_request(request_id: str = "r1") -> str:
        return json.dumps({
            "request_version": 1,
            "request_id": request_id,
            "mode": "point_set",
            "overrides": [
                {"path": "fractal.view.fractal_type", "value": "newton"},
            ],
            "points": [{"x": 0.5, "y": 0.3}],
        })

    def test_single_request(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
            self._make_newton_request("r1"),
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 3  # ready + response + close-ack

        resp = lines[1]
        assert resp["ok"] is True
        assert resp["request_id"] == "r1"
        assert "state_token" in resp
        assert resp["response_version"] == 2

    def test_two_requests_have_different_state_tokens(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
            self._make_newton_request("r1"),
            self._make_newton_request("r2"),
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 4  # ready, r1, r2, close

        t0 = lines[0]["state_token"]
        t1 = lines[1]["state_token"]
        t2 = lines[2]["state_token"]
        assert t0 != t1
        assert t1 != t2

    def test_multiple_requests_echo_correct_ids(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
            self._make_newton_request("alpha"),
            self._make_newton_request("beta"),
            self._make_newton_request("gamma"),
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 5  # ready + 3 responses + close
        assert lines[1]["request_id"] == "alpha"
        assert lines[2]["request_id"] == "beta"
        assert lines[3]["request_id"] == "gamma"


class TestSessionErrorHandling:
    """Error cases that should not crash the session."""

    def test_request_without_open_errors(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({
                "request_version": 1, "request_id": "r1", "mode": "point_set",
                "overrides": [{"path": "fractal.view.fractal_type", "value": "newton"}],
                "points": [{"x": 0.5, "y": 0.3}],
            }),
        ])
        assert result.returncode != 0

        lines = _parse_output_lines(result.stdout)
        assert len(lines) >= 1
        assert lines[0]["ok"] is False

    def test_malformed_json_produces_error_response(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
            "not valid json {{{{",
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 3
        assert lines[1]["ok"] is False

    def test_bad_request_does_not_kill_session(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        good_req = json.dumps({
            "request_version": 1, "request_id": "good", "mode": "point_set",
            "overrides": [{"path": "fractal.view.fractal_type", "value": "newton"}],
            "points": [{"x": 0.5, "y": 0.3}],
        })
        bad_req = json.dumps({
            "request_version": 1, "request_id": "bad", "mode": "point_set",
        })
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
            bad_req,
            good_req,
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 4  # ready + bad_error + good_ok + close
        assert lines[1]["ok"] is False
        assert lines[2]["ok"] is True
        assert lines[2]["request_id"] == "good"

    def test_eof_without_close_returns_error(self) -> None:
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        result = _run_session([
            json.dumps({"session": "open", "request_id": "init"}),
        ])
        assert result.returncode != 0

    def test_sample_session_conflicts_with_sample_request(self) -> None:
        """--sample-session + --sample-request-stdin should error at dispatch."""
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        exe_path = _active_runtime_exe()
        result = subprocess.run(
            [str(exe_path), "--sample-session", "--sample-request-stdin"],
            cwd=str(RUNTIME_DIR),
            input="",
            text=True,
            capture_output=True,
            check=False,
            timeout=10.0,
        )
        assert result.returncode != 0
        assert "mutually exclusive" in result.stderr


class TestSessionDiffMode:
    """V2-C: parameter diff mode with state_token + merged overrides."""

    def test_diff_carries_fractal_type_forward(self) -> None:
        """r1 sets fractal_type=mandelbrot; r2 diffs from s1 with only zoom.
        r2 should still run as mandelbrot, not fall back to default newton."""
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        r1 = json.dumps({
            "request_version": 1, "request_id": "r1", "mode": "point_set",
            "overrides": [{"path": "fractal.view.fractal_type", "value": "mandelbrot"}],
            "points": [{"x": -0.5, "y": 0.0}],
        })
        # After open (token s0), r1 response gives token s1.
        r2 = json.dumps({
            "request_version": 1, "request_id": "r2",
            "state_token": "s1",
            "mode": "point_set",
            "overrides": [{"path": "fractal.view.zoom", "value": 2.0}],
            "points": [{"x": -0.5, "y": 0.0}],
        })
        result = _run_session([
            json.dumps({"session": "open"}),
            r1, r2,
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 4  # ready + r1 + r2 + close

        assert lines[1]["ok"] is True
        assert lines[1]["runtime"]["fractal_type"] == "mandelbrot"

        assert lines[2]["ok"] is True
        assert lines[2]["runtime"]["fractal_type"] == "mandelbrot", (
            "diff request should inherit fractal_type from state_token"
        )

    def test_invalid_state_token_errors(self) -> None:
        """Referencing a non-existent state_token should produce an error."""
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        r1 = json.dumps({
            "request_version": 1, "request_id": "r1",
            "state_token": "s999",
            "mode": "point_set",
            "overrides": [{"path": "fractal.view.fractal_type", "value": "newton"}],
            "points": [{"x": 0.5, "y": 0.3}],
        })
        result = _run_session([
            json.dumps({"session": "open"}),
            r1,
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 3  # ready + error + close
        assert lines[1]["ok"] is False
        assert "s999" in lines[1]["error"]

    def test_request_without_token_still_works(self) -> None:
        """Requests without state_token should work as before (V2-B compat)."""
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        r1 = json.dumps({
            "request_version": 1, "request_id": "r1", "mode": "point_set",
            "overrides": [{"path": "fractal.view.fractal_type", "value": "newton"}],
            "points": [{"x": 0.5, "y": 0.3}],
        })
        result = _run_session([
            json.dumps({"session": "open"}),
            r1,
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 3
        assert lines[1]["ok"] is True
        assert "state_token" in lines[1]

    def test_chained_diffs_carry_all_overrides(self) -> None:
        """Chain r1→r2→r3: fractal_type from r1 should survive through r3."""
        if sys.platform != "win32":
            pytest.skip("session mode is Windows-only")
        r1 = json.dumps({
            "request_version": 1, "request_id": "r1", "mode": "point_set",
            "overrides": [{"path": "fractal.view.fractal_type", "value": "mandelbrot"}],
            "points": [{"x": -0.5, "y": 0.0}],
        })
        r2 = json.dumps({
            "request_version": 1, "request_id": "r2",
            "state_token": "s1",
            "mode": "point_set",
            "overrides": [{"path": "fractal.view.zoom", "value": 2.0}],
            "points": [{"x": -0.5, "y": 0.0}],
        })
        r3 = json.dumps({
            "request_version": 1, "request_id": "r3",
            "state_token": "s2",
            "mode": "point_set",
            "overrides": [{"path": "fractal.params.max_iter", "value": 100}],
            "points": [{"x": -0.5, "y": 0.0}],
        })
        result = _run_session([
            json.dumps({"session": "open"}),
            r1, r2, r3,
            json.dumps({"session": "close"}),
        ])
        assert result.returncode == 0, result.stderr

        lines = _parse_output_lines(result.stdout)
        assert len(lines) == 5  # ready + 3 responses + close

        for i in range(1, 4):
            assert lines[i]["ok"] is True
            assert lines[i]["runtime"]["fractal_type"] == "mandelbrot", (
                f"response {i} should have fractal_type=mandelbrot"
            )

        # All tokens should be distinct
        tokens = [lines[i]["state_token"] for i in range(4)]  # ready + 3 responses
        assert len(set(tokens)) == len(tokens)
