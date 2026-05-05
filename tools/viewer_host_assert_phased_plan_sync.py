from __future__ import annotations

import argparse
import contextlib
import importlib
import importlib.util
import json
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
MAINLINE_PHASED_PLAN_HOOK = Path(r"C:\code\salticid-cuda\tools\hook_require_phased_plan_sync.py")
PHASED_PLAN_SUFFIX = "_PHASED_PLAN.md"
PLAN_SYNC_VALIDATION_ARTIFACT = Path("artifacts/validation/viewer_host_assert_phased_plan_sync.json")


@contextlib.contextmanager
def _temporary_sys_path(paths: list[Path]):
    original = list(sys.path)
    additions = [str(path) for path in paths if str(path) not in sys.path]
    try:
        sys.path[:0] = additions
        yield
    finally:
        sys.path[:] = original


@contextlib.contextmanager
def _temporary_mainline_tools_namespace():
    saved = {
        name: module
        for name, module in sys.modules.items()
        if name == "tools" or name.startswith("tools.")
    }
    try:
        for name in saved:
            sys.modules.pop(name, None)
        importlib.import_module("tools")
        yield
    finally:
        for name in list(sys.modules):
            if name == "tools" or name.startswith("tools."):
                sys.modules.pop(name, None)
        sys.modules.update(saved)


def _repo_relative(path: Path) -> str:
    try:
        return path.resolve().relative_to(REPO_ROOT).as_posix()
    except Exception:
        return path.as_posix()


def _all_plan_paths() -> list[Path]:
    return sorted(
        path
        for path in REPO_ROOT.rglob(f"*{PHASED_PLAN_SUFFIX}")
        if ".git" not in path.parts and path.is_file()
    )


def _capture_git(*args: str) -> str:
    proc = subprocess.run(
        ["git", *args],
        cwd=str(REPO_ROOT),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    if proc.returncode != 0:
        detail = (proc.stderr or proc.stdout or "git command failed").strip()
        raise RuntimeError(detail)
    return (proc.stdout or "").strip()


def _resolve_paths(raw_paths: list[str]) -> list[Path]:
    resolved: list[Path] = []
    missing: list[str] = []
    for raw in raw_paths:
        path = Path(raw)
        if not path.is_absolute():
            path = REPO_ROOT / path
        if not path.exists():
            missing.append(raw)
            continue
        resolved.append(path)
    if missing:
        raise FileNotFoundError(
            "viewer_host_assert_phased_plan_sync: missing path(s): " + ", ".join(missing)
        )
    return resolved


def _load_mainline_plan_guard():
    if not MAINLINE_PHASED_PLAN_HOOK.exists():
        raise FileNotFoundError(f"Mainline phased-plan hook not found: {MAINLINE_PHASED_PLAN_HOOK}")
    spec = importlib.util.spec_from_file_location("mainline_hook_require_phased_plan_sync", MAINLINE_PHASED_PLAN_HOOK)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Unable to load mainline phased-plan hook from: {MAINLINE_PHASED_PLAN_HOOK}")
    module = importlib.util.module_from_spec(spec)
    mainline_repo_root = MAINLINE_PHASED_PLAN_HOOK.parent.parent
    mainline_tools_dir = MAINLINE_PHASED_PLAN_HOOK.parent
    with _temporary_sys_path([mainline_repo_root, mainline_tools_dir]):
        with _temporary_mainline_tools_namespace():
            spec.loader.exec_module(module)
    module.REPO_ROOT = REPO_ROOT
    return module


def _dirty_plan_paths() -> list[Path]:
    guard = _load_mainline_plan_guard()
    return guard._dirty_plan_paths()


def _head_commit_plan_paths() -> list[Path]:
    guard = _load_mainline_plan_guard()
    return guard._head_commit_plan_paths()


def _default_target_paths() -> tuple[list[Path], str]:
    dirty = _dirty_plan_paths()
    if dirty:
        return dirty, "dirty"
    head = _head_commit_plan_paths()
    if head:
        return head, "head"
    return [], "none"


def validate_plan_text(text: str, *, display_path: str) -> str | None:
    guard = _load_mainline_plan_guard()
    path = Path(display_path)
    if not path.is_absolute():
        path = REPO_ROOT / path
    return guard.validate_plan_text(path, text)


def _validate_plan_file(path: Path) -> str | None:
    guard = _load_mainline_plan_guard()
    return guard._validate_plan_file(path)


def _write_validation_artifact(*, paths: list[Path], source: str) -> None:
    artifact_path = (REPO_ROOT / PLAN_SYNC_VALIDATION_ARTIFACT).resolve()
    artifact_path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "ok": True,
        "source": source,
        "plan_count": len(paths),
        "checked_plans": [_repo_relative(path) for path in paths],
    }
    artifact_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Assert phased-plan checklist/current-phase sync for this repo")
    parser.add_argument("paths", nargs="*", help="Explicit phased-plan paths to check")
    parser.add_argument("--all", action="store_true", help="Check all phased plans in the repo")
    args = parser.parse_args(argv)

    if args.all and args.paths:
        parser.error("use either explicit paths or --all, not both")

    try:
        if args.paths:
            paths = _resolve_paths(args.paths)
            source = "explicit"
        elif args.all:
            paths = _all_plan_paths()
            source = "all"
        else:
            paths, source = _default_target_paths()

        if not paths:
            _write_validation_artifact(paths=[], source=source)
            print("viewer_host_assert_phased_plan_sync: no phased plan changes detected")
            return 0

        failures = [message for path in paths if (message := _validate_plan_file(path))]
        if failures:
            sys.stderr.write("viewer_host_assert_phased_plan_sync: FAILED\n")
            for failure in failures:
                sys.stderr.write(f"- {failure}\n")
            return 2

        _write_validation_artifact(paths=paths, source=source)
        print(f"viewer_host_assert_phased_plan_sync: OK ({len(paths)} plan(s), source={source})")
        for path in paths:
            print(f"  {_repo_relative(path)}")
        return 0
    except FileNotFoundError as exc:
        sys.stderr.write(f"{exc}\n")
        return 3
    except Exception as exc:
        sys.stderr.write(f"viewer_host_assert_phased_plan_sync: ERROR: {exc}\n")
        return 3


if __name__ == "__main__":
    raise SystemExit(main())