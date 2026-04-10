from __future__ import annotations

import argparse
import importlib.util
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
MAINLINE_PHASED_PLAN_HOOK = Path(r"C:\code\salticid-cuda\tools\hook_require_phased_plan_sync.py")
PHASED_PLAN_SUFFIX = "_PHASED_PLAN.md"


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
            print("viewer_host_assert_phased_plan_sync: no phased plan changes detected")
            return 0

        failures = [message for path in paths if (message := _validate_plan_file(path))]
        if failures:
            sys.stderr.write("viewer_host_assert_phased_plan_sync: FAILED\n")
            for failure in failures:
                sys.stderr.write(f"- {failure}\n")
            return 2

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