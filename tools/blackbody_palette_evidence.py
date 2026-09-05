#!/usr/bin/env python3
"""Generate bounded Black Body palette evidence from shipped authority seams."""

from __future__ import annotations

import argparse
import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

from PIL import Image

REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tests.runtime_harness import (  # noqa: E402
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)

GENERATED_CONTRACT = REPO_ROOT / "docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json"
LUT_INC = REPO_ROOT / "docs/reference/blackbody_chromaticity/blackbody_palette_lut_v1.inc"
LUT_METADATA = REPO_ROOT / "docs/reference/blackbody_chromaticity/blackbody_palette_lut_v1.metadata.json"

PUBLIC_RECIPE_CASES = (
    ("default_smooth_escape", "mandelbrot"),
    ("phase_orbit_wheel", "mandelbrot"),
    ("root_phase_wheel", "explaino_magnet_root_well"),
    ("root_proximity_heatmap", "explaino_magnet_root_well"),
    ("sdf_normal_angle_diagnostic", "mandelbrot"),
    ("sdf_normal_angle_beauty", "mandelbrot"),
    ("root_glow", "explaino_magnet_root_well"),
    ("curvature_relief", "mandelbrot"),
    ("lens_topography", "mandelbrot"),
)


def _sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _sha256_file(path: Path) -> str:
    return _sha256_bytes(path.read_bytes())


def _canonical_sha256(value: Any) -> str:
    return _sha256_bytes(json.dumps(value, sort_keys=True, separators=(",", ":")).encode("utf-8"))


def _canonical_runtime_receipt(value: Any) -> Any:
    """Normalize float32 serialization noise while preserving receipt structure."""
    if isinstance(value, dict):
        return {key: _canonical_runtime_receipt(item) for key, item in value.items()}
    if isinstance(value, list):
        return [_canonical_runtime_receipt(item) for item in value]
    if isinstance(value, float):
        return round(value, 9)
    return value


def _recipe_metadata() -> tuple[list[dict[str, Any]], dict[str, dict[str, Any]]]:
    payload = json.loads(GENERATED_CONTRACT.read_text(encoding="utf-8"))
    recipes = payload["composition_recipe_contract"]["recipe_v2"]
    assert isinstance(recipes, list)
    by_id = {recipe["id"]: recipe for recipe in recipes}
    return recipes, by_id


def _headless_actions(receipt: dict[str, Any]) -> list[str]:
    nodes = receipt.get("nodes")
    assert isinstance(nodes, list), receipt
    actions: list[str] = []
    for lane_id in ("source", "shape", "palette", "grading"):
        lane_nodes = sorted(
            (
                node
                for node in nodes
                if isinstance(node, dict)
                and node.get("lane_id") == lane_id
                and node.get("enabled") is True
            ),
            key=lambda node: int(node["row_index"]),
        )
        assert lane_nodes, (lane_id, receipt)
        for row_index, node in enumerate(lane_nodes):
            function_id = node["function_id"]
            actions.append(
                f"select_function:{lane_id}:0:{function_id}"
                if row_index == 0
                else f"add_row:{lane_id}:{function_id}"
            )
            for param in node["params"]:
                param_type = param["type"]
                if param_type == "enum":
                    kind, value = "enum", param["enum_value"]
                elif param_type == "bool":
                    kind, value = "bool", "true" if param["bool_value"] else "false"
                else:
                    kind, value = "number", param["number_value"]
                actions.append(
                    f"set_param:{lane_id}:{row_index}:{param['path']}:{kind}:{value}"
                )
    return actions


def _capture_with_actions(exe_path: Path, state_path: Path, actions: list[str]) -> dict[str, Any]:
    args = [str(exe_path), "--load-state-json", str(state_path)]
    for action in actions:
        args.extend(["--color-pipeline-action", action])
    args.append("--capture-diagnostic")
    return run_headless_capture(*args)


def _application_projection(report: dict[str, Any]) -> dict[str, Any]:
    application = report.get("color_pipeline_recipe_application_report")
    assert isinstance(application, dict), report
    receipt = application.get("receipt")
    assert isinstance(receipt, dict), application
    return {
        "current_recipe_match": application.get("current_recipe_match"),
        "authoritative_pipeline_rows_fingerprint": application.get(
            "authoritative_pipeline_rows_fingerprint"
        ),
        "recipe_id": receipt.get("recipe_id"),
        "recipe_version": receipt.get("recipe_version"),
        "metadata_content_hash": receipt.get("metadata_content_hash"),
        "application_authority": receipt.get("application_authority"),
        "fallback_active": receipt.get("fallback_active"),
        "semantic_node_ids": receipt.get("semantic_node_ids"),
        "normalized_parameter_overrides": receipt.get("normalized_parameter_overrides"),
        "approved_adapters": receipt.get("approved_adapters"),
        "committed_rows": receipt.get("committed_rows"),
        "committed_row_fingerprint": receipt.get("committed_row_fingerprint"),
        "committed_live_row_fingerprint": receipt.get("committed_live_row_fingerprint"),
    }


def freeze_preservation(out_path: Path, runtime_exe: Path | None) -> None:
    out_path = out_path.resolve()
    exe_path = runtime_exe or active_runtime_exe()
    recipes, by_id = _recipe_metadata()
    expected_ids = [case[0] for case in PUBLIC_RECIPE_CASES]
    actual_ids = [recipe["id"] for recipe in recipes]
    if actual_ids != expected_ids:
        raise RuntimeError(f"pre-recipe inventory mismatch: expected {expected_ids}, got {actual_ids}")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    states_dir = out_path.parent / "states"
    states_dir.mkdir(parents=True, exist_ok=True)
    cases: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="blackbody_recipe_freeze_") as temp:
        temp_root = Path(temp)
        for recipe_id, fractal_type in PUBLIC_RECIPE_CASES:
            neutral = run_headless_capture(
                str(exe_path),
                "--capture-diagnostic",
                "--fractal-type",
                fractal_type,
                "--width",
                "256",
                "--height",
                "192",
            )
            seed_state = write_state_bundle(
                temp_root / f"{recipe_id}_seed", json.loads(json.dumps(neutral["state"]))
            )
            with PersistentRuntimeViewerAutomation(
                exe_path=exe_path,
                state_path=seed_state,
                report_path=temp_root / f"{recipe_id}_report.json",
                command_path=temp_root / f"{recipe_id}_command.json",
                open_color_pipeline=True,
            ) as viewer:
                ready = viewer.wait_for_report(timeout_seconds=60.0)
                applicability = next(
                    item
                    for item in ready["color_pipeline_recipe_capability_report"]["recipe_applicability"]
                    if item["recipe_id"] == recipe_id
                )
                if applicability.get("available") is not True:
                    raise RuntimeError(f"{recipe_id} unavailable during freeze: {applicability}")
                selected = viewer.click_control(
                    f"color_pipeline.recipe.{recipe_id}.select", timeout_seconds=60.0
                )
                assert selected.get("click_consumed") is True, selected
                applied = viewer.click_control(
                    "color_pipeline.recipe.apply_selected", timeout_seconds=60.0
                )
                assert applied.get("click_consumed") is True, applied
                settled = viewer.click_control("render_once", timeout_seconds=60.0)
                assert settled.get("click_consumed") is True, settled

            graph_receipt = settled.get("color_pipeline_graph_receipt")
            assert isinstance(graph_receipt, dict), settled
            capture = _capture_with_actions(exe_path, seed_state, _headless_actions(graph_receipt))
            state_file = states_dir / f"{recipe_id}.state.json"
            state_file.write_text(json.dumps(capture["state"], indent=2) + "\n", encoding="utf-8")
            replay = run_headless_capture(
                str(exe_path), "--load-state-json", str(state_file), "--capture-diagnostic"
            )
            assert replay["frame_hash"] == capture["frame_hash"], (capture, replay)
            cases.append(
                {
                    "recipe_id": recipe_id,
                    "fractal_type": fractal_type,
                    "metadata": by_id[recipe_id],
                    "metadata_canonical_sha256": _canonical_sha256(by_id[recipe_id]),
                    "public_frame_sha256": settled["rendered_frame_hash"],
                    "public_frame_width": settled.get("rendered_frame_width"),
                    "public_frame_height": settled.get("rendered_frame_height"),
                    "lane_rows": settled.get("lane_rows"),
                    "graph_receipt": graph_receipt,
                    "application_projection": _application_projection(settled),
                    "headless_frame_sha256": capture["frame_hash"],
                    "replay_frame_sha256": replay["frame_hash"],
                    "state_file": state_file.relative_to(REPO_ROOT).as_posix(),
                    "state_sha256": _sha256_file(state_file),
                }
            )

    artifact = {
        "schema_id": "viewer.blackbody_recipe_preservation.v1",
        "classification": "baseline_frozen",
        "runtime_exe": str(exe_path),
        "runtime_exe_sha256": _sha256_file(exe_path),
        "generated_contract_sha256": _sha256_file(GENERATED_CONTRACT),
        "ordered_recipe_ids": actual_ids,
        "baseline_recipe_count": len(actual_ids),
        "comparison_domain": "exact metadata objects, public settled frame SHA-256, and 256x192 BMP byte SHA-256",
        "cases": cases,
    }
    out_path.write_text(json.dumps(artifact, indent=2) + "\n", encoding="utf-8")


def verify_preservation(out_path: Path, runtime_exe: Path | None) -> None:
    out_path = out_path.resolve()
    exe_path = runtime_exe or active_runtime_exe()
    artifact = json.loads(out_path.read_text(encoding="utf-8"))
    recipes, by_id = _recipe_metadata()
    baseline_ids = artifact["ordered_recipe_ids"]
    actual_ids = [recipe["id"] for recipe in recipes]
    if actual_ids != baseline_ids + ["blackbody_thermal_ridges"]:
        raise RuntimeError(f"post-recipe inventory mismatch: {actual_ids}")

    verification_cases: list[dict[str, Any]] = []
    with tempfile.TemporaryDirectory(prefix="blackbody_recipe_verify_") as temp:
        temp_root = Path(temp)
        for baseline in artifact["cases"]:
            recipe_id = baseline["recipe_id"]
            if by_id[recipe_id] != baseline["metadata"]:
                raise RuntimeError(f"materialized metadata drift for {recipe_id}")
            state_file = REPO_ROOT / baseline["state_file"]
            with PersistentRuntimeViewerAutomation(
                exe_path=exe_path,
                state_path=state_file,
                report_path=temp_root / f"{recipe_id}_report.json",
                command_path=temp_root / f"{recipe_id}_command.json",
                open_color_pipeline=True,
            ) as viewer:
                viewer.wait_for_report(timeout_seconds=60.0)
                selected = viewer.click_control(
                    f"color_pipeline.recipe.{recipe_id}.select", timeout_seconds=60.0
                )
                assert selected.get("click_consumed") is True, selected
                applied = viewer.click_control(
                    "color_pipeline.recipe.apply_selected", timeout_seconds=60.0
                )
                assert applied.get("click_consumed") is True, applied
                settled = viewer.click_control("render_once", timeout_seconds=60.0)
            if settled["rendered_frame_hash"] != baseline["public_frame_sha256"]:
                raise RuntimeError(f"public frame drift for {recipe_id}")
            if settled.get("lane_rows") != baseline["lane_rows"]:
                raise RuntimeError(f"committed row drift for {recipe_id}")
            current_graph_receipt = settled.get("color_pipeline_graph_receipt")
            if _canonical_runtime_receipt(current_graph_receipt) != _canonical_runtime_receipt(
                baseline["graph_receipt"]
            ):
                raise RuntimeError(f"graph receipt drift for {recipe_id}")
            if _application_projection(settled) != baseline["application_projection"]:
                raise RuntimeError(f"application receipt drift for {recipe_id}")
            replay = run_headless_capture(
                str(exe_path), "--load-state-json", str(state_file), "--capture-diagnostic"
            )
            if replay["frame_hash"] != baseline["headless_frame_sha256"]:
                raise RuntimeError(f"headless replay drift for {recipe_id}")
            verification_cases.append(
                {
                    "recipe_id": recipe_id,
                    "metadata_exact": True,
                    "public_frame_exact": True,
                    "graph_receipt_exact": True,
                    "application_projection_exact": True,
                    "headless_frame_exact": True,
                }
            )

    artifact["classification"] = "passed"
    artifact["verification"] = {
        "runtime_exe": str(exe_path),
        "runtime_exe_sha256": _sha256_file(exe_path),
        "generated_contract_sha256": _sha256_file(GENERATED_CONTRACT),
        "ordered_recipe_ids": actual_ids,
        "added_recipe_ids": ["blackbody_thermal_ridges"],
        "cases": verification_cases,
    }
    out_path.write_text(json.dumps(artifact, indent=2) + "\n", encoding="utf-8")


def render_strips(sampler_exe: Path, out_json: Path, out_dir: Path) -> None:
    if not sampler_exe.is_file():
        raise FileNotFoundError(f"host sampler executable not found: {sampler_exe}")
    out_json = out_json.resolve()
    out_dir = out_dir.resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    out_json.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="blackbody_host_samples_") as temp:
        raw_path = Path(temp) / "samples.json"
        subprocess.run(
            [str(sampler_exe), "--emit-host-samples", str(raw_path)],
            cwd=REPO_ROOT,
            check=True,
        )
        payload = json.loads(raw_path.read_text(encoding="utf-8"))

    inc_hash = _sha256_file(LUT_INC)
    if payload.get("compiled_lut_inc_sha256") != inc_hash:
        raise RuntimeError(
            f"compiled LUT identity mismatch: {payload.get('compiled_lut_inc_sha256')} != {inc_hash}"
        )
    metadata = json.loads(LUT_METADATA.read_text(encoding="utf-8"))
    if payload.get("lut_entry_count") != metadata.get("entry_count"):
        raise RuntimeError("compiled LUT entry count differs from checked metadata")

    strips_by_id = {strip["id"]: strip for strip in payload["strips"]}
    forward = strips_by_id["forward_reciprocal"]["samples"]
    reverse = strips_by_id["reverse_reciprocal"]["samples"]
    reversal_error = max(
        abs(a - b)
        for left, right in zip(forward, reversed(reverse), strict=True)
        for a, b in zip(left, right, strict=True)
    )
    constant = strips_by_id["constant_1600"]["samples"]
    constant_error = max(
        abs(value - constant[0][channel])
        for value_triplet in constant
        for channel, value in enumerate(value_triplet)
    )
    linear = strips_by_id["forward_linear"]["samples"]
    endpoint_error = max(
        abs(a - b)
        for left, right in ((forward[0], linear[0]), (forward[-1], linear[-1]))
        for a, b in zip(left, right, strict=True)
    )
    tolerance = 2.0e-6
    if reversal_error > tolerance or constant_error > tolerance or endpoint_error > tolerance:
        raise RuntimeError(
            f"strip metamorphic failure: reversal={reversal_error} constant={constant_error} endpoint={endpoint_error}"
        )

    strip_receipts: list[dict[str, Any]] = []
    for strip in payload["strips"]:
        samples = strip["samples"]
        image = Image.new("RGB", (len(samples), 48))
        row = [tuple(round(max(0.0, min(1.0, channel)) * 255.0) for channel in rgb) for rgb in samples]
        image.putdata(row * 48)
        png_path = out_dir / f"{strip['id']}.png"
        image.save(png_path, format="PNG", optimize=False)
        strip_receipts.append(
            {
                **{key: value for key, value in strip.items() if key != "samples"},
                "sample_rgb_sha256": _canonical_sha256(samples),
                "png_file": png_path.relative_to(REPO_ROOT).as_posix(),
                "png_sha256": _sha256_file(png_path),
            }
        )

    out_json.write_text(
        json.dumps(
            {
                "schema_id": "viewer.blackbody_palette_strips.v1",
                "classification": "passed",
                "sampler_exe": str(sampler_exe),
                "sampler_exe_sha256": _sha256_file(sampler_exe),
                "sampler_id": payload["sampler_id"],
                "compiled_lut_inc_sha256": inc_hash,
                "lut_metadata_sha256": _sha256_file(LUT_METADATA),
                "sample_count": payload["sample_count"],
                "checks": {
                    "tolerance": tolerance,
                    "reversal_max_channel_error": reversal_error,
                    "equal_endpoint_max_channel_error": constant_error,
                    "mapping_endpoint_max_channel_error": endpoint_error,
                },
                "strips": strip_receipts,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)

    freeze = subparsers.add_parser("freeze-preservation")
    freeze.add_argument("--out", required=True, type=Path)
    freeze.add_argument("--runtime-exe", type=Path)

    verify = subparsers.add_parser("verify-preservation")
    verify.add_argument("--out", required=True, type=Path)
    verify.add_argument("--runtime-exe", type=Path)

    strips = subparsers.add_parser("render-strips")
    strips.add_argument("--sampler-exe", required=True, type=Path)
    strips.add_argument("--out-json", required=True, type=Path)
    strips.add_argument("--out-dir", required=True, type=Path)

    args = parser.parse_args()
    if args.command == "freeze-preservation":
        freeze_preservation(args.out, args.runtime_exe)
    elif args.command == "verify-preservation":
        verify_preservation(args.out, args.runtime_exe)
    else:
        render_strips(args.sampler_exe, args.out_json, args.out_dir)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
