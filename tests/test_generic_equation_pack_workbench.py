from __future__ import annotations

import json
from pathlib import Path

import pytest

from tools.reality_toolkit.fractal_explorer import generic_equation_pack as pack_mod


def _sample_pack() -> dict[str, object]:
    return {
        "schema_version": 1,
        "pack_id": "test_quadratic",
        "name": "Test Quadratic",
        "formula": {
            "kind": "iterate_map",
            "iteration_param": "steps",
            "ast": {
                "op": "add",
                "args": [
                    {"op": "pow_int", "base": {"op": "var_z"}, "exponent": 2},
                    {"op": "complex_param", "name": "c"},
                ],
            },
        },
        "params": {"steps": 12.0, "c_real": -0.75, "c_imag": 0.1},
        "controls": [
            {"id": "steps", "param": "steps", "label": "Steps", "min": 1.0, "max": 100.0, "step": 1.0, "default": 12.0}
        ],
        "epsilon": 1e-9,
        "escape_radius": 1000.0,
        "region": {"center_x": 0.0, "center_y": 0.0, "span_x": 2.0, "span_y": 2.0, "grid_width": 2, "grid_height": 1},
    }


def test_workbench_writes_pack_request_response_manifest_and_png(tmp_path: Path) -> None:
    pack_path = tmp_path / "pack.json"
    pack_path.write_text(json.dumps(_sample_pack(), indent=2), encoding="utf-8")
    out_dir = tmp_path / "out"

    def fake_runner(repo_root: Path, request: dict[str, object], **_: object) -> dict[str, object]:
        assert request["function_id"] == "generic.sample"
        assert request["execution"] == {"backend_preference": "cuda"}
        function = request["function"]
        assert "ast" in function
        assert "expression" not in function
        assert function["params"]["steps"] == pytest.approx(18.0)
        return {
            "ok": True,
            "function_id": "generic.sample",
            "request_id": request["request_id"],
            "runtime": {"backend_used": "cuda"},
            "summary": {"sample_count": 2},
            "samples": [
                {"sequence_index": 0, "grid_x": 0, "grid_y": 0, "iterations": 4, "status": "bounded", "value_x": 1.0, "value_y": 0.0, "abs2": 1.0},
                {"sequence_index": 0, "grid_x": 1, "grid_y": 0, "iterations": 7, "status": "bounded", "value_x": 0.0, "value_y": 1.0, "abs2": 1.0},
            ],
        }

    result = pack_mod.run_equation_pack_workbench(
        pack_path,
        out_dir,
        backend="cuda",
        control_overrides={"steps": 18.0},
        sample_runner=fake_runner,
    )

    assert (out_dir / "pack.json").exists()
    assert (out_dir / "request.json").exists()
    assert (out_dir / "response.json").exists()
    assert (out_dir / "gallery_manifest.json").exists()
    assert (out_dir / "frame_0000.png").read_bytes()[:8] == b"\x89PNG\r\n\x1a\n"
    assert result["gallery_manifest"] == str(out_dir / "gallery_manifest.json")

    request = json.loads((out_dir / "request.json").read_text(encoding="utf-8"))
    assert request["function"]["ast"]["op"] == "add"
    assert "expression" not in request["function"]


def test_pack_validation_rejects_duplicate_control_params(tmp_path: Path) -> None:
    pack = _sample_pack()
    pack["controls"] = [
        {"id": "a", "param": "steps"},
        {"id": "b", "param": "steps"},
    ]
    pack_path = tmp_path / "bad.json"
    pack_path.write_text(json.dumps(pack), encoding="utf-8")

    with pytest.raises(ValueError, match="duplicate"):
        pack_mod.load_equation_pack(pack_path)
