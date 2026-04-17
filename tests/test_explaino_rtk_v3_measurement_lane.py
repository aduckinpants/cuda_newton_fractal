from __future__ import annotations

import json
import os
import sys
from pathlib import Path

import pytest


REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "tools"))

import explaino_rtk_v3_measurement_lane as rtk_lane  # noqa: E402


def _write_minimal_fits(path: Path) -> None:
    fits = pytest.importorskip("astropy.io.fits")
    import numpy as np

    primary = fits.PrimaryHDU(data=np.zeros((8, 8), dtype=np.float32))
    op_state = fits.BinTableHDU.from_columns(
        [fits.Column(name="OP_STATE", format="4B", array=np.array([[116, 101, 115, 116]], dtype=np.uint8))],
        name="OP_STATE",
    )
    fits.HDUList([primary, op_state]).writeto(path, overwrite=True)


def test_discover_fits_inputs_prefers_explicit_and_scans_non_checkpoint_fits(tmp_path: Path) -> None:
    explicit_dir = tmp_path / "explicit"
    scanned_dir = tmp_path / "nested" / "archive" / "candidate"
    explicit_dir.mkdir(parents=True)
    scanned_dir.mkdir(parents=True)

    explicit_fits = explicit_dir / "checkpoint_final.fits"
    scanned_fits = scanned_dir / "frames_delta_stack.fits"
    _write_minimal_fits(explicit_fits)
    _write_minimal_fits(scanned_fits)

    discovered = rtk_lane.discover_fits_inputs(
        explicit_paths=[explicit_fits],
        search_roots=[tmp_path],
        max_depth=6,
    )

    assert discovered[0] == explicit_fits
    assert scanned_fits in discovered


def test_build_tool_env_includes_nine_scripts_path(tmp_path: Path) -> None:
    nine_root = tmp_path / "nine"
    scripts_root = nine_root / "scripts"
    scripts_root.mkdir(parents=True)

    env = rtk_lane.build_tool_env(nine_root)
    pythonpath_entries = [entry for entry in env.get("PYTHONPATH", "").split(os.pathsep) if entry]
    assert str(scripts_root.resolve()) in pythonpath_entries


def test_harvest_dataset_outputs_collects_both_ply_surfaces(tmp_path: Path) -> None:
    dataset_root = tmp_path / "runs" / "group_000_other_8x8"
    ghost_root = dataset_root / "ghost_plate"
    inv_root = dataset_root / "invariance_sculpture"
    ghost_root.mkdir(parents=True)
    inv_root.mkdir(parents=True)
    (tmp_path / "out_unknown_toolkit_eval_all.csv").write_text("dataset,status\ngroup_000_other_8x8,ok\n", encoding="utf-8")

    (ghost_root / "out_ghost_plate_summary.json").write_text("{}", encoding="utf-8")
    (inv_root / "out_invariance_sculpture_summary.csv").write_text("metric,value\nx,1\n", encoding="utf-8")
    (inv_root / "out_invariance_sculpture_stability.ply").write_text("ply\n", encoding="utf-8")
    (inv_root / "out_invariance_sculpture_churn.ply").write_text("ply\n", encoding="utf-8")

    harvested = rtk_lane.harvest_dataset_outputs("alpha", tmp_path)
    assert harvested["rtk_dataset_ids"] == ["group_000_other_8x8"]
    assert harvested["ghost_plate"]["status"] == "ok"
    assert harvested["invariance"]["status"] == "ok"
    assert harvested["invariance_sculpture"]["status"] == "ok"
    assert harvested["invariance_sculpture"]["stability_ply"].endswith("out_invariance_sculpture_stability.ply")
    assert harvested["invariance_sculpture"]["churn_ply"].endswith("out_invariance_sculpture_churn.ply")


def test_scan_existing_measurement_families_detects_named_results(tmp_path: Path) -> None:
    result_root = tmp_path / "results"
    (result_root / "ghost_plate" / "godel_g13").mkdir(parents=True)
    (result_root / "ghost_plate" / "godel_g13" / "out_ghost_plate_summary.json").write_text("{}", encoding="utf-8")
    (result_root / "fits_invariance_study").mkdir(parents=True)
    (result_root / "fits_invariance_study" / "fits_invariance_study_summary.json").write_text("{}", encoding="utf-8")
    (result_root / "bridge_contrast_v2" / "_flat").mkdir(parents=True)
    (result_root / "bridge_contrast_v2" / "_flat" / "godel_g10__invariance_sculpture__out_invariance_sculpture_stability.ply").write_text("ply\n", encoding="utf-8")
    (result_root / "bridge_contrast_v2" / "_flat" / "godel_g10__invariance_sculpture__out_invariance_sculpture_churn.ply").write_text("ply\n", encoding="utf-8")

    catalog = rtk_lane.scan_existing_measurement_families([result_root], max_depth=6)
    assert len(catalog["ghost_plate"]) == 1
    assert len(catalog["fits_invariance_study"]) == 1
    assert len(catalog["invariance_sculpture"]) == 2


def test_run_measurement_lane_with_fake_prepare_and_runner(tmp_path: Path) -> None:
    source_root = tmp_path / "source"
    out_root = tmp_path / "out"
    external_root = tmp_path / "external"
    source_root.mkdir()
    external_root.mkdir()

    source_fits = source_root / "checkpoint_final.fits"
    _write_minimal_fits(source_fits)
    (external_root / "godel_fits_entropy_campaign").mkdir(parents=True)
    (external_root / "godel_fits_entropy_campaign" / "out_godel_fits_entropy_campaign_summary.json").write_text(
        "{}",
        encoding="utf-8",
    )

    prepare_script = tmp_path / "fake_prepare.py"
    runner_script = tmp_path / "fake_runner.py"
    prepare_script.write_text(
        """
import argparse
import csv
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("--data-dir", required=True)
ap.add_argument("--out-root", required=True)
ap.add_argument("--min-frames", required=True)
args = ap.parse_args()
out_root = Path(args.out_root)
group_dir = out_root / "groups" / "group_000__other__8x8"
group_dir.mkdir(parents=True, exist_ok=True)
(group_dir / "frame_0000.npy").write_bytes(b"npy")
with (out_root / "out_fits_groups_emitted.csv").open("w", encoding="utf-8", newline="") as handle:
    writer = csv.DictWriter(handle, fieldnames=["group_name", "group_dir", "suffix", "height", "width", "n_files", "n_emitted", "manifest_csv"])
    writer.writeheader()
    writer.writerow({
        "group_name": "group_000__other__8x8",
        "group_dir": str(group_dir),
        "suffix": "other.fits",
        "height": "8",
        "width": "8",
        "n_files": "1",
        "n_emitted": "1",
        "manifest_csv": str(group_dir / "out_group_manifest.csv"),
    })
(out_root / "out_fits_groups_emitted.csv").parent.mkdir(parents=True, exist_ok=True)
        """.strip()
        + "\n",
        encoding="utf-8",
    )
    runner_script.write_text(
        """
import argparse
import csv
import json
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("--groups-csv", required=True)
ap.add_argument("--out-root", required=True)
ap.add_argument("--blind-mode", required=True)
ap.add_argument("--profile", required=True)
ap.add_argument("--prefer", required=True)
args = ap.parse_args()
rows = list(csv.DictReader(Path(args.groups_csv).open("r", encoding="utf-8")))
dataset = rows[0]["dataset"]
run_root = Path(args.out_root) / "runs" / "group_000_other_8x8"
ghost = run_root / "ghost_plate"
inv = run_root / "invariance_sculpture"
ghost.mkdir(parents=True, exist_ok=True)
inv.mkdir(parents=True, exist_ok=True)
(Path(args.out_root) / "out_unknown_toolkit_eval_all.csv").write_text("dataset,status\\ngroup_000_other_8x8,ok\\n", encoding="utf-8")
(ghost / "out_ghost_plate_summary.json").write_text(json.dumps({"dataset": dataset}), encoding="utf-8")
(inv / "out_invariance_sculpture_summary.csv").write_text("metric,value\\nfoo,1\\n", encoding="utf-8")
(inv / "out_invariance_sculpture_metadata.json").write_text(json.dumps({"dataset": dataset}), encoding="utf-8")
(inv / "out_invariance_sculpture_stability.ply").write_text("ply\\n", encoding="utf-8")
(inv / "out_invariance_sculpture_churn.ply").write_text("ply\\n", encoding="utf-8")
        """.strip()
        + "\n",
        encoding="utf-8",
    )

    receipt = rtk_lane.run_measurement_lane(
        explicit_fits=[source_fits],
        search_roots=[],
        external_results_roots=[external_root],
        out_root=out_root,
        run_stamp="fake_run",
        nine_root=tmp_path,
        prepare_script=prepare_script,
        runner_script=runner_script,
        profile="entry",
    )

    assert receipt["ok"] is True
    run_root = out_root / "fake_run"
    selection_manifest = json.loads((run_root / "manifest" / "rtk_v3_selection_manifest.json").read_text(encoding="utf-8"))
    harvest_summary = json.loads((run_root / "manifest" / "rtk_v3_harvest_summary.json").read_text(encoding="utf-8"))
    external_catalog = json.loads((run_root / "manifest" / "rtk_v3_external_family_catalog.json").read_text(encoding="utf-8"))

    assert selection_manifest["entries"][0]["dataset_id"] == "source_checkpoint_final"
    dataset = harvest_summary["datasets"][0]
    assert dataset["ghost_plate"]["status"] == "ok"
    assert dataset["invariance_sculpture"]["stability_ply"].endswith("out_invariance_sculpture_stability.ply")
    assert dataset["invariance_sculpture"]["churn_ply"].endswith("out_invariance_sculpture_churn.ply")
    assert len(external_catalog["entropy_campaign"]) == 1
