from __future__ import annotations

from pathlib import Path
import subprocess
import sys

REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.viewer_host_deploy_disk_pressure import (  # noqa: E402
    build_publish_report,
    discover_runtime_diagnostics_cleanup_candidates,
    execute_cleanup_candidates,
)


def _write_bytes(path: Path, size: int = 8) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"x" * size)


def test_publish_report_excludes_bulk_diagnostics_and_findings(tmp_path: Path) -> None:
    repo = tmp_path / "repo"
    publish_root = tmp_path / "publish"

    _write_bytes(repo / "artifacts" / "validation" / "receipt.json")
    _write_bytes(repo / "artifacts" / "logs" / "build.log")
    _write_bytes(repo / "artifacts" / "diagnostics" / "huge.bin")
    _write_bytes(repo / "artifacts" / "findings" / "huge.bin")
    _write_bytes(repo / "ui" / "diagnostics" / "huge.bmp")
    _write_bytes(repo / "findings" / "manual_capture" / "state.json")
    _write_bytes(repo / "runtime" / "diagnostics" / "20260525_120214_686__diagnostic_36768" / "frame.bmp")
    _write_bytes(repo / "ui_app" / "build" / "main.obj")
    _write_bytes(repo / "ui_app" / "fractal_ui.exe")
    _write_bytes(repo / "ui_app" / "imgui.ini")

    report = build_publish_report(repo, publish_root, label="unit")
    sources = [Path(entry["source"]) for entry in report["entries"]]
    source_text = [source.as_posix() for source in sources]

    assert any(path.endswith("artifacts/validation/receipt.json") for path in source_text)
    assert any(path.endswith("artifacts/logs/build.log") for path in source_text)
    assert any(path.endswith("ui_app/build/main.obj") for path in source_text)
    assert all("/diagnostics/" not in path for path in source_text)
    assert all("/findings/" not in path for path in source_text)
    assert all(not path.endswith("/findings/manual_capture/state.json") for path in source_text)
    assert report["excluded_roots"], "bulk roots should be reported instead of silently ignored"


def test_cleanup_candidates_preserve_last_named_dirs_and_findings(tmp_path: Path) -> None:
    runtime_dir = tmp_path / "runtime"
    generated = runtime_dir / "diagnostics" / "20260525_120214_686__diagnostic_36768"
    last = runtime_dir / "diagnostics" / "last"
    named = runtime_dir / "diagnostics" / "runtime_walk_sessions"
    finding = runtime_dir / "findings" / "manual_capture" / "state.json"

    _write_bytes(generated / "frame.bmp", 11)
    _write_bytes(last / "frame.bmp", 13)
    _write_bytes(named / "trace.bin", 17)
    _write_bytes(finding, 19)

    candidates = discover_runtime_diagnostics_cleanup_candidates(runtime_dir)

    assert [candidate.path for candidate in candidates] == [generated]
    assert candidates[0].bytes_total == 11

    dry_run = execute_cleanup_candidates(candidates, execute=False)
    assert dry_run["deleted"] == []
    assert generated.exists()
    assert last.exists()
    assert named.exists()
    assert finding.exists()

    executed = execute_cleanup_candidates(candidates, execute=True)
    assert executed["deleted"] == [str(generated)]
    assert not generated.exists()
    assert last.exists()
    assert named.exists()
    assert finding.exists()


def test_publish_repo_artifacts_whatif_uses_filtered_helper(tmp_path: Path) -> None:
    script = REPO_ROOT / "tools" / "publish_repo_artifacts.ps1"
    result = subprocess.run(
        [
            "powershell",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-File",
            str(script),
            "-PublishRoot",
            str(tmp_path / "published"),
            "-Label",
            "unit_smoke",
            "-WhatIf",
        ],
        cwd=REPO_ROOT,
        text=True,
        capture_output=True,
        check=False,
    )

    assert result.returncode == 0, result.stderr or result.stdout
    assert "published" in result.stdout
    assert "WHATIF move" not in result.stdout
    assert "Move-Item" not in result.stdout
