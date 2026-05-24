from __future__ import annotations

import subprocess
from pathlib import Path

from runtime_harness import RUNTIME_DIR, active_runtime_exe, run_headless_capture, runtime_automation_lock


def _diagnostics_archive_dirs() -> set[Path]:
    diagnostics_dir = RUNTIME_DIR / "diagnostics"
    if not diagnostics_dir.exists():
        return set()
    return {
        path.resolve()
        for path in diagnostics_dir.iterdir()
        if path.is_dir() and path.name != "last"
    }


def _assert_bundle_files(bundle_dir: Path) -> None:
    assert (bundle_dir / "state.json").exists(), f"missing state.json in {bundle_dir}"
    frame_path = bundle_dir / "frame.bmp"
    assert frame_path.exists(), f"missing frame.bmp in {bundle_dir}"
    assert frame_path.read_bytes()[:2] == b"BM", f"frame.bmp is not a BMP in {bundle_dir}"


def test_capture_diagnostic_default_writes_unique_archives_and_last_mirror() -> None:
    exe_path = active_runtime_exe()

    with runtime_automation_lock():
        before = _diagnostics_archive_dirs()
        run_headless_capture(
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "multibrot",
            "--width",
            "64",
            "--height",
            "48",
        )
        run_headless_capture(
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            "multibrot",
            "--width",
            "64",
            "--height",
            "48",
        )
        new_archives = sorted(_diagnostics_archive_dirs() - before)

    assert len(new_archives) >= 2, "default diagnostic capture must not only overwrite diagnostics/last"
    for bundle_dir in new_archives[-2:]:
        _assert_bundle_files(bundle_dir)
    _assert_bundle_files(RUNTIME_DIR / "diagnostics" / "last")


def test_capture_diagnostic_explicit_out_dir_writes_requested_bundle(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    output_dir = tmp_path / "explicit_diagnostic_bundle"

    with runtime_automation_lock():
        result = subprocess.run(
            [
                str(exe_path),
                "--capture-diagnostic",
                "--diagnostics-out-dir",
                str(output_dir),
                "--fractal-type",
                "julia",
                "--width",
                "64",
                "--height",
                "48",
            ],
            cwd=str(RUNTIME_DIR),
            text=True,
            capture_output=True,
            check=False,
        )

    assert result.returncode == 0, result.stderr or result.stdout
    _assert_bundle_files(output_dir)


def test_capture_diagnostic_bad_explicit_out_dir_fails_usefully(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    blocked_parent = tmp_path / "not_a_directory"
    blocked_parent.write_text("blocked", encoding="utf-8")
    output_dir = blocked_parent / "child"

    with runtime_automation_lock():
        result = subprocess.run(
            [
                str(exe_path),
                "--capture-diagnostic",
                "--diagnostics-out-dir",
                str(output_dir),
                "--width",
                "16",
                "--height",
                "16",
            ],
            cwd=str(RUNTIME_DIR),
            text=True,
            capture_output=True,
            check=False,
        )

    assert result.returncode != 0
    assert "diagnostics directory" in (result.stderr + result.stdout).lower()
