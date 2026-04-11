from __future__ import annotations

import csv
import json
from pathlib import Path

import pytest

from tools.reality_toolkit.fractal_explorer import explaino_param_sensitivity as sensitivity_mod


def test_variant_sensitivity_specs_match_phase2_benchmark_table() -> None:
    specs = sensitivity_mod.resolve_variant_sensitivity_specs()

    assert [spec.variant_name for spec in specs] == [
        "explaino_ripple",
        "explaino_splice",
        "explaino_vortex",
        "explaino_tension",
    ]
    assert [spec.param_path for spec in specs] == [
        "fractal.params.ripple_amplitude",
        "fractal.params.splice_offset",
        "fractal.params.vortex_strength",
        "fractal.params.tension_strength",
    ]
    assert [spec.default_value for spec in specs] == pytest.approx([0.15, 0.5, 0.3, 0.02])
    assert [spec.zero_case_id for spec in specs] == [
        "explaino_ripple_zero",
        "explaino_splice_zero",
        "explaino_vortex_zero",
        "explaino_tension_zero",
    ]
    assert [spec.default_case_id for spec in specs] == [
        "explaino_ripple_default",
        "explaino_splice_default",
        "explaino_vortex_default",
        "explaino_tension_default",
    ]


def test_build_variant_sensitivity_request_uses_phase2_defaults() -> None:
    spec = sensitivity_mod.resolve_variant_sensitivity_specs(["explaino_ripple"])[0]

    request, ticks = sensitivity_mod.build_variant_sensitivity_request(
        spec=spec,
        ticks=3,
        center_x=0.5,
        center_y=-0.25,
        span_x=0.2,
        span_y=0.3,
        grid_width=4,
        grid_height=5,
    )

    assert spec.zero_case_id == "explaino_ripple_zero"
    assert spec.default_case_id == "explaino_ripple_default"
    assert ticks[0] == (0, 0.0, 0.0)
    assert ticks[1][0] == 1
    assert ticks[1][1] == pytest.approx(0.5)
    assert ticks[1][2] == pytest.approx(0.075)
    assert ticks[2] == (2, 1.0, pytest.approx(0.15))
    assert request["function_id"] == "fractal.sample"
    assert request["mode"] == "sequence_grid"
    assert request["region"] == {
        "center_x": 0.5,
        "center_y": -0.25,
        "span_x": 0.2,
        "span_y": 0.3,
        "grid_width": 4,
        "grid_height": 5,
    }
    assert request["overrides"] == [{"path": "fractal.view.fractal_type", "value": "explaino_ripple"}]
    assert request["sequence"] == {
        "zip_paths": True,
        "vary": [{
            "path": "fractal.params.ripple_amplitude",
            "values": [0.0, pytest.approx(0.075), pytest.approx(0.15)],
        }],
    }
    assert request["metrics"] == sensitivity_mod.DEFAULT_SENSITIVITY_METRICS


def test_compute_root_index_entropy_ignores_missing_values() -> None:
    entropy_bits, root_sample_count, unique_root_count = sensitivity_mod.compute_root_index_entropy([
        {"sequence_index": 0, "root_index": 0},
        {"sequence_index": 0, "root_index": 0},
        {"sequence_index": 0, "root_index": 1},
        {"sequence_index": 0, "root_index": None},
        {"sequence_index": 0},
    ])

    assert entropy_bits == pytest.approx(0.9182958340544896)
    assert root_sample_count == 3
    assert unique_root_count == 2


def test_run_explaino_param_sensitivity_writes_outputs(tmp_path: Path) -> None:
    repo_root = tmp_path / "cuda_newton_fractal_clone"
    repo_root.mkdir()
    out_dir = tmp_path / "sensitivity_run"

    captured: dict[str, object] = {}

    def fake_sample_runner(repo_root_arg: Path, request: dict[str, object], *, timeout_seconds: float, exe_path: Path | None = None) -> dict[str, object]:
        captured["repo_root"] = repo_root_arg
        captured["request"] = request
        captured["timeout_seconds"] = timeout_seconds
        captured["exe_path"] = exe_path
        return {
            "ok": True,
            "request_id": request["request_id"],
            "runtime": {
                "exe_path": "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe",
                "fractal_type": "explaino_ripple",
                "device_id": 0,
            },
            "summary": {
                "sample_count": 8,
                "mean_iterations": 11.0,
                "escape_fraction": 0.125,
                "converged_fraction": 0.875,
                "nonfinite_fraction": 0.0,
                "pole_fraction": 0.0,
            },
            "sequence_results": [
                {
                    "sequence_index": 0,
                    "summary": {
                        "mean_iterations": 8.0,
                        "escape_fraction": 0.0,
                        "converged_fraction": 1.0,
                        "nonfinite_fraction": 0.0,
                        "pole_fraction": 0.0,
                    },
                },
                {
                    "sequence_index": 1,
                    "summary": {
                        "mean_iterations": 14.0,
                        "escape_fraction": 0.25,
                        "converged_fraction": 0.75,
                        "nonfinite_fraction": 0.0,
                        "pole_fraction": 0.0,
                    },
                },
            ],
            "samples": [
                {"sequence_index": 0, "root_index": 0},
                {"sequence_index": 0, "root_index": 1},
                {"sequence_index": 0, "root_index": 0},
                {"sequence_index": 0, "root_index": 1},
                {"sequence_index": 1, "root_index": 0},
                {"sequence_index": 1, "root_index": 0},
                {"sequence_index": 1, "root_index": 0},
                {"sequence_index": 1, "root_index": 1},
            ],
        }

    summary = sensitivity_mod.run_explaino_param_sensitivity_sweep(
        repo_root=repo_root,
        variants=["explaino_ripple"],
        ticks=2,
        out_dir=out_dir,
        center_x=0.0,
        center_y=0.0,
        span_x=0.2,
        span_y=0.2,
        grid_width=2,
        grid_height=2,
        timeout_seconds=45.0,
        sample_runner=fake_sample_runner,
    )

    assert captured["repo_root"] == repo_root
    assert captured["timeout_seconds"] == 45.0
    assert (out_dir / "explaino_ripple" / "probe_request.json").exists()
    assert (out_dir / "explaino_ripple" / "probe_response.json").exists()
    assert (out_dir / "explaino_ripple" / "probe_manifest.csv").exists()
    assert (out_dir / "explaino_ripple" / "probe_summary.json").exists()
    assert (out_dir / "explaino_variant_sensitivity.csv").exists()
    assert (out_dir / "explaino_variant_sensitivity_summary.json").exists()

    request_json = json.loads((out_dir / "explaino_ripple" / "probe_request.json").read_text(encoding="utf-8"))
    summary_json = json.loads((out_dir / "explaino_variant_sensitivity_summary.json").read_text(encoding="utf-8"))

    assert request_json["sequence"]["vary"][0]["path"] == "fractal.params.ripple_amplitude"
    assert request_json["metrics"] == sensitivity_mod.DEFAULT_SENSITIVITY_METRICS
    assert summary["variant_count"] == 1
    assert summary_json["variant_summaries"][0]["peak_entropy_tick"] == 0
    assert summary_json["variant_summaries"][0]["zero_case_id"] == "explaino_ripple_zero"

    with (out_dir / "explaino_variant_sensitivity.csv").open("r", encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader(handle))

    assert len(rows) == 2
    assert rows[0]["variant_name"] == "explaino_ripple"
    assert float(rows[0]["param_value"]) == pytest.approx(0.0)
    assert float(rows[0]["root_entropy_bits"]) == pytest.approx(1.0)
    assert float(rows[1]["param_value"]) == pytest.approx(0.15)
    assert float(rows[1]["root_entropy_bits"]) == pytest.approx(0.8112781244591328)


def test_resolve_variant_sensitivity_specs_rejects_unknown_variant() -> None:
    with pytest.raises(ValueError, match="Unknown Explaino sensitivity variant"):
        sensitivity_mod.resolve_variant_sensitivity_specs(["explaino_unknown"])