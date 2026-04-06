from __future__ import annotations

import json
from pathlib import Path
from types import SimpleNamespace

from tools.reality_toolkit.fractal_explorer import probe_client as probe_client_mod


def test_resolve_probe_runtime_path_prefers_active_runtime(tmp_path: Path, monkeypatch) -> None:
    publish_root = tmp_path / "publish"
    repo_root = tmp_path / "cuda_newton_fractal_clone"
    repo_root.mkdir()
    monkeypatch.setenv("SALT_FRACTAL_ROOT", str(publish_root))

    runtime_dir = publish_root / repo_root.name / "runtime"
    runtime_dir.mkdir(parents=True)
    active_runtime = runtime_dir / "fractal_ui_headless.exe"
    active_runtime.write_bytes(b"")
    (runtime_dir / "fractal_ui_active.txt").write_text("fractal_ui_headless.exe\n", encoding="utf-8")
    (runtime_dir / "fractal_ui.exe").write_bytes(b"")

    resolved = probe_client_mod.resolve_probe_runtime_path(repo_root)

    assert resolved == active_runtime


def test_run_sample_request_uses_stdin_stdout_cli(tmp_path: Path, monkeypatch) -> None:
    publish_root = tmp_path / "publish"
    repo_root = tmp_path / "cuda_newton_fractal_clone"
    repo_root.mkdir()
    monkeypatch.setenv("SALT_FRACTAL_ROOT", str(publish_root))

    runtime_dir = publish_root / repo_root.name / "runtime"
    runtime_dir.mkdir(parents=True)
    exe_path = runtime_dir / "fractal_ui.exe"
    exe_path.write_bytes(b"")

    calls: list[dict[str, object]] = []

    def fake_run(command: list[str], **kwargs: object) -> SimpleNamespace:
        calls.append({"command": command, **kwargs})
        return SimpleNamespace(
            returncode=0,
            stdout=json.dumps({"ok": True, "request_id": "probe-123", "summary": {"sample_count": 1}}),
            stderr="",
        )

    monkeypatch.setattr(probe_client_mod.subprocess, "run", fake_run)

    response = probe_client_mod.run_sample_request(
        repo_root,
        {"request_id": "probe-123", "request_version": 1, "mode": "point_set", "points": [{"x": 0.0, "y": 0.0}]},
        exe_path=exe_path,
        timeout_seconds=12.5,
    )

    assert response["ok"] is True
    assert len(calls) == 1
    assert calls[0]["command"] == [str(exe_path), "--sample-request-stdin", "--sample-response-stdout"]
    assert calls[0]["cwd"] == str(runtime_dir)
    assert json.loads(str(calls[0]["input"]))["request_id"] == "probe-123"
