from __future__ import annotations

import argparse
from dataclasses import dataclass
from datetime import datetime
import json
from pathlib import Path
import re
import shutil
from typing import Any, Iterable


GENERATED_DIAGNOSTIC_DIR_RE = re.compile(r"^\d{8}_\d{6}_\d{3}__diagnostic_\d+$")
BULK_COMPONENTS = {"diagnostics", "findings"}

PUBLISH_INCLUDE_DIRS: tuple[tuple[str, str], ...] = (
    ("artifacts/validation", "artifacts/validation"),
    ("artifacts/logs", "artifacts/logs"),
    ("artifacts/hooks/viewer_host_validation_receipts", "artifacts/hooks/viewer_host_validation_receipts"),
    ("artifacts/hooks/viewer_host_contract_proof_receipts", "artifacts/hooks/viewer_host_contract_proof_receipts"),
    ("artifacts/hooks/viewer_host_rearward_review", "artifacts/hooks/viewer_host_rearward_review"),
    ("ui_app/build", "ui_app_build"),
    ("ui_app/build_tests", "ui_app_build_tests"),
    ("ui_app/ui", "ui_app_runtime_ui"),
)

PUBLISH_INCLUDE_FILES: tuple[tuple[str, str], ...] = (
    ("newton.ppm", "root_outputs/newton.ppm"),
    ("newton_driver_updated.ppm", "root_outputs/newton_driver_updated.ppm"),
    ("newton_fractal.exe", "root_outputs/newton_fractal.exe"),
    ("newton_fractal_fresh.exe", "root_outputs/newton_fractal_fresh.exe"),
    ("imgui.ini", "root_outputs/imgui.ini"),
    ("_build_tmp.cmd", "root_outputs/_build_tmp.cmd"),
    ("ui_app/fractal_ui.exe", "ui_app_runtime/fractal_ui.exe"),
    ("ui_app/imgui.ini", "ui_app_runtime/imgui.ini"),
)

EXCLUDED_ROOTS: tuple[str, ...] = (
    "artifacts/diagnostics",
    "artifacts/findings",
    "diagnostics",
    "findings",
    "runtime/diagnostics",
    "runtime/findings",
    "ui/diagnostics",
)


@dataclass(frozen=True)
class CleanupCandidate:
    path: Path
    bytes_total: int
    files_total: int


def _safe_label(label: str) -> str:
    cleaned = "".join(ch if ch.isalnum() or ch in ("-", "_") else "_" for ch in label.strip())
    return cleaned.strip("_") or "artifact_publish"


def _directory_totals(path: Path) -> tuple[int, int]:
    files = 0
    bytes_total = 0
    if not path.exists():
        return 0, 0
    for file_path in path.rglob("*"):
        if not file_path.is_file():
            continue
        try:
            stat = file_path.stat()
        except OSError:
            continue
        files += 1
        bytes_total += int(stat.st_size)
    return files, bytes_total


def _has_bulk_component(path: Path) -> bool:
    return any(part.lower() in BULK_COMPONENTS for part in path.parts)


def _add_file(entries: list[dict[str, Any]], source: Path, destination: Path, repo_root: Path) -> None:
    if not source.exists() or not source.is_file():
        return
    relative_source = source.resolve().relative_to(repo_root.resolve())
    if _has_bulk_component(relative_source):
        return
    entries.append(
        {
            "source": str(source),
            "destination": str(destination),
            "bytes": int(source.stat().st_size),
        }
    )


def _iter_files(root: Path) -> Iterable[Path]:
    if not root.exists() or not root.is_dir():
        return []
    return (path for path in root.rglob("*") if path.is_file())


def _collect_excluded_roots(repo_root: Path) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for rel in EXCLUDED_ROOTS:
        path = repo_root / rel
        if not path.exists():
            continue
        files, bytes_total = _directory_totals(path)
        rows.append(
            {
                "path": str(path),
                "repo_relative": rel,
                "files": files,
                "bytes": bytes_total,
            }
        )
    return rows


def build_publish_report(
    repo_root: Path,
    publish_root: Path,
    *,
    label: str = "artifact_publish",
    timestamp: str | None = None,
) -> dict[str, Any]:
    repo_root = repo_root.resolve()
    publish_root = publish_root.resolve()
    stamp = timestamp or datetime.now().strftime("%Y-%m-%d_%H%M%S")
    publish_dir = publish_root / repo_root.name / "published" / f"{stamp}_{_safe_label(label)}"
    entries: list[dict[str, Any]] = []

    for source_rel, destination_rel in PUBLISH_INCLUDE_DIRS:
        source_root = repo_root / source_rel
        destination_root = publish_dir / destination_rel
        for source in _iter_files(source_root):
            relative_source = source.relative_to(source_root)
            repo_relative = source.relative_to(repo_root)
            if _has_bulk_component(repo_relative):
                continue
            entries.append(
                {
                    "source": str(source),
                    "destination": str(destination_root / relative_source),
                    "bytes": int(source.stat().st_size),
                }
            )

    for source_rel, destination_rel in PUBLISH_INCLUDE_FILES:
        _add_file(entries, repo_root / source_rel, publish_dir / destination_rel, repo_root)

    for source in sorted((repo_root / "ui_app").glob("*.obj")) if (repo_root / "ui_app").exists() else []:
        _add_file(entries, source, publish_dir / "ui_app_runtime" / source.name, repo_root)

    return {
        "schema_version": "viewer_host.deploy_disk_pressure.v1",
        "repo_root": str(repo_root),
        "publish_root": str(publish_root),
        "publish_dir": str(publish_dir),
        "timestamp": stamp,
        "label": _safe_label(label),
        "entries": entries,
        "entry_count": len(entries),
        "bytes_total": sum(int(entry["bytes"]) for entry in entries),
        "excluded_roots": _collect_excluded_roots(repo_root),
    }


def publish_from_report(report: dict[str, Any], *, execute: bool) -> dict[str, Any]:
    copied: list[str] = []
    for entry in report["entries"]:
        source = Path(entry["source"])
        destination = Path(entry["destination"])
        if execute:
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, destination)
        copied.append(str(destination))

    manifest_path = Path(report["publish_dir"]) / "publish_manifest.json"
    if execute:
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return {"publish_dir": report["publish_dir"], "copied": copied, "manifest": str(manifest_path), "executed": execute}


def discover_runtime_diagnostics_cleanup_candidates(runtime_dir: Path) -> list[CleanupCandidate]:
    diagnostics_dir = runtime_dir / "diagnostics"
    if not diagnostics_dir.exists() or not diagnostics_dir.is_dir():
        return []
    candidates: list[CleanupCandidate] = []
    for child in sorted(diagnostics_dir.iterdir(), key=lambda p: p.name):
        if not child.is_dir():
            continue
        if not GENERATED_DIAGNOSTIC_DIR_RE.match(child.name):
            continue
        files, bytes_total = _directory_totals(child)
        candidates.append(CleanupCandidate(path=child, bytes_total=bytes_total, files_total=files))
    return candidates


def execute_cleanup_candidates(candidates: list[CleanupCandidate], *, execute: bool) -> dict[str, Any]:
    deleted: list[str] = []
    for candidate in candidates:
        path = candidate.path
        if not GENERATED_DIAGNOSTIC_DIR_RE.match(path.name):
            raise ValueError(f"refusing to delete non-generated diagnostic directory: {path}")
        if execute:
            shutil.rmtree(path)
            deleted.append(str(path))
    return {
        "executed": execute,
        "candidate_count": len(candidates),
        "candidate_bytes": sum(candidate.bytes_total for candidate in candidates),
        "deleted": deleted,
        "candidates": [
            {"path": str(candidate.path), "bytes": candidate.bytes_total, "files": candidate.files_total}
            for candidate in candidates
        ],
    }


def _write_json(path: Path | None, payload: dict[str, Any]) -> None:
    if path is None:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def _add_common_publish_args(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    parser.add_argument("--publish-root", type=Path, default=Path(r"D:\salt-fractal"))
    parser.add_argument("--label", default="artifact_publish")
    parser.add_argument("--timestamp")
    parser.add_argument("--out-json", type=Path)


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description="Build filtered deploy manifests and clean generated diagnostics safely.")
    sub = ap.add_subparsers(dest="command", required=True)

    manifest_parser = sub.add_parser("manifest", help="Write or print the filtered publish manifest without copying files")
    _add_common_publish_args(manifest_parser)

    publish_parser = sub.add_parser("publish", help="Copy the filtered publish manifest entries")
    _add_common_publish_args(publish_parser)
    publish_parser.add_argument("--execute", action="store_true", help="Actually copy files; omitted means dry-run")

    cleanup_parser = sub.add_parser("cleanup-diagnostics", help="Delete generated timestamped runtime diagnostic archives")
    cleanup_parser.add_argument("--runtime-dir", type=Path, default=Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime"))
    cleanup_parser.add_argument("--execute", action="store_true", help="Actually delete candidates; omitted means dry-run")
    cleanup_parser.add_argument("--out-json", type=Path)

    args = ap.parse_args(argv)
    if args.command == "manifest":
        report = build_publish_report(args.repo_root, args.publish_root, label=args.label, timestamp=args.timestamp)
        _write_json(args.out_json, report)
        print(json.dumps(report, indent=2, sort_keys=True))
        return 0
    if args.command == "publish":
        report = build_publish_report(args.repo_root, args.publish_root, label=args.label, timestamp=args.timestamp)
        result = publish_from_report(report, execute=args.execute)
        payload = {**report, "publish_result": result}
        _write_json(args.out_json, payload)
        print(report["publish_dir"])
        return 0
    if args.command == "cleanup-diagnostics":
        candidates = discover_runtime_diagnostics_cleanup_candidates(args.runtime_dir)
        result = execute_cleanup_candidates(candidates, execute=args.execute)
        _write_json(args.out_json, result)
        summary = {
            "executed": result["executed"],
            "candidate_count": result["candidate_count"],
            "candidate_bytes": result["candidate_bytes"],
            "deleted_count": len(result["deleted"]),
        }
        print(json.dumps(summary, indent=2, sort_keys=True))
        return 0
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
