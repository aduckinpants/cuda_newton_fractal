from __future__ import annotations

import os
from datetime import datetime
from pathlib import Path


def publish_root() -> Path:
    env_value = os.environ.get("SALT_FRACTAL_ROOT")
    if env_value:
        return Path(env_value)
    return Path("D:/salt-fractal")


def repo_publish_root(repo_root: Path) -> Path:
    return publish_root() / repo_root.name


def runtime_root(repo_root: Path) -> Path:
    return repo_publish_root(repo_root) / "runtime"


def runtime_launcher_path(repo_root: Path) -> Path:
    return runtime_root(repo_root) / "fractal_ui.cmd"


def diagnostics_last_dir(repo_root: Path) -> Path:
    return runtime_root(repo_root) / "diagnostics" / "last"


def findings_root(repo_root: Path) -> Path:
    return repo_publish_root(repo_root) / "findings"


def default_seed_sweep_out_dir(repo_root: Path) -> Path:
    stamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
    return repo_publish_root(repo_root) / "artifacts" / f"seed_sweep_{stamp}"