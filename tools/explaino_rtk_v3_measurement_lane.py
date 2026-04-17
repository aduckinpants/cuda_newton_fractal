from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import shutil
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable, Sequence


DEFAULT_NINE_ROOT = Path(r"C:\Users\Adam\Desktop\b3\whatisthis\nine")
DEFAULT_OUT_ROOT = Path(r"D:\salt-output\results\explaino_rtk_v3")
DEFAULT_EXTERNAL_RESULTS_ROOT = Path(r"D:\salt-output\results")
DEFAULT_SALTS_ROOT = Path(r"C:\code\hat-rack-v2\salts")
FITS_SUFFIXES = {".fits", ".fit", ".fts"}


def _read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _write_json(path: Path, payload: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def build_tool_env(nine_root: Path) -> dict[str, str]:
    env = dict(os.environ)
    existing = [item for item in env.get("PYTHONPATH", "").split(os.pathsep) if item]
    scripts_root = str((nine_root / "scripts").resolve(strict=False))
    if scripts_root not in existing:
        existing.insert(0, scripts_root)
    env["PYTHONPATH"] = os.pathsep.join(existing)
    return env


def _slug(value: str) -> str:
    cleaned = []
    for ch in str(value):
        if ch.isalnum():
            cleaned.append(ch.lower())
        else:
            cleaned.append("_")
    out = "".join(cleaned).strip("_")
    while "__" in out:
        out = out.replace("__", "_")
    return out or "dataset"


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _iter_fits_files(search_root: Path, max_depth: int) -> Iterable[Path]:
    root = search_root.resolve(strict=False)
    if not root.exists():
        return
    for current_root, dir_names, file_names in os.walk(root):
        current = Path(current_root)
        try:
            rel_parts = current.relative_to(root).parts
        except ValueError:
            rel_parts = ()
        if len(rel_parts) > max_depth:
            dir_names[:] = []
            continue
        dir_names[:] = sorted(dir_names)
        for file_name in sorted(file_names):
            if Path(file_name).suffix.lower() in FITS_SUFFIXES:
                yield current / file_name


def _unique_paths(paths: Iterable[Path]) -> list[Path]:
    ordered: list[Path] = []
    seen: set[str] = set()
    for path in paths:
        key = os.path.normcase(str(path.resolve(strict=False)))
        if key in seen:
            continue
        seen.add(key)
        ordered.append(path)
    return ordered


def discover_fits_inputs(
    *,
    explicit_paths: Sequence[Path] | None = None,
    search_roots: Sequence[Path] | None = None,
    max_depth: int = 8,
) -> list[Path]:
    discovered: list[Path] = []
    for explicit in explicit_paths or ():
        candidate = Path(explicit)
        if candidate.is_file() and candidate.suffix.lower() in FITS_SUFFIXES:
            discovered.append(candidate.resolve())
        elif candidate.is_dir():
            discovered.extend(_iter_fits_files(candidate, max_depth))
    for root in search_roots or ():
        discovered.extend(_iter_fits_files(Path(root), max_depth))
    return _unique_paths(discovered)


def _source_family(path: Path) -> str:
    name = path.name.lower()
    if name == "checkpoint_final.fits":
        return "checkpoint_final"
    if name == "frames_delta_stack.fits":
        return "delta_stack"
    if name == "ghost_plate_cube.fits":
        return "ghost_plate_cube"
    if name == "out_invariance_sculpture_fields.fits":
        return "invariance_fields"
    return "generic_fits"


def _dataset_id_for_path(path: Path) -> str:
    name = path.name.lower()
    if name == "checkpoint_final.fits" and path.parent.name.lower() == "analysis" and path.parent.parent.name:
        return _slug(path.parent.parent.name)
    if name in {"frames_delta_stack.fits", "ghost_plate_cube.fits"} and path.parent.name:
        return _slug(path.parent.name)
    if name == "out_invariance_sculpture_fields.fits" and path.parent.name:
        return _slug(path.parent.name)
    return _slug(f"{path.parent.name}_{path.stem}")


def _companion_paths(path: Path) -> list[str]:
    companions: list[str] = []
    parent = path.parent
    for sibling in sorted(parent.iterdir()):
        if sibling == path or not sibling.is_file():
            continue
        if sibling.suffix.lower() not in {".json", ".csv", ".txt", ".md", ".png", ".ply"}:
            continue
        companions.append(str(sibling))
        if len(companions) >= 12:
            break
    return companions


def inspect_fits_input(path: Path, *, explicit: bool) -> dict:
    try:
        from astropy.io import fits
    except ImportError as exc:  # pragma: no cover - environment-specific guard
        raise RuntimeError("astropy is required to inspect FITS inputs") from exc

    resolved = path.resolve()
    with fits.open(resolved) as hdul:
        hdu_names = [str(hdu.name or "PRIMARY") for hdu in hdul]
        primary = hdul[0].data if hdul else None
        primary_shape = list(primary.shape) if primary is not None else []
        primary_dtype = str(primary.dtype) if primary is not None else None
    return {
        "path": str(resolved),
        "size_bytes": resolved.stat().st_size,
        "sha256": _sha256_file(resolved),
        "dataset_id": _dataset_id_for_path(resolved),
        "source_family": _source_family(resolved),
        "selection_kind": "explicit" if explicit else "scanned",
        "hdu_names": hdu_names,
        "primary_shape": primary_shape,
        "primary_dtype": primary_dtype,
        "companion_paths": _companion_paths(resolved),
        "compatibility": {
            "rtk_prepare_candidate": True,
            "rtk_unknown_candidate": True,
            "walker_compare_candidate": resolved.name.lower() == "checkpoint_final.fits",
        },
    }


def build_selection_manifest(
    *,
    explicit_paths: Sequence[Path] | None = None,
    search_roots: Sequence[Path] | None = None,
    max_depth: int = 8,
    max_datasets: int = 0,
) -> list[dict]:
    explicit_resolved = {
        os.path.normcase(str(Path(path).resolve(strict=False)))
        for path in explicit_paths or ()
        if Path(path).exists()
    }
    entries = [
        inspect_fits_input(path, explicit=os.path.normcase(str(path.resolve(strict=False))) in explicit_resolved)
        for path in discover_fits_inputs(explicit_paths=explicit_paths, search_roots=search_roots, max_depth=max_depth)
    ]
    if max_datasets > 0:
        entries = entries[:max_datasets]
    return _ensure_unique_dataset_ids(entries)


def _ensure_unique_dataset_ids(entries: Sequence[dict]) -> list[dict]:
    counts: dict[str, int] = {}
    unique_entries: list[dict] = []
    for entry in entries:
        base = str(entry["dataset_id"])
        count = counts.get(base, 0)
        counts[base] = count + 1
        updated = dict(entry)
        if count > 0:
            updated["dataset_id"] = f"{base}_{count + 1}"
        unique_entries.append(updated)
    return unique_entries


def _normalize_groups_csv(emitted_csv: Path, dataset_id: str, source_entry: dict) -> Path:
    rows: list[dict[str, str]] = []
    with emitted_csv.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = list(reader.fieldnames or [])
        for row in reader:
            updated = dict(row)
            updated["dataset"] = dataset_id
            updated["source_fits_path"] = str(source_entry["path"])
            updated["source_family"] = str(source_entry["source_family"])
            updated["selection_kind"] = str(source_entry["selection_kind"])
            rows.append(updated)
    for extra in ("dataset", "source_fits_path", "source_family", "selection_kind"):
        if extra not in fieldnames:
            fieldnames.append(extra)
    normalized_csv = emitted_csv.with_name("out_fits_groups_emitted_normalized.csv")
    with normalized_csv.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)
    return normalized_csv


def _read_rtk_dataset_ids(run_root: Path) -> list[str]:
    eval_csv = run_root / "out_unknown_toolkit_eval_all.csv"
    dataset_ids: list[str] = []
    if eval_csv.exists():
        with eval_csv.open("r", encoding="utf-8", newline="") as handle:
            reader = csv.DictReader(handle)
            for row in reader:
                dataset_id = str(row.get("dataset") or "").strip()
                if dataset_id and dataset_id not in dataset_ids:
                    dataset_ids.append(dataset_id)
    if dataset_ids:
        return dataset_ids
    runs_root = run_root / "runs"
    if runs_root.exists():
        for child in sorted(runs_root.iterdir()):
            if child.is_dir():
                dataset_ids.append(child.name)
    return dataset_ids


def _run_command(command: list[str], *, cwd: Path, log_path: Path, extra_env: dict[str, str] | None = None) -> dict:
    env = dict(os.environ)
    if extra_env:
        env.update(extra_env)
    result = subprocess.run(
        command,
        cwd=str(cwd),
        text=True,
        capture_output=True,
        check=False,
        env=env,
    )
    log_lines = [
        "=== COMMAND ===",
        " ".join(command),
        "",
        f"=== RETURN CODE: {result.returncode} ===",
        "",
        "=== STDOUT ===",
        result.stdout or "",
    ]
    if result.stderr:
        log_lines.extend(["", "=== STDERR ===", result.stderr])
    _write_text(log_path, "\n".join(log_lines).rstrip() + "\n")
    return {
        "command": command,
        "cwd": str(cwd),
        "returncode": int(result.returncode),
        "stdout": result.stdout,
        "stderr": result.stderr,
        "log_path": str(log_path),
        "ok": result.returncode == 0,
    }


def _copy_fits_to_source(entry: dict, source_dir: Path) -> Path:
    source_dir.mkdir(parents=True, exist_ok=True)
    src = Path(str(entry["path"]))
    dst = source_dir / src.name
    shutil.copyfile(src, dst)
    return dst


def harvest_dataset_outputs(dataset_id: str, run_root: Path) -> dict:
    rtk_dataset_ids = _read_rtk_dataset_ids(run_root)
    run_roots = [run_root / "runs" / candidate for candidate in rtk_dataset_ids]

    ghost_summaries: list[str] = []
    occupancy_pngs: list[str] = []
    stasis_pngs: list[str] = []
    variance_pngs: list[str] = []
    entropy_summaries: list[str] = []
    inv_summaries: list[str] = []
    inv_metadata_jsons: list[str] = []
    inv_fields_npzs: list[str] = []
    inv_fields_fits_paths: list[str] = []
    stability_plys: list[str] = []
    churn_plys: list[str] = []

    for dataset_root in run_roots:
        ghost_plate_root = dataset_root / "ghost_plate"
        inv_root = dataset_root / "invariance_sculpture"
        candidates = {
            "ghost_summary": ghost_plate_root / "out_ghost_plate_summary.json",
            "occupancy_png": ghost_plate_root / "out_ghost_plate_occupancy.png",
            "stasis_png": ghost_plate_root / "out_ghost_plate_stasis.png",
            "variance_png": ghost_plate_root / "out_ghost_plate_variance.png",
            "entropy_summary": dataset_root / "entropy_campaign" / "out_godel_fits_entropy_campaign_summary.json",
            "inv_summary": inv_root / "out_invariance_sculpture_summary.csv",
            "inv_metadata": inv_root / "out_invariance_sculpture_metadata.json",
            "inv_fields_npz": inv_root / "out_invariance_sculpture_fields.npz",
            "inv_fields_fits": inv_root / "out_invariance_sculpture_fields.fits",
            "stability_ply": inv_root / "out_invariance_sculpture_stability.ply",
            "churn_ply": inv_root / "out_invariance_sculpture_churn.ply",
        }
        if candidates["ghost_summary"].exists():
            ghost_summaries.append(str(candidates["ghost_summary"]))
        if candidates["occupancy_png"].exists():
            occupancy_pngs.append(str(candidates["occupancy_png"]))
        if candidates["stasis_png"].exists():
            stasis_pngs.append(str(candidates["stasis_png"]))
        if candidates["variance_png"].exists():
            variance_pngs.append(str(candidates["variance_png"]))
        if candidates["entropy_summary"].exists():
            entropy_summaries.append(str(candidates["entropy_summary"]))
        if candidates["inv_summary"].exists():
            inv_summaries.append(str(candidates["inv_summary"]))
        if candidates["inv_metadata"].exists():
            inv_metadata_jsons.append(str(candidates["inv_metadata"]))
        if candidates["inv_fields_npz"].exists():
            inv_fields_npzs.append(str(candidates["inv_fields_npz"]))
        if candidates["inv_fields_fits"].exists():
            inv_fields_fits_paths.append(str(candidates["inv_fields_fits"]))
        if candidates["stability_ply"].exists():
            stability_plys.append(str(candidates["stability_ply"]))
        if candidates["churn_ply"].exists():
            churn_plys.append(str(candidates["churn_ply"]))

    return {
        "dataset_id": dataset_id,
        "rtk_dataset_ids": rtk_dataset_ids,
        "run_roots": [str(path) for path in run_roots],
        "ghost_plate": {
            "status": "ok" if ghost_summaries else "absent",
            "summary_json": ghost_summaries[0] if ghost_summaries else "",
            "summary_jsons": ghost_summaries,
            "occupancy_png": occupancy_pngs[0] if occupancy_pngs else "",
            "occupancy_pngs": occupancy_pngs,
            "stasis_png": stasis_pngs[0] if stasis_pngs else "",
            "stasis_pngs": stasis_pngs,
            "variance_png": variance_pngs[0] if variance_pngs else "",
            "variance_pngs": variance_pngs,
        },
        "entropy_campaign": {
            "status": "ok" if entropy_summaries else "absent",
            "summary_json": entropy_summaries[0] if entropy_summaries else "",
            "summary_jsons": entropy_summaries,
        },
        "invariance": {
            "status": "ok" if inv_summaries else "absent",
            "summary_csv": inv_summaries[0] if inv_summaries else "",
            "summary_csvs": inv_summaries,
            "metadata_json": inv_metadata_jsons[0] if inv_metadata_jsons else "",
            "metadata_jsons": inv_metadata_jsons,
            "fields_npz": inv_fields_npzs[0] if inv_fields_npzs else "",
            "fields_npzs": inv_fields_npzs,
            "fields_fits": inv_fields_fits_paths[0] if inv_fields_fits_paths else "",
            "fields_fits_paths": inv_fields_fits_paths,
        },
        "invariance_sculpture": {
            "status": "ok" if (stability_plys or churn_plys) else "absent",
            "stability_ply": stability_plys[0] if stability_plys else "",
            "stability_plys": stability_plys,
            "churn_ply": churn_plys[0] if churn_plys else "",
            "churn_plys": churn_plys,
        },
    }


def scan_existing_measurement_families(search_roots: Sequence[Path], *, max_depth: int = 8) -> dict:
    patterns = {
        "ghost_plate": {"out_ghost_plate_summary.json"},
        "entropy_campaign": {"out_godel_fits_entropy_campaign_summary.json"},
        "fits_invariance_study": {"fits_invariance_study_summary.json"},
        "invariance_sculpture": {
            "out_invariance_sculpture_stability.ply",
            "out_invariance_sculpture_churn.ply",
        },
    }
    catalog: dict[str, list[dict[str, str]]] = {key: [] for key in patterns}
    for root in search_roots:
        if not root.exists():
            continue
        base = root.resolve(strict=False)
        for current_root, dir_names, file_names in os.walk(base):
            current = Path(current_root)
            try:
                rel_parts = current.relative_to(base).parts
            except ValueError:
                rel_parts = ()
            if len(rel_parts) > max_depth:
                dir_names[:] = []
                continue
            dir_names[:] = sorted(dir_names)
            for file_name in sorted(file_names):
                lowered = file_name.lower()
                file_path = current / file_name
                for family, family_patterns in patterns.items():
                    if lowered in family_patterns or any(lowered.endswith(suffix.lower()) for suffix in family_patterns if "*" not in suffix):
                        catalog[family].append(
                            {
                                "path": str(file_path),
                                "parent": str(file_path.parent),
                                "name": file_name,
                            }
                        )
    return catalog


def _timestamp_slug() -> str:
    return datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S")


def run_measurement_lane(
    *,
    explicit_fits: Sequence[Path] | None = None,
    search_roots: Sequence[Path] | None = None,
    external_results_roots: Sequence[Path] | None = None,
    max_depth: int = 8,
    max_datasets: int = 0,
    out_root: Path = DEFAULT_OUT_ROOT,
    run_stamp: str | None = None,
    nine_root: Path = DEFAULT_NINE_ROOT,
    prepare_script: Path | None = None,
    runner_script: Path | None = None,
    profile: str = "entry",
    out_json: Path | None = None,
) -> dict:
    prepare_path = (prepare_script or (nine_root / "scripts" / "utilities" / "prepare_fits_frame_groups.py")).resolve()
    runner_path = (runner_script or (nine_root / "scripts" / "batch_runners" / "run_reality_toolkit_unknown.py")).resolve()
    stamp = run_stamp or _timestamp_slug()
    run_root = out_root / stamp
    manifest_dir = run_root / "manifest"
    receipts_dir = run_root / "receipts"
    run_root.mkdir(parents=True, exist_ok=True)

    selection_entries = build_selection_manifest(
        explicit_paths=explicit_fits,
        search_roots=search_roots,
        max_depth=max_depth,
        max_datasets=max_datasets,
    )
    selection_manifest = {
        "version": 1,
        "run_stamp": stamp,
        "nine_root": str(nine_root),
        "prepare_script": str(prepare_path),
        "runner_script": str(runner_path),
        "entries": selection_entries,
    }
    selection_manifest_path = manifest_dir / "rtk_v3_selection_manifest.json"
    _write_json(selection_manifest_path, selection_manifest)

    dataset_receipts: list[dict] = []
    harvest_datasets: list[dict] = []
    status = "ok"
    py = sys.executable
    extra_env = build_tool_env(nine_root)

    for entry in selection_entries:
        dataset_id = str(entry["dataset_id"])
        dataset_root = run_root / "datasets" / dataset_id
        source_dir = dataset_root / "source"
        prepared_dir = dataset_root / "prepared"
        rtk_unknown_dir = dataset_root / "rtk_unknown"
        logs_dir = dataset_root / "logs"
        _copy_fits_to_source(entry, source_dir)

        prepare_command = [
            py,
            str(prepare_path),
            "--data-dir",
            str(source_dir),
            "--out-root",
            str(prepared_dir),
            "--min-frames",
            "1",
        ]
        prepare_receipt = _run_command(
            prepare_command,
            cwd=nine_root,
            log_path=logs_dir / "prepare.log",
            extra_env=extra_env,
        )
        normalized_groups_csv = prepared_dir / "out_fits_groups_emitted_normalized.csv"
        emitted_csv = prepared_dir / "out_fits_groups_emitted.csv"
        if prepare_receipt["ok"] and emitted_csv.exists():
            normalized_groups_csv = _normalize_groups_csv(emitted_csv, dataset_id, entry)
        else:
            status = "partial_failed_prepare"

        runner_receipt: dict | None = None
        if prepare_receipt["ok"] and normalized_groups_csv.exists():
            runner_command = [
                py,
                str(runner_path),
                "--groups-csv",
                str(normalized_groups_csv),
                "--out-root",
                str(rtk_unknown_dir),
                "--blind-mode",
                "on",
                "--profile",
                str(profile),
                "--prefer",
                "fits",
            ]
            runner_receipt = _run_command(
                runner_command,
                cwd=nine_root,
                log_path=logs_dir / "runner.log",
                extra_env=extra_env,
            )
            if not runner_receipt["ok"]:
                status = "partial_failed_runner"
        harvest = harvest_dataset_outputs(dataset_id, rtk_unknown_dir)
        harvest["source_entry"] = entry
        harvest["prepare_ok"] = bool(prepare_receipt["ok"])
        harvest["runner_ok"] = bool(runner_receipt and runner_receipt["ok"])
        harvest_datasets.append(harvest)
        dataset_receipts.append(
            {
                "dataset_id": dataset_id,
                "source_path": str(entry["path"]),
                "prepare": {
                    "ok": bool(prepare_receipt["ok"]),
                    "log_path": prepare_receipt["log_path"],
                    "groups_csv": str(normalized_groups_csv) if normalized_groups_csv.exists() else "",
                },
                "runner": {
                    "ok": bool(runner_receipt and runner_receipt["ok"]),
                    "log_path": str((runner_receipt or {}).get("log_path") or ""),
                    "out_root": str(rtk_unknown_dir),
                },
            }
        )

    harvest_summary = {
        "version": 1,
        "run_stamp": stamp,
        "datasets": harvest_datasets,
    }
    harvest_summary_path = manifest_dir / "rtk_v3_harvest_summary.json"
    _write_json(harvest_summary_path, harvest_summary)

    external_catalog = scan_existing_measurement_families(
        external_results_roots or (DEFAULT_EXTERNAL_RESULTS_ROOT,),
        max_depth=max_depth,
    )
    external_catalog_path = manifest_dir / "rtk_v3_external_family_catalog.json"
    _write_json(external_catalog_path, external_catalog)

    run_manifest = {
        "version": 1,
        "run_stamp": stamp,
        "run_root": str(run_root),
        "profile": str(profile),
        "prepare_script": str(prepare_path),
        "runner_script": str(runner_path),
        "selection_manifest_json": str(selection_manifest_path),
        "harvest_summary_json": str(harvest_summary_path),
        "external_family_catalog_json": str(external_catalog_path),
        "dataset_count": len(selection_entries),
        "datasets_with_prepare_failures": sum(1 for item in dataset_receipts if not item["prepare"]["ok"]),
        "datasets_with_runner_failures": sum(1 for item in dataset_receipts if item["prepare"]["ok"] and not item["runner"]["ok"]),
    }
    run_manifest_path = manifest_dir / "rtk_v3_run_manifest.json"
    _write_json(run_manifest_path, run_manifest)

    receipt = {
        "ok": status == "ok",
        "status": status,
        "run_stamp": stamp,
        "run_root": str(run_root),
        "selection_manifest_json": str(selection_manifest_path),
        "run_manifest_json": str(run_manifest_path),
        "harvest_summary_json": str(harvest_summary_path),
        "external_family_catalog_json": str(external_catalog_path),
        "dataset_receipts": dataset_receipts,
    }
    receipt_path = out_json or (receipts_dir / "rtk_v3_receipt.json")
    _write_json(receipt_path, receipt)

    summary_lines = [
        "Explaino RTK v3 measurement lane",
        f"status: {status}",
        f"run_root: {run_root}",
        f"datasets_selected: {len(selection_entries)}",
        f"selection_manifest: {selection_manifest_path}",
        f"harvest_summary: {harvest_summary_path}",
        f"external_family_catalog: {external_catalog_path}",
    ]
    _write_text(manifest_dir / "rtk_v3_summary.txt", "\n".join(summary_lines) + "\n")
    return receipt


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Stage FITS through Reality Toolkit v3 and harvest normalized external measurement artifacts.")
    parser.add_argument("--explicit-fits", type=Path, nargs="*", default=[])
    parser.add_argument("--search-root", type=Path, action="append", default=[])
    parser.add_argument("--external-results-root", type=Path, action="append", default=[])
    parser.add_argument("--max-depth", type=int, default=8)
    parser.add_argument("--max-datasets", type=int, default=0)
    parser.add_argument("--out-root", type=Path, default=DEFAULT_OUT_ROOT)
    parser.add_argument("--run-stamp", default=None)
    parser.add_argument("--nine-root", type=Path, default=DEFAULT_NINE_ROOT)
    parser.add_argument("--prepare-script", type=Path, default=None)
    parser.add_argument("--runner-script", type=Path, default=None)
    parser.add_argument("--profile", choices=["entry", "full"], default="entry")
    parser.add_argument("--out-json", type=Path, default=None)
    args = parser.parse_args(argv)

    search_roots = list(args.search_root)
    if not search_roots and not args.explicit_fits:
        search_roots = [DEFAULT_SALTS_ROOT]
    external_roots = list(args.external_results_root) or [DEFAULT_EXTERNAL_RESULTS_ROOT]

    receipt = run_measurement_lane(
        explicit_fits=args.explicit_fits,
        search_roots=search_roots,
        external_results_roots=external_roots,
        max_depth=args.max_depth,
        max_datasets=args.max_datasets,
        out_root=args.out_root,
        run_stamp=args.run_stamp,
        nine_root=args.nine_root,
        prepare_script=args.prepare_script,
        runner_script=args.runner_script,
        profile=args.profile,
        out_json=args.out_json,
    )
    print(json.dumps(receipt, indent=2))
    return 0 if receipt["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
