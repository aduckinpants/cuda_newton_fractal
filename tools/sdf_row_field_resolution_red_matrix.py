#!/usr/bin/env python3
"""Report the current RED matrix for row-local SDF field resolution.

This is a Step 3A design/proof helper. It intentionally proves the current
checked-in code has only shared Lens SDF field downsample authority plus
row-local postprocess sampling, not row-local SDF field producer authority.
"""

from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any


FIELD_POLICY_TOKENS = (
    "sdf_field_downsample",
    "field_downsample",
    "sdf_field_resolution",
    "field_resolution_policy",
    "sdf_field_group",
)


@dataclass(frozen=True)
class Check:
    check_id: str
    description: str
    expected_gap_present: bool
    evidence: str
    required_next_green: str
    files: tuple[str, ...]

    def to_json(self) -> dict[str, Any]:
        return {
            "id": self.check_id,
            "description": self.description,
            "expected_gap_present": self.expected_gap_present,
            "evidence": self.evidence,
            "required_next_green": self.required_next_green,
            "files": list(self.files),
        }


def read_text(repo: Path, rel_path: str) -> str:
    return (repo / rel_path).read_text(encoding="utf-8")


def has_any_token(text: str, tokens: tuple[str, ...]) -> bool:
    return any(token in text for token in tokens)


def between(text: str, start: str, end: str) -> str:
    start_index = text.find(start)
    if start_index < 0:
        return ""
    end_index = text.find(end, start_index)
    if end_index < 0:
        return text[start_index:]
    return text[start_index:end_index]


def build_report(repo: Path) -> dict[str, Any]:
    fractal_types = read_text(repo, "ui_app/src/fractal_types.h")
    diagnostics_state = read_text(repo, "ui_app/src/diagnostics_state_io.cpp")
    diagnostics_capture = read_text(repo, "ui_app/src/diagnostics_capture.cpp")
    color_window = read_text(repo, "ui_app/src/color_pipeline_window.h")
    main_cpp = read_text(repo, "ui_app/src/main.cpp")
    automation_report = read_text(repo, "ui_app/src/viewer_ui_automation_report.cpp")

    source_params_body = between(
        fractal_types,
        "struct ColorPipelineSourceRuntimeParams",
        "struct ColorPipelineSourceStackEntry",
    )
    parse_entry_body = between(
        diagnostics_state,
        "bool ParseColorSourceStackEntry",
        "bool IsSdfColorSourceStackSignal",
    )
    write_stack_body = between(
        diagnostics_capture,
        "void WriteColorSourceStackJson",
        "int CaptureColorSourceStackCount",
    )
    effective_source_body = between(
        diagnostics_capture,
        "void WriteEffectiveColorSourceJson",
        "void WriteColorPipelineStacksJson",
    )
    field_compute_body = between(
        main_cpp,
        "ResolveEffectiveLensSdfDownsample",
        "if (!postprocessOk)",
    )

    has_row_postprocess_step = "sdf_sample_step" in source_params_body
    has_row_field_policy = has_any_token(source_params_body, FIELD_POLICY_TOKENS)
    state_has_row_field_policy = has_any_token(parse_entry_body, FIELD_POLICY_TOKENS)
    capture_has_row_field_policy = has_any_token(write_stack_body, FIELD_POLICY_TOKENS)
    summary_has_field_groups = has_any_token(effective_source_body, ("field_group", "field_downsample", "effective_downsample"))
    ui_has_shared_alias = "color_pipeline.source.sdf_field.downsample.primary" in color_window
    ui_has_row_field_policy = has_any_token(color_window, FIELD_POLICY_TOKENS)
    report_has_shared_downsample = (
        "lens_sdf_requested_downsample" in automation_report and
        "lens_sdf_effective_downsample" in automation_report
    )
    report_has_field_groups = has_any_token(automation_report, ("field_group", "field_groups", "field_count"))
    main_uses_single_effective_downsample = (
        "lens.downsample" in field_compute_body and
        field_compute_body.count("fieldQuality.effective_downsample") >= 4 and
        "ComputeLensSdfFieldForMaskWithBackend" in field_compute_body
    )
    main_has_multi_field_grouping = has_any_token(field_compute_body, ("field_group", "field_groups", "field_count"))
    mixed_stack_fail_closed = (
        "requires enabled Source rows to be all SDF rows or all non-SDF rows" in color_window
    )

    checks = [
        Check(
            "row_runtime_params_lack_field_policy",
            "Source-row runtime params have row-local postprocess sampling but no row-local field downsample policy.",
            has_row_postprocess_step and not has_row_field_policy,
            "ColorPipelineSourceRuntimeParams contains sdf_sample_step but no sdf_field_downsample/field policy token.",
            "Step 3B/3C must add an explicit row-local field policy separate from sdf_sample_step.",
            ("ui_app/src/fractal_types.h",),
        ),
        Check(
            "state_io_lacks_row_field_policy",
            "State loading parses source-row params but cannot load row-local field downsample authority.",
            "sdf_sample_step" in parse_entry_body and not state_has_row_field_policy,
            "ParseColorSourceStackEntry handles sdf_sample_step and no field-downsample key.",
            "State IO must round-trip old inherit-shared rows and new explicit row field downsample rows.",
            ("ui_app/src/diagnostics_state_io.cpp",),
        ),
        Check(
            "capture_state_lacks_row_field_policy",
            "Capture state writes source-row params but records no row-local field downsample authority.",
            "sdf_sample_step" in write_stack_body and not capture_has_row_field_policy,
            "WriteColorSourceStackJson writes sdf_sample_step and no field-downsample key.",
            "Capture/replay must preserve row-local field authority exactly before Step 3 closes.",
            ("ui_app/src/diagnostics_capture.cpp",),
        ),
        Check(
            "effective_summary_lacks_field_groups",
            "Effective-source summaries report stack rows but no row field groups or per-group timing keys.",
            not summary_has_field_groups,
            "WriteEffectiveColorSourceJson reports source_stack entries only.",
            "Runtime/capture summaries must expose effective field groups once multi-field authority ships.",
            ("ui_app/src/diagnostics_capture.cpp",),
        ),
        Check(
            "ui_has_shared_alias_only",
            "The UI exposes a shared SDF Field Downsample alias, not a row-local field policy.",
            ui_has_shared_alias and not ui_has_row_field_policy,
            "color_pipeline.source.sdf_field.downsample.primary is present and row field policy tokens are absent.",
            "Step 3C must add a row-local control while preserving the shared alias as the default authority.",
            ("ui_app/src/color_pipeline_window.h",),
        ),
        Check(
            "runtime_report_lacks_field_groups",
            "Automation reports shared requested/effective Lens SDF downsample but no row-local field groups.",
            report_has_shared_downsample and not report_has_field_groups,
            "viewer_ui_automation_report writes lens_sdf_requested_downsample/effective_downsample only.",
            "No-mouse report must expose requested shared downsample, row-local groups, field count, and per-group timing.",
            ("ui_app/src/viewer_ui_automation_report.cpp",),
        ),
        Check(
            "main_renders_one_effective_field",
            "The live render path resolves one effective Lens SDF downsample and computes/reuses one field.",
            main_uses_single_effective_downsample and not main_has_multi_field_grouping,
            "main.cpp resolves one fieldQuality.effective_downsample before a single field compute/cache path.",
            "Step 3B must keep this as the fast path when all rows inherit while adding multi-field grouping for distinct row policies.",
            ("ui_app/src/main.cpp",),
        ),
    ]

    red_rows = [check.to_json() for check in checks]
    all_expected_gaps_present = all(check.expected_gap_present for check in checks)
    return {
        "status": "expected_red" if all_expected_gaps_present else "unexpected_surface_drift",
        "summary": {
            "red_count": len(red_rows),
            "all_expected_gaps_present": all_expected_gaps_present,
            "has_shared_lens_downsample_report": report_has_shared_downsample,
            "has_shared_sdf_field_downsample_ui_alias": ui_has_shared_alias,
            "has_row_postprocess_sample_step": has_row_postprocess_step,
            "has_row_field_downsample_policy": has_row_field_policy,
            "mixed_sdf_non_sdf_fail_closed_present": mixed_stack_fail_closed,
            "distinct_live_field_cap_candidate": 4,
        },
        "authority_model": {
            "default_policy": "inherit_shared",
            "explicit_values": [1, 2, 4, 8, 16],
            "grouping_key": "effective_sdf_field_downsample",
            "live_distinct_field_cap": 4,
            "disabled_rows_request_fields": False,
            "non_sdf_rows_request_fields": False,
            "shared_lens_downsample_remains_default": True,
        },
        "red_rows": red_rows,
        "preservation_rows": [
            {
                "id": "mixed_sdf_non_sdf_stack_fail_closed",
                "present": mixed_stack_fail_closed,
                "evidence": "Color Pipeline window still carries the all-SDF-or-all-non-SDF enabled Source row error.",
            },
            {
                "id": "shared_downsample_default_authority",
                "present": ui_has_shared_alias and report_has_shared_downsample,
                "evidence": "Shared UI alias and shared requested/effective report fields are present.",
            },
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo-root", default=".", type=Path)
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()

    repo = args.repo_root.resolve()
    report = build_report(repo)
    report["repo_root"] = str(repo)
    text = json.dumps(report, indent=2) + "\n"
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding="utf-8")
    else:
        print(text, end="")
    return 0 if report["status"] == "expected_red" else 1


if __name__ == "__main__":
    raise SystemExit(main())
