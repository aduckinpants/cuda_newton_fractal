from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path
from typing import Iterable, Sequence

import flashlight_bridge_runner as flashlight_bridge


DEFAULT_RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
DEFAULT_MAINLINE_REPO = Path(r"C:\code\salticid-cuda")


def _read_json(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def _write_json(path: Path, payload: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


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


def _iter_checkpoint_fits(search_root: Path, max_depth: int) -> Iterable[Path]:
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
            if file_name.lower() == "checkpoint_final.fits":
                yield current / file_name


def discover_checkpoint_fits(
    *,
    explicit_paths: Sequence[Path] | None = None,
    search_roots: Sequence[Path] | None = None,
    max_depth: int = 8,
) -> list[Path]:
    discovered: list[Path] = []
    for path in explicit_paths or ():
        candidate = Path(path)
        if candidate.exists():
            discovered.append(candidate.resolve())
    for root in search_roots or ():
        discovered.extend(_iter_checkpoint_fits(Path(root), max_depth))
    return _unique_paths(discovered)


def inspect_checkpoint_fits(path: Path) -> dict:
    try:
        from astropy.io import fits
    except ImportError as exc:  # pragma: no cover - environment-specific guard
        raise RuntimeError("astropy is required to inspect checkpoint FITS files") from exc

    fits_path = Path(path).resolve()
    with fits.open(fits_path) as hdul:
        hdu_names = [str(hdu.name or "PRIMARY") for hdu in hdul]
        primary = hdul[0].data if hdul else None
        primary_shape = list(primary.shape) if primary is not None else []
        primary_dtype = str(primary.dtype) if primary is not None else None
    return {
        "path": str(fits_path),
        "size_bytes": fits_path.stat().st_size,
        "sha256": _sha256_file(fits_path),
        "hdu_names": hdu_names,
        "primary_shape": primary_shape,
        "primary_dtype": primary_dtype,
        "has_op_state": "OP_STATE" in hdu_names,
    }


def build_corpus_manifest(
    *,
    explicit_paths: Sequence[Path] | None = None,
    search_roots: Sequence[Path] | None = None,
    max_depth: int = 8,
) -> list[dict]:
    return [
        inspect_checkpoint_fits(path)
        for path in discover_checkpoint_fits(
            explicit_paths=explicit_paths,
            search_roots=search_roots,
            max_depth=max_depth,
        )
    ]


def _copy_artifact(src: Path, dst: Path) -> Path:
    if src.resolve(strict=False) != dst.resolve(strict=False):
        shutil.copyfile(src, dst)
    return dst


def build_trace_artifacts(report_path: Path, diagnostics_dir: Path) -> dict[str, str]:
    artifacts = flashlight_bridge._build_trace_artifacts(report_path, diagnostics_dir)
    renames = {
        "trace_frame_bmp": diagnostics_dir / "runtime_walk_trace_frame.bmp",
        "trace_overlay_bmp": diagnostics_dir / "runtime_walk_trace_overlay.bmp",
        "trace_stl": diagnostics_dir / "runtime_walk_trace.stl",
        "trace_obj": diagnostics_dir / "runtime_walk_trace.obj",
        "trace_csv": diagnostics_dir / "runtime_walk_trace.csv",
    }
    rewritten: dict[str, str] = {
        "reference_frame_bmp": artifacts["reference_frame_bmp"],
    }
    for key, dst in renames.items():
        src = Path(artifacts[key])
        rewritten[key] = str(_copy_artifact(src, dst))
    return rewritten


def _resolve_output_dir(runtime_dir: Path, request_path: Path, request: dict) -> Path:
    out_dir = Path(request["out_dir"])
    if out_dir.is_absolute():
        return out_dir
    return (runtime_dir / out_dir).resolve()


def _build_parity_summary(report_path: Path, comparison_fits_path: Path | None) -> dict | None:
    if comparison_fits_path is None or not comparison_fits_path.exists():
        return None
    report = _read_json(report_path)
    fits_info = inspect_checkpoint_fits(comparison_fits_path)
    summary = report.get("summary", {})
    return {
        "comparison_fits": fits_info,
        "structural_parity": {
            "report_kind": report.get("kind"),
            "tick_count": int(summary.get("tick_count", 0)),
            "report_has_trace": bool(report.get("trace")),
            "fits_has_op_state": bool(fits_info.get("has_op_state")),
            "fits_primary_shape": fits_info.get("primary_shape"),
        },
    }


def run_mainline_checkpoint_continuation(
    checkpoint_in: Path,
    checkpoint_out: Path,
    *,
    extra_args: Sequence[str] | None = None,
    mainline_repo: Path = DEFAULT_MAINLINE_REPO,
) -> dict:
    script = mainline_repo / "scripts" / "run_salticid.py"
    if not script.exists():
        raise RuntimeError(f"missing mainline checkpoint runner: {script}")
    command = [
        sys.executable,
        str(script),
        "--checkpoint-in",
        str(checkpoint_in),
        "--checkpoint-out",
        str(checkpoint_out),
    ]
    command.extend(extra_args or ())
    result = subprocess.run(
        command,
        cwd=str(mainline_repo),
        text=True,
        capture_output=True,
        check=False,
    )
    return {
        "ok": result.returncode == 0,
        "returncode": result.returncode,
        "stdout": result.stdout,
        "stderr": result.stderr,
        "checkpoint_out": str(checkpoint_out),
    }


def run_runtime_walk(
    runtime_dir: Path,
    request_path: Path,
    *,
    explicit_fits: Sequence[Path] | None = None,
    search_roots: Sequence[Path] | None = None,
    max_depth: int = 8,
    out_json: Path | None = None,
) -> dict:
    runtime_dir = runtime_dir.resolve()
    request_path = request_path.resolve()
    request = _read_json(request_path)
    output_dir = _resolve_output_dir(runtime_dir, request_path, request)
    report_path = output_dir / "runtime_walk_report.json"
    manifest_path = output_dir / "runtime_walk_branch_manifest.json"
    status_path = out_json or (output_dir / "runtime_walk_status.json")
    exe_path = flashlight_bridge._active_runtime_exe(runtime_dir)

    before_report = report_path.stat().st_mtime if report_path.exists() else 0.0
    before_manifest = manifest_path.stat().st_mtime if manifest_path.exists() else 0.0
    time.sleep(1.05)
    result = subprocess.run(
        [
            str(exe_path),
            "--runtime-walk-request-json",
            str(request_path),
        ],
        cwd=str(runtime_dir),
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        payload = {
            "ok": False,
            "request_json": str(request_path),
            "error": result.stderr or result.stdout or "runtime walk execution failed",
        }
        _write_json(status_path, payload)
        return payload

    if not report_path.exists() or report_path.stat().st_mtime <= before_report:
        payload = {
            "ok": False,
            "request_json": str(request_path),
            "error": "runtime walk did not regenerate runtime_walk_report.json",
        }
        _write_json(status_path, payload)
        return payload
    if not manifest_path.exists() or manifest_path.stat().st_mtime <= before_manifest:
        payload = {
            "ok": False,
            "request_json": str(request_path),
            "error": "runtime walk did not regenerate runtime_walk_branch_manifest.json",
        }
        _write_json(status_path, payload)
        return payload

    artifacts = build_trace_artifacts(report_path, output_dir)
    comparison_fits_path = Path(request["comparison_fits"]).resolve() if request.get("comparison_fits") else None
    manifest_entries = build_corpus_manifest(
        explicit_paths=[
            *(explicit_fits or ()),
            *(() if comparison_fits_path is None else (comparison_fits_path,)),
        ],
        search_roots=search_roots,
        max_depth=max_depth,
    )
    manifest_json = output_dir / "runtime_walk_corpus_manifest.json"
    parity_summary = _build_parity_summary(report_path, comparison_fits_path)
    parity_summary_json = output_dir / "runtime_walk_parity_summary.json"

    if manifest_entries:
        _write_json(
            manifest_json,
            {
                "version": 1,
                "entries": manifest_entries,
            },
        )
    if parity_summary:
        _write_json(parity_summary_json, parity_summary)

    payload = {
        "ok": True,
        "request_json": str(request_path),
        "report_json": str(report_path),
        "branch_manifest_json": str(manifest_path),
        "artifacts": artifacts,
    }
    if manifest_entries:
        payload["corpus_manifest_json"] = str(manifest_json)
    if parity_summary:
        payload["parity_summary_json"] = str(parity_summary_json)
    _write_json(status_path, payload)
    return payload


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Run the Explaino saved-runtime walker and emit trace/corpus artifacts.")
    parser.add_argument("--runtime-dir", type=Path, default=DEFAULT_RUNTIME_DIR)
    parser.add_argument("--request-json", type=Path, required=True)
    parser.add_argument("--explicit-fits", type=Path, nargs="*", default=[])
    parser.add_argument("--search-root", type=Path, action="append", default=[])
    parser.add_argument("--max-depth", type=int, default=8)
    parser.add_argument("--out-json", type=Path, default=None)
    args = parser.parse_args(argv)

    payload = run_runtime_walk(
        args.runtime_dir,
        args.request_json,
        explicit_fits=args.explicit_fits,
        search_roots=args.search_root,
        max_depth=args.max_depth,
        out_json=args.out_json,
    )
    print(json.dumps(payload, indent=2))
    return 0 if payload.get("ok") else 1


if __name__ == "__main__":
    raise SystemExit(main())
