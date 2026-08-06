from __future__ import annotations

import json
import subprocess
from pathlib import Path

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    RUNTIME_DIR,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


REPO_ROOT = Path(__file__).resolve().parents[1]
CHECKED_IN_CONTRACT = REPO_ROOT / "docs" / "ui_salt" / "generated" / "color_pipeline_function_library.contract.v1.json"
STAGED_CONTRACT = RUNTIME_DIR / "ui_salt" / "generated" / "color_pipeline_function_library.contract.v1.json"


def test_published_runtime_consumes_staged_ui_salt_contract(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    report_path = tmp_path / "ui_salt_contract_report.json"

    with runtime_automation_lock():
        result = subprocess.run(
            [
                str(exe_path),
                "--validate-ui-salt-contract",
                "--ui-salt-contract-report-json",
                str(report_path),
            ],
            cwd=str(RUNTIME_DIR),
            text=True,
            capture_output=True,
            check=False,
        )

    assert result.returncode == 0, result.stderr or result.stdout
    assert STAGED_CONTRACT.exists(), f"published runtime did not stage {STAGED_CONTRACT}"
    assert report_path.exists(), "runtime contract validation did not write its report"
    report = json.loads(report_path.read_text(encoding="utf-8"))
    staged = json.loads(STAGED_CONTRACT.read_text(encoding="utf-8"))
    assert STAGED_CONTRACT.read_bytes() == CHECKED_IN_CONTRACT.read_bytes()
    lanes = staged["function_library"]["lanes"]
    function_count = sum(len(lane["functions"]) for lane in lanes)
    compatibility_count = len(staged["composition_recipe_contract"]["compatibility"])
    recipe_count = len(staged["composition_recipe_contract"]["recipes"])
    taxonomy_group_count = len({
        function["taxonomy_group"]
        for lane in lanes
        for function in lane["functions"]
    })
    assert report["ok"] is True
    assert report["contract_path"] == str(STAGED_CONTRACT)
    assert report["schema_version"] == staged["schema_version"]
    assert report["lane_count"] == len(lanes)
    assert report["function_count"] == function_count
    assert report["catalog_authority"] == "materialized_json"
    assert report["active_catalog_function_count"] == function_count
    assert report["compatibility_count"] == compatibility_count
    assert report["compatibility_authority"] == "materialized_json"
    assert report["active_compatibility_count"] == compatibility_count
    assert report["typed_compatibility_resolver_enabled"] is True
    assert report["typed_compatibility_resolver_authority"] == "typed_resolver_live"
    compatibility_audit = staged["composition_recipe_contract"]["compatibility_audit"]
    assert report["typed_compatibility_resolver_route_count"] == sum(
        row["classification"] == "typed_resolved" for row in compatibility_audit
    )
    assert report["specialized_compatibility_route_count"] == sum(
        row["classification"] == "runtime_legacy_override" for row in compatibility_audit
    )
    assert report["typed_compatibility_pilot_enabled"] is True
    assert report["typed_compatibility_pilot_authority"] == "typed_resolver_live"
    assert report["companion_suggestion_authority"] == "materialized_json"
    assert report["active_companion_suggestion_count"] > 0
    assert report["recipe_count"] == recipe_count
    assert report["recipe_expansion_authority"] == "recipe_v2_graph"
    assert report["active_recipe_count"] == recipe_count
    assert report["taxonomy_group_count"] == taxonomy_group_count
    assert report["lane_taxonomy_groups"]["source"] == [
        "escape",
        "phase",
        "bands",
        "basin",
        "sdf",
        "sdf_phase",
        "lens_field_v2",
    ]
    assert "palette_phase" in report["lane_taxonomy_groups"]["palette"]
    assert "grade_manifold" in report["lane_taxonomy_groups"]["grading"]
    assert report["unsupported_pair_count"] > 0
    assert report["errors"] == []


def test_published_runtime_reports_visible_grouped_function_picker_controls(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "mandelbrot",
        "--width",
        "160",
        "--height",
        "120",
    )
    state_path = write_state_bundle(
        tmp_path / "function_picker_visible_seed",
        json.loads(json.dumps(neutral_capture["state"])),
    )

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "function_picker_visible_report.json",
        command_path=tmp_path / "function_picker_visible_command.json",
        open_color_pipeline=True,
    ) as viewer:
        report = viewer.wait_for_report(timeout_seconds=30.0)
        assert "source:smooth_escape_ramp" in report.get("lane_rows", []), report
        rows = report.get("rows")
        assert isinstance(rows, list), report
        row_ids_by_lane = {
            row.get("lane_id"): row.get("ui_row_id")
            for row in rows
            if isinstance(row, dict) and row.get("lane_id") in {"source", "shape", "palette", "grading"}
        }
        assert set(row_ids_by_lane) == {"source", "shape", "palette", "grading"}, report
        for lane_id in ("source", "shape", "palette", "grading"):
            control_id = f"color_pipeline.{lane_id}.{row_ids_by_lane[lane_id]}.function"
            viewer.wait_for_control(control_id, timeout_seconds=20.0)
