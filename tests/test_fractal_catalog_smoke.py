from __future__ import annotations

from pathlib import Path

from tools.reality_toolkit.fractal_explorer import fractal_catalog_smoke as smoke_mod


def test_extract_fractal_types_from_catalog() -> None:
    catalog = {
        "engine_version": 1,
        "functions": [
            {
                "id": "fractal.sample",
                "parameters": [
                    {
                        "path": "fractal.view.fractal_type",
                        "type": "enum",
                        "options": [
                            {"id": "newton", "label": "Newton"},
                            {"id": "mandelbrot", "label": "Mandelbrot"},
                            {"id": "explaino", "label": "Explaino"},
                        ],
                    }
                ],
            }
        ],
    }

    assert smoke_mod.extract_fractal_types(catalog) == ["newton", "mandelbrot", "explaino"]


def test_run_fractal_catalog_smoke_collects_successes_and_failures(tmp_path: Path) -> None:
    repo_root = tmp_path / "cuda_newton_fractal_clone"
    repo_root.mkdir()

    def fake_describer(repo_root_arg: Path, *, exe_path=None, timeout_seconds: float = 30.0):
        assert repo_root_arg == repo_root
        return {
            "functions": [
                {
                    "id": "fractal.sample",
                    "parameters": [
                        {
                            "path": "fractal.view.fractal_type",
                            "type": "enum",
                            "options": ["newton", "mandelbrot"],
                        }
                    ],
                }
            ]
        }

    def fake_sample_runner(repo_root_arg: Path, request: dict[str, object], *, exe_path=None, timeout_seconds: float = 180.0):
        fractal_type = None
        for override in request["overrides"]:
            if override["path"] == "fractal.view.fractal_type":
                fractal_type = override["value"]
                break
        if fractal_type == "mandelbrot":
            raise RuntimeError("mandelbrot failed smoke")
        return {
            "ok": True,
            "runtime": {"fractal_type": fractal_type, "device_id": 0},
            "summary": {"sample_count": 2, "mean_iterations": 7.5, "escape_fraction": 0.5},
        }

    report = smoke_mod.run_fractal_catalog_smoke(
        repo_root=repo_root,
        describe_runner=fake_describer,
        sample_runner=fake_sample_runner,
    )

    assert report["fractal_types_total"] == 2
    assert report["fractal_types_ok"] == 1
    assert report["fractal_types_failed"] == 1
    assert report["results"][0]["fractal_type"] == "newton"
    assert report["results"][0]["ok"] is True
    assert report["results"][1]["fractal_type"] == "mandelbrot"
    assert report["results"][1]["ok"] is False
    assert "failed smoke" in report["results"][1]["error"]
