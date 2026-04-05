from __future__ import annotations

import json
import os
import struct
from pathlib import Path

from tools.reality_toolkit.fractal_explorer.paths import findings_root
import tools.reality_toolkit.fractal_explorer.seed_sweep as seed_sweep_mod
from tools.reality_toolkit.fractal_explorer.seed_sweep import build_seed_values, format_seed_label


def test_build_seed_values_inclusive_step_range() -> None:
    assert build_seed_values(seed_start=0.70, seed_stop=0.80, seed_step=0.02) == [0.70, 0.72, 0.74, 0.76, 0.78, 0.80]


def test_build_seed_values_uses_explicit_seeds() -> None:
    assert build_seed_values(explicit_seeds=[0.745, 0.755, 0.775]) == [0.745, 0.755, 0.775]


def test_build_seed_values_rejects_zero_step() -> None:
    try:
        build_seed_values(seed_start=0.70, seed_stop=0.80, seed_step=0.0)
    except ValueError as exc:
        assert "non-zero" in str(exc)
    else:
        raise AssertionError("Expected zero seed_step to raise ValueError")


def test_format_seed_label_uses_fixed_precision() -> None:
    assert format_seed_label(0.755) == "0.755000"


def test_runtime_command_wraps_cmd_launcher() -> None:
    launcher = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd")
    assert seed_sweep_mod._runtime_command(launcher, "--validate-ui") == [
        "cmd.exe",
        "/d",
        "/c",
        str(launcher),
        "--validate-ui",
    ]


def test_runtime_command_keeps_exe_direct() -> None:
    exe_path = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe")
    assert seed_sweep_mod._runtime_command(exe_path, "--validate-ui") == [
        str(exe_path),
        "--validate-ui",
    ]


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


def test_run_seed_sweep_archives_findings(tmp_path: Path) -> None:
    publish_root_dir = tmp_path / "publish"
    os.environ["SALT_FRACTAL_ROOT"] = str(publish_root_dir)

    repo_root = tmp_path / "repo"
    repo_root.mkdir()
    out_dir = tmp_path / "artifacts" / "seed_sweep_2026-04-05_171500"

    config = seed_sweep_mod.SweepConfig(
        repo_root=repo_root,
        exe_path=tmp_path / "fractal_ui.exe",
        diagnostics_last_dir=tmp_path / "runtime" / "diagnostics" / "last",
        out_dir=out_dir,
        seeds=[0.70, 0.72],
        fractal_type="explaino",
        validate_ui=False,
    )
    config.archive_findings = True
    config.finding_group = "explaino_seed_scout"

    original_run_one_seed = seed_sweep_mod._run_one_seed
    try:
        def fake_run_one_seed(config: seed_sweep_mod.SweepConfig, seed: float, previous_pixels: bytes | None):
            seed_label = seed_sweep_mod.format_seed_label(seed)
            diagnostics_dir = config.out_dir / f"seed_{seed_label}" / "diagnostics_last"
            diagnostics_dir.mkdir(parents=True, exist_ok=True)
            _write_test_bmp24(diagnostics_dir / "frame.bmp")
            (diagnostics_dir / "state.json").write_text(
                json.dumps({"fractal_type": config.fractal_type, "params": {"max_iter": 500}}, indent=2),
                encoding="utf-8",
            )
            return {
                "seed": seed,
                "seed_label": seed_label,
                "mode": "capture_diagnostic",
                "ok": True,
                "returncode": 0,
                "fractal_type": config.fractal_type,
                "state_fractal_type": config.fractal_type,
                "frame_path": str(diagnostics_dir / "frame.bmp"),
                "state_path": str(diagnostics_dir / "state.json"),
                "stdout": "",
                "stderr": "",
                "error": None,
                "mean_luma": 1.0,
                "std_luma": 2.0,
                "edge_energy": 3.0,
                "delta_prev_mean_abs": 4.0,
            }, b"pixels"

        seed_sweep_mod._run_one_seed = fake_run_one_seed
        summary = seed_sweep_mod.run_seed_sweep(config)
    finally:
        seed_sweep_mod._run_one_seed = original_run_one_seed

    finding_batch_dir = findings_root(repo_root) / "explaino_seed_scout" / out_dir.name
    first_finding_dir = finding_batch_dir / "001__seed_0.700000"
    second_finding_dir = finding_batch_dir / "002__seed_0.720000"

    assert (first_finding_dir / "frame.png").exists()
    assert (first_finding_dir / "finding.json").exists()
    assert (second_finding_dir / "frame.png").exists()
    assert summary["finding_batch_dir"] == str(finding_batch_dir)
    assert summary["runs"][0]["finding_dir"] == str(first_finding_dir)