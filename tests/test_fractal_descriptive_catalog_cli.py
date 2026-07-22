"""Published-runtime contract tests for the deterministic fractal descriptive catalog."""
from __future__ import annotations

import hashlib
import json
import subprocess
import sys

import pytest

from tests.runtime_harness import active_runtime_exe


def _run_stdout() -> subprocess.CompletedProcess[bytes]:
    return subprocess.run(
        [str(active_runtime_exe()), "--describe-fractal-catalog"],
        capture_output=True,
        timeout=15,
    )


def test_descriptive_catalog_stdout_is_deterministic_and_complete() -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    first = _run_stdout()
    second = _run_stdout()
    assert first.returncode == 0, first.stderr.decode(errors="replace")
    assert second.returncode == 0, second.stderr.decode(errors="replace")
    assert first.stdout == second.stdout
    assert first.stderr == b""

    catalog = json.loads(first.stdout.decode("utf-8"))
    assert catalog["schema_version"] == 1
    assert len(catalog["entries"]) == 51
    selectors = [entry["selector_id"] for entry in catalog["entries"]]
    assert len(selectors) == len(set(selectors))
    assert "lambda" in selectors
    assert "lambda_map" not in selectors

    reviewed = [entry for entry in catalog["entries"] if entry["description_status"] == "reviewed"]
    assert [entry["selector_id"] for entry in reviewed] == [
        "newton",
        "nova",
        "mandelbrot",
        "julia",
        "burning_ship",
        "multibrot",
        "phoenix",
        "explaino",
        "explaino_all",
        "explaino_y",
        "explaino_fp",
        "explaino_nova",
        "explaino_halley",
        "explaino_dual",
        "explaino_mult",
        "explaino_phoenix",
        "explaino_transcendental",
        "explaino_inertial",
        "explaino_julia",
        "explaino_rational",
        "multicorn",
        "halley",
        "collatz",
        "explaino_collatz",
        "explaino_collatz_direct",
        "mcmullen",
        "lambda",
        "explaino_lambda",
        "explaino_rational_escape",
        "spider",
        "celtic_mandelbrot",
        "perpendicular_burning_ship",
        "explaino_joy",
        "explaino_fold",
        "explaino_bell",
        "explaino_ripple",
        "explaino_splice",
        "explaino_vortex",
        "explaino_tension",
        "explaino_balance_void",
        "counterfactual_pair",
        "explaino_counterfactual_pair",
        "projection_and_flow",
        "explaino_projection_and_flow",
        "magnet",
        "explaino_magnet_root_well",
    ]
    for entry in catalog["entries"]:
        if entry["description_status"] == "unavailable":
            assert entry["description"] is None
        else:
            assert entry["description_status"] == "reviewed"
            description = entry["description"]
            for field in (
                "math_summary",
                "recurrence_or_field_model",
                "state_order",
                "termination_or_classification",
                "interpretation_notes",
            ):
                assert isinstance(description[field], str) and description[field]
            assert description["source_refs"]

    forbidden_keys = {"generated_at", "timestamp", "branch", "commit", "commit_timestamp", "build_machine", "local_path"}
    for entry in catalog["entries"]:
        assert forbidden_keys.isdisjoint(entry)


def test_descriptive_catalog_file_bytes_match_stdout(tmp_path) -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    stdout_result = _run_stdout()
    assert stdout_result.returncode == 0
    target = tmp_path / "catalog.json"
    target.write_bytes(b"old")
    result = subprocess.run(
        [str(active_runtime_exe()), "--describe-fractal-catalog-json", str(target)],
        capture_output=True,
        timeout=15,
    )
    assert result.returncode == 0, result.stderr.decode(errors="replace")
    assert result.stdout == b""
    assert result.stderr == b""
    assert target.read_bytes() == stdout_result.stdout
    assert hashlib.sha256(target.read_bytes()).hexdigest() == hashlib.sha256(stdout_result.stdout).hexdigest()
    assert not target.with_name(target.name + ".tmp").exists()


def test_descriptive_catalog_file_failures_are_clear_and_clean(tmp_path) -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    missing_target = tmp_path / "missing" / "catalog.json"
    result = subprocess.run(
        [str(active_runtime_exe()), "--describe-fractal-catalog-json", str(missing_target)],
        capture_output=True,
        timeout=15,
    )
    assert result.returncode != 0
    assert result.stdout == b""
    assert result.stderr
    assert not missing_target.with_name(missing_target.name + ".tmp").exists()


def test_descriptive_catalog_modes_reject_missing_or_conflicting_arguments(tmp_path) -> None:
    if sys.platform != "win32":
        pytest.skip("Windows-only")
    exe = active_runtime_exe()
    missing = subprocess.run([str(exe), "--describe-fractal-catalog-json"], capture_output=True, timeout=15)
    assert missing.returncode != 0
    conflict = subprocess.run(
        [str(exe), "--describe-fractal-catalog", "--describe-functions"],
        capture_output=True,
        timeout=15,
    )
    assert conflict.returncode != 0
