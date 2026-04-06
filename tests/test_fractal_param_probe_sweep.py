from __future__ import annotations

import csv
import json
from pathlib import Path

from tools.reality_toolkit.fractal_explorer import param_probe_sweep as probe_sweep_mod


def test_build_sequence_grid_request_uses_probe_metadata() -> None:
    request, ticks = probe_sweep_mod.build_sequence_grid_request(
        probe_name="seed_linear",
        ticks=3,
        fractal_type="explaino_lambda",
        center_x=0.5,
        center_y=-0.25,
        span_x=0.2,
        span_y=0.3,
        grid_width=4,
        grid_height=5,
    )

    assert ticks == [(0, 0.0, 0.0), (1, 0.5, 5.0), (2, 1.0, 10.0)]
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
    assert {entry["path"]: entry["value"] for entry in request["overrides"]}["fractal.view.fractal_type"] == "explaino_lambda"
    assert request["sequence"]["zip_paths"] is True
    assert request["sequence"]["vary"] == [{
        "path": "fractal.params.explaino_seed",
        "values": [0.0, 5.0, 10.0],
    }]
    assert request["metrics"] == probe_sweep_mod.DEFAULT_SAMPLE_METRICS


def test_run_sample_probe_sweep_writes_outputs(tmp_path: Path) -> None:
    repo_root = tmp_path / "cuda_newton_fractal_clone"
    repo_root.mkdir()
    out_dir = tmp_path / "probe_run"

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
                "fractal_type": "explaino",
                "device_id": 0,
            },
            "summary": {
                "sample_count": 8,
                "mean_iterations": 12.5,
                "escape_fraction": 0.25,
                "converged_fraction": 0.75,
                "nonfinite_fraction": 0.0,
                "pole_fraction": 0.0,
                "best_sequence_index": 1,
            },
            "sequence_results": [
                {
                    "sequence_index": 0,
                    "applied": {"fractal.params.explaino_seed": 0.0},
                    "summary": {"mean_iterations": 10.0, "escape_fraction": 0.2},
                },
                {
                    "sequence_index": 1,
                    "applied": {"fractal.params.explaino_seed": 10.0},
                    "summary": {"mean_iterations": 15.0, "escape_fraction": 0.3},
                },
            ],
            "samples": [],
        }

    summary = probe_sweep_mod.run_sample_probe_sweep(
        repo_root=repo_root,
        probe_name="seed_linear",
        ticks=2,
        out_dir=out_dir,
        fractal_type="explaino",
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
    assert (out_dir / "probe_request.json").exists()
    assert (out_dir / "probe_response.json").exists()
    assert (out_dir / "probe_summary.json").exists()
    assert (out_dir / "probe_manifest.csv").exists()
    assert summary["mode"] == "sample_sequence_grid"
    assert summary["best_sequence_index"] == 1
    assert summary["best_tick"] == 1
    assert summary["best_param_value"] == 10.0

    request_json = json.loads((out_dir / "probe_request.json").read_text(encoding="utf-8"))
    response_json = json.loads((out_dir / "probe_response.json").read_text(encoding="utf-8"))
    summary_json = json.loads((out_dir / "probe_summary.json").read_text(encoding="utf-8"))

    assert request_json["sequence"]["vary"][0]["path"] == "fractal.params.explaino_seed"
    assert response_json["summary"]["best_sequence_index"] == 1
    assert summary_json["best_param_value"] == 10.0

    with (out_dir / "probe_manifest.csv").open("r", encoding="utf-8", newline="") as handle:
        rows = list(csv.DictReader(handle))
    assert len(rows) == 2
    assert rows[0]["sequence_index"] == "0"
    assert rows[1]["param_value"] == "10"
