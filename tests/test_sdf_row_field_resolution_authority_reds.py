from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


EXPECTED_RED_IDS = {
    "row_runtime_params_lack_field_policy",
    "state_io_lacks_row_field_policy",
    "capture_state_lacks_row_field_policy",
    "effective_summary_lacks_field_groups",
    "ui_has_shared_alias_only",
    "runtime_report_lacks_field_groups",
    "main_renders_one_effective_field",
}


def _run_matrix(tmp_path: Path) -> dict[str, object]:
    out_path = tmp_path / "red_matrix.json"
    subprocess.run(
        [
            sys.executable,
            str(REPO_ROOT / "tools" / "sdf_row_field_resolution_red_matrix.py"),
            "--repo-root",
            str(REPO_ROOT),
            "--out",
            str(out_path),
        ],
        check=True,
    )
    return json.loads(out_path.read_text(encoding="utf-8"))


def test_sdf_row_field_resolution_red_matrix_records_current_gaps(tmp_path: Path) -> None:
    payload = _run_matrix(tmp_path)
    summary = payload["summary"]
    assert payload["status"] == "expected_red"
    assert summary["all_expected_gaps_present"] is True
    assert summary["has_shared_sdf_field_downsample_ui_alias"] is True
    assert summary["has_row_postprocess_sample_step"] is True
    assert summary["has_row_field_downsample_policy"] is False
    assert {row["id"] for row in payload["red_rows"]} == EXPECTED_RED_IDS


def test_sdf_row_field_resolution_red_matrix_preserves_existing_boundaries(tmp_path: Path) -> None:
    payload = _run_matrix(tmp_path)
    preservation = {row["id"]: row for row in payload["preservation_rows"]}
    assert preservation["mixed_sdf_non_sdf_stack_fail_closed"]["present"] is True
    assert preservation["shared_downsample_default_authority"]["present"] is True
    model = payload["authority_model"]
    assert model["default_policy"] == "inherit_shared"
    assert model["explicit_values"] == [1, 2, 4, 8, 16]
    assert model["live_distinct_field_cap"] == 4
