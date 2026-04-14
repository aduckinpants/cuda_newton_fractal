from __future__ import annotations

import json
import struct
from pathlib import Path

import pytest

from tools.reality_toolkit.fractal_explorer import fractal_extensions as extensions_mod


def _write_test_bmp24(path: Path) -> None:
    width = 2
    height = 1
    row_stride = width * 3
    row_padded = (row_stride + 3) & ~3
    pixel_offset = 54
    file_size = pixel_offset + row_padded * height

    header = bytearray()
    header.extend(b"BM")
    header.extend(struct.pack("<I", file_size))
    header.extend(b"\x00\x00\x00\x00")
    header.extend(struct.pack("<I", pixel_offset))
    header.extend(struct.pack("<I", 40))
    header.extend(struct.pack("<i", width))
    header.extend(struct.pack("<i", height))
    header.extend(struct.pack("<H", 1))
    header.extend(struct.pack("<H", 24))
    header.extend(struct.pack("<I", 0))
    header.extend(struct.pack("<I", row_padded * height))
    header.extend(struct.pack("<i", 2835))
    header.extend(struct.pack("<i", 2835))
    header.extend(struct.pack("<I", 0))
    header.extend(struct.pack("<I", 0))

    row = bytes([
        0, 0, 255,
        0, 255, 0,
        0, 0,
    ])
    path.write_bytes(bytes(header) + row)


def test_load_manifest_and_build_sidecar_request(tmp_path: Path) -> None:
    state_json = tmp_path / "ripple_state.json"
    state_json.write_text(json.dumps({"fractal_type": "explaino_ripple"}), encoding="utf-8")

    manifest_path = tmp_path / "fractal_extensions_manifest.json"
    manifest_path.write_text(
        json.dumps(
            {
                "finding_group": "fractal_extensions_gallery_2026_04_13",
                "scenes": [
                    {
                        "scene_id": "ripple_cathedral",
                        "state_json": str(state_json),
                        "why": "Chladni-like ring interference around a root basin shell.",
                        "region": {
                            "center_x": 0.0,
                            "center_y": 0.0,
                            "span_x": 0.35,
                            "span_y": 0.35,
                            "grid_width": 8,
                            "grid_height": 8,
                        },
                        "sidecars": [
                            {
                                "sidecar_id": "newton_z3m1",
                                "expression": "iterate(z - (z^3 - 1) / (3 * z^2), 60)",
                                "epsilon": 1e-10,
                                "escape_radius": 1000.0,
                                "metrics": ["iterations", "status", "value", "abs2", "derivative"],
                                "notes": "Three-root cathedral glass comparison field.",
                            }
                        ],
                    }
                ],
            },
            indent=2,
        ),
        encoding="utf-8",
    )

    manifest = extensions_mod.load_fractal_extensions_manifest(manifest_path)

    assert manifest.finding_group == "fractal_extensions_gallery_2026_04_13"
    assert len(manifest.scenes) == 1
    scene = manifest.scenes[0]
    assert scene.scene_id == "ripple_cathedral"
    assert scene.state_json == state_json
    assert scene.region.grid_width == 8
    assert scene.region.span_x == pytest.approx(0.35)
    assert scene.sidecars[0].sidecar_id == "newton_z3m1"

    request = extensions_mod.build_sidecar_request(scene, scene.sidecars[0])

    assert request["function_id"] == "generic.sample"
    assert request["mode"] == "grid"
    assert request["request_id"] == "ripple_cathedral-newton_z3m1"
    assert request["region"] == {
        "center_x": 0.0,
        "center_y": 0.0,
        "span_x": 0.35,
        "span_y": 0.35,
        "grid_width": 8,
        "grid_height": 8,
    }
    assert request["function"]["expression"] == "iterate(z - (z^3 - 1) / (3 * z^2), 60)"
    assert request["function"]["epsilon"] == pytest.approx(1e-10)
    assert request["function"]["escape_radius"] == pytest.approx(1000.0)
    assert request["metrics"] == ["iterations", "status", "value", "abs2", "derivative"]
    assert request["operator_context"]["operator"] == "fractal_extensions"


def test_run_composite_dry_run_writes_scene_plan_and_requests(tmp_path: Path) -> None:
    repo_root = tmp_path / "repo_without_published_runtime"
    repo_root.mkdir()
    state_json = tmp_path / "vortex_state.json"
    state_json.write_text(json.dumps({"fractal_type": "explaino_vortex"}), encoding="utf-8")

    manifest = extensions_mod.FractalExtensionsManifest(
        finding_group="fractal_extensions_gallery_2026_04_13",
        scenes=(
            extensions_mod.FractalExtensionScene(
                scene_id="vortex_glass",
                state_json=state_json,
                why="Swirling basin-glass study.",
                region=extensions_mod.FractalExtensionRegion(
                    center_x=0.0,
                    center_y=0.0,
                    span_x=0.25,
                    span_y=0.25,
                    grid_width=6,
                    grid_height=6,
                ),
                sidecars=(
                    extensions_mod.FractalExtensionSidecar(
                        sidecar_id="compose_sin",
                        expression="compose(sin(z), z^2 + c)",
                        params={"c_real": -0.745, "c_imag": 0.186},
                    ),
                ),
            ),
        ),
    )

    def forbid_capture(*args: object, **kwargs: object) -> Path:
        raise AssertionError("dry-run should not invoke capture")

    summary = extensions_mod.run_fractal_extensions_composite(
        repo_root=repo_root,
        manifest=manifest,
        out_dir=tmp_path / "out_gallery",
        analyze_findings=False,
        dry_run=True,
        capture_runner=forbid_capture,
    )

    scene_dir = tmp_path / "out_gallery" / "vortex_glass"
    sidecar_dir = scene_dir / "sidecars" / "compose_sin"

    assert (scene_dir / "scene.json").exists()
    assert (sidecar_dir / "request.json").exists()
    assert not (sidecar_dir / "response.json").exists()
    assert (tmp_path / "out_gallery" / "fractal_extensions_summary.json").exists()
    assert summary["dry_run"] is True
    assert summary["scene_count"] == 1
    assert summary["scene_summaries"][0]["captured"] is False
    assert summary["scene_summaries"][0]["finding_dir"] == str(scene_dir)
    assert summary["scene_summaries"][0]["sidecars"][0]["response_path"] is None


def test_run_composite_live_writes_finding_sidecars_and_analysis(tmp_path: Path) -> None:
    repo_root = tmp_path / "cuda_newton_fractal_clone"
    repo_root.mkdir()
    state_json = tmp_path / "lambda_state.json"
    state_json.write_text(
        json.dumps({"fractal_type": "explaino_lambda", "params": {"lambda_real": 2.9685855}}),
        encoding="utf-8",
    )

    manifest = extensions_mod.FractalExtensionsManifest(
        finding_group="fractal_extensions_gallery_2026_04_13",
        scenes=(
            extensions_mod.FractalExtensionScene(
                scene_id="lambda_cathedral",
                state_json=state_json,
                why="Stained-glass basin contrast study.",
                region=extensions_mod.FractalExtensionRegion(
                    center_x=0.1,
                    center_y=-0.1,
                    span_x=0.4,
                    span_y=0.3,
                    grid_width=4,
                    grid_height=3,
                ),
                sidecars=(
                    extensions_mod.FractalExtensionSidecar(
                        sidecar_id="exp_glass",
                        expression="compose(exp(z), z^2 + c)",
                        params={"c_real": -0.75, "c_imag": 0.1},
                        notes="Exponential interference sidecar.",
                    ),
                ),
            ),
        ),
    )

    diagnostics_dir = tmp_path / "diagnostics_last"

    def fake_capture_runner(
        repo_root_arg: Path,
        state_json_path: Path,
        *,
        exe_path: Path | None = None,
        timeout_seconds: float = 180.0,
    ) -> Path:
        assert repo_root_arg == repo_root
        assert state_json_path == state_json
        diagnostics_dir.mkdir(parents=True, exist_ok=True)
        _write_test_bmp24(diagnostics_dir / "frame.bmp")
        (diagnostics_dir / "state.json").write_text(state_json.read_text(encoding="utf-8"), encoding="utf-8")
        return diagnostics_dir

    def fake_sample_runner(
        repo_root_arg: Path,
        request: dict[str, object],
        *,
        exe_path: Path | None = None,
        timeout_seconds: float = 180.0,
    ) -> dict[str, object]:
        assert repo_root_arg == repo_root
        assert request["function_id"] == "generic.sample"
        return {
            "ok": True,
            "request_id": request["request_id"],
            "function_id": "generic.sample",
            "summary": {"sample_count": 12, "mean_abs2": 9.5},
            "samples": [
                {"sequence_index": 0, "grid_x": 0, "grid_y": 0, "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
            ],
        }

    def fake_analysis_runner(finding_dir: Path) -> dict[str, object]:
        return {"finding_dir": str(finding_dir), "kind": "analysis"}

    def fake_analysis_writer(analysis: dict[str, object], out_dir: Path) -> dict[str, str]:
        out_dir.mkdir(parents=True, exist_ok=True)
        report_path = out_dir / "report.txt"
        report_path.write_text(json.dumps(analysis, indent=2), encoding="utf-8")
        return {"report_txt": str(report_path)}

    summary = extensions_mod.run_fractal_extensions_composite(
        repo_root=repo_root,
        manifest=manifest,
        out_dir=tmp_path / "live_gallery",
        analyze_findings=True,
        dry_run=False,
        capture_runner=fake_capture_runner,
        sample_runner=fake_sample_runner,
        analysis_runner=fake_analysis_runner,
        analysis_writer=fake_analysis_writer,
    )

    scene_dir = tmp_path / "live_gallery" / "lambda_cathedral"
    sidecar_dir = scene_dir / "sidecars" / "exp_glass"

    assert (scene_dir / "frame.png").exists()
    assert (scene_dir / "state.json").exists()
    assert (scene_dir / "finding.json").exists()
    assert (scene_dir / "finding.md").exists()
    assert (scene_dir / "field-notes.md").exists()
    assert (scene_dir / "extension_sidecars.json").exists()
    assert (scene_dir / "analysis" / "report.txt").exists()
    assert (sidecar_dir / "request.json").exists()
    assert (sidecar_dir / "response.json").exists()

    finding_md = (scene_dir / "finding.md").read_text(encoding="utf-8")
    assert "Extension Sidecars" in finding_md
    assert "compose(exp(z), z^2 + c)" in finding_md

    sidecar_manifest = json.loads((scene_dir / "extension_sidecars.json").read_text(encoding="utf-8"))
    assert sidecar_manifest["scene_id"] == "lambda_cathedral"
    assert sidecar_manifest["sidecars"][0]["sidecar_id"] == "exp_glass"
    assert sidecar_manifest["sidecars"][0]["summary"]["mean_abs2"] == pytest.approx(9.5)

    assert summary["dry_run"] is False
    assert summary["scene_summaries"][0]["captured"] is True
    assert summary["scene_summaries"][0]["analysis_manifest"]["report_txt"].replace("\\", "/").endswith("analysis/report.txt")