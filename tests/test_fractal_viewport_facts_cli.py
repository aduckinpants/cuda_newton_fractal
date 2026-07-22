"""Published-runtime tests for deterministic engine-owned viewport geometry."""
from __future__ import annotations

import json
import math
from pathlib import Path
import subprocess
import sys

import pytest

from tests.runtime_harness import active_runtime_exe


REPO_ROOT = Path(__file__).resolve().parents[1]
STATE_FIXTURE = REPO_ROOT / "docs" / "examples" / "magnet_state_pack" / "default_smooth" / "state.json"


def _run_stdout() -> subprocess.CompletedProcess[bytes]:
    return subprocess.run(
        [
            str(active_runtime_exe()),
            "--describe-viewport-facts",
            "--load-state-json",
            str(STATE_FIXTURE),
        ],
        capture_output=True,
        timeout=15,
    )


def test_viewport_facts_stdout_is_deterministic_and_renderer_grounded() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    first = _run_stdout()
    second = _run_stdout()
    assert first.returncode == 0, first.stderr.decode(errors="replace")
    assert second.returncode == 0, second.stderr.decode(errors="replace")
    assert first.stdout == second.stdout
    assert first.stderr == b""
    assert first.stdout.endswith(b"\n") and b"\r\n" not in first.stdout

    facts = json.loads(first.stdout.decode("utf-8"))
    state = json.loads(STATE_FIXTURE.read_text(encoding="utf-8"))
    assert facts["schema_version"] == 1
    assert facts["mapping_id"] == "cuda_fractal_renderer_pixel_center_v1"
    assert facts["selected_fractal_type"] == state["fractal_type"]
    assert facts["render"]["width"] == state["render"]["width"]
    assert facts["render"]["height"] == state["render"]["height"]
    assert facts["camera"]["center_hp_x"] == pytest.approx(state["view"]["center_hp_x"])
    assert facts["camera"]["center_hp_y"] == pytest.approx(state["view"]["center_hp_y"])
    assert facts["camera"]["resolved_zoom"] == pytest.approx(math.exp2(state["view"]["log2_zoom"]))
    assert facts["local_frame"]["full_height"] == pytest.approx(4.0 / facts["camera"]["resolved_zoom"])
    assert len(facts["continuous_edge_corners"]) == 4
    assert len(facts["pixel_center_corners"]) == 4
    assert "fit_log2_zoom" in facts["fit_model"]["inverse_fit"]


def test_viewport_facts_file_bytes_match_stdout(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    stdout_result = _run_stdout()
    assert stdout_result.returncode == 0
    target = tmp_path / "viewport.json"
    target.write_bytes(b"old")
    result = subprocess.run(
        [
            str(active_runtime_exe()),
            "--describe-viewport-facts-json",
            str(target),
            "--load-state-json",
            str(STATE_FIXTURE),
        ],
        capture_output=True,
        timeout=15,
    )
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    assert result.stdout == b""
    assert result.stderr == b""
    assert target.read_bytes() == stdout_result.stdout
    assert not target.with_name(target.name + ".tmp").exists()


def test_viewport_facts_requires_valid_exact_loaded_state(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    exe = active_runtime_exe()
    missing = subprocess.run([str(exe), "--describe-viewport-facts"], capture_output=True, timeout=15)
    assert missing.returncode != 0

    malformed_state = tmp_path / "state.json"
    malformed_state.write_text("{not json", encoding="utf-8")
    malformed = subprocess.run(
        [str(exe), "--describe-viewport-facts", "--load-state-json", str(malformed_state)],
        capture_output=True,
        timeout=15,
    )
    assert malformed.returncode != 0
    assert malformed.stdout == b""

    conflict = subprocess.run(
        [
            str(exe),
            "--describe-viewport-facts",
            "--load-state-json",
            str(STATE_FIXTURE),
            "--width",
            "320",
        ],
        capture_output=True,
        timeout=15,
    )
    assert conflict.returncode != 0


def test_viewport_facts_file_failure_is_clear_and_cleans_owned_temp(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    target = tmp_path / "missing" / "viewport.json"
    result = subprocess.run(
        [
            str(active_runtime_exe()),
            "--describe-viewport-facts-json",
            str(target),
            "--load-state-json",
            str(STATE_FIXTURE),
        ],
        capture_output=True,
        timeout=15,
    )
    assert result.returncode != 0
    assert result.stdout == b""
    assert result.stderr
    assert not target.with_name(target.name + ".tmp").exists()
