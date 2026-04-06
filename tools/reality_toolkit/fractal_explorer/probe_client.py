from __future__ import annotations

import json
import subprocess
from pathlib import Path
from typing import Any, Mapping

from .paths import runtime_root


def _runtime_command(exe_path: Path, *args: str) -> list[str]:
    command = [str(exe_path), *args]
    if exe_path.suffix.lower() in {".cmd", ".bat"}:
        return ["cmd.exe", "/d", "/c", *command]
    return command


def resolve_probe_runtime_path(repo_root: Path) -> Path:
    runtime_dir = runtime_root(repo_root)
    active_runtime_file = runtime_dir / "fractal_ui_active.txt"
    if active_runtime_file.exists():
        active_name = active_runtime_file.read_text(encoding="utf-8").strip()
        if active_name:
            active_path = runtime_dir / active_name
            if active_path.exists():
                return active_path

    for candidate in (runtime_dir / "fractal_ui.exe", runtime_dir / "fractal_ui.cmd"):
        if candidate.exists():
            return candidate

    raise FileNotFoundError(f"No active fractal runtime found under {runtime_dir}")


def describe_functions(
    repo_root: Path,
    *,
    exe_path: Path | None = None,
    timeout_seconds: float = 30.0,
) -> dict[str, Any]:
    resolved_exe = exe_path or resolve_probe_runtime_path(repo_root)
    result = subprocess.run(
        _runtime_command(resolved_exe, "--describe-functions"),
        cwd=str(runtime_root(repo_root)),
        text=True,
        capture_output=True,
        timeout=timeout_seconds,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or result.stdout.strip() or f"describe-functions failed with exit code {result.returncode}")
    if not result.stdout.strip():
        raise RuntimeError("describe-functions returned no JSON")
    try:
        parsed = json.loads(result.stdout)
    except json.JSONDecodeError as exc:
        raise RuntimeError(f"describe-functions returned invalid JSON: {exc}") from exc
    if not isinstance(parsed, dict):
        raise RuntimeError("describe-functions response must be a JSON object")
    return parsed


def run_sample_request(
    repo_root: Path,
    request: Mapping[str, object],
    *,
    exe_path: Path | None = None,
    timeout_seconds: float = 180.0,
) -> dict[str, Any]:
    resolved_exe = exe_path or resolve_probe_runtime_path(repo_root)
    result = subprocess.run(
        _runtime_command(resolved_exe, "--sample-request-stdin", "--sample-response-stdout"),
        cwd=str(runtime_root(repo_root)),
        text=True,
        capture_output=True,
        timeout=timeout_seconds,
        check=False,
        input=json.dumps(request),
    )

    response_text = result.stdout.strip()
    if not response_text:
        raise RuntimeError(result.stderr.strip() or f"sample request returned no JSON (exit {result.returncode})")

    try:
        parsed = json.loads(response_text)
    except json.JSONDecodeError as exc:
        raise RuntimeError(f"sample request returned invalid JSON: {exc}") from exc

    if not isinstance(parsed, dict):
        raise RuntimeError("sample request response must be a JSON object")

    if result.returncode != 0 or not parsed.get("ok", False):
        raise RuntimeError(str(parsed.get("error") or result.stderr.strip() or f"sample request failed with exit code {result.returncode}"))

    return parsed
