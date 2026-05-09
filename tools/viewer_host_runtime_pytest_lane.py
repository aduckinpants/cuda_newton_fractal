from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
DEFAULT_TEST_FILES = [
    "tests/test_explaino_runtime_walk_tool.py",
    "tests/test_flashlight_bridge_runner.py",
    "tests/test_fractal_runtime_explaino_runtime_walk.py",
    "tests/test_fractal_runtime_probe_cli.py",
    "tests/test_fractal_runtime_flashlight_bridge.py",
    "tests/test_fractal_runtime_flashlight_probe.py",
    "tests/test_fractal_runtime_session.py",
    "tests/test_function_descriptor_cli.py",
    "tests/test_generic_probe_cli.py",
]


def extract_pytest_counts(text: str) -> dict[str, int]:
    counts: dict[str, int] = {}
    patterns = {
        "passed": r"(\d+)\s+passed\b",
        "failed": r"(\d+)\s+failed\b",
        "skipped": r"(\d+)\s+skipped\b",
        "errors": r"(\d+)\s+errors?\b",
        "xfailed": r"(\d+)\s+xfailed\b",
        "xpassed": r"(\d+)\s+xpassed\b",
    }
    for label, pattern in patterns.items():
        matches = re.findall(pattern, text)
        if matches:
            counts[label] = int(matches[-1])
    return counts


def active_runtime_exe(
    *,
    runtime_dir: Path = RUNTIME_DIR,
    active_runtime_file: Path = ACTIVE_RUNTIME_FILE,
) -> Path:
    if sys.platform != "win32":
        raise RuntimeError("runtime probe/session pytest lane is Windows-only")
    if not active_runtime_file.exists():
        raise RuntimeError(f"missing active runtime metadata: {active_runtime_file}")

    active_name = active_runtime_file.read_text(encoding="utf-8").strip()
    if not active_name:
        raise RuntimeError(f"empty active runtime metadata: {active_runtime_file}")

    exe_path = runtime_dir / active_name
    if not exe_path.exists():
        raise RuntimeError(f"active runtime missing: {exe_path}")
    return exe_path


def build_pytest_command(*, python_executable: str, test_files: list[str], pytest_args: list[str] | None = None) -> list[str]:
    extra_args = list(pytest_args or [])
    return [python_executable, "-m", "pytest", *test_files, *extra_args, "-q"]


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Run the runtime probe/session pytest lane with explicit published-runtime preflight and non-ambiguous pass criteria.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--python", default=sys.executable, help="Python executable to use for pytest")
    parser.add_argument("test_files", nargs="*", help="Optional override list of pytest files; defaults to the canonical runtime probe/session lane")
    args, pytest_args = parser.parse_known_args(argv)

    test_files = list(args.test_files) or list(DEFAULT_TEST_FILES)

    try:
        exe_path = active_runtime_exe()
    except RuntimeError as exc:
        print(f"viewer_host_runtime_pytest_lane: {exc}")
        return 2

    print(f"viewer_host_runtime_pytest_lane: active_runtime={exe_path}")
    print("viewer_host_runtime_pytest_lane: tests=" + ", ".join(test_files))
    if pytest_args:
        print("viewer_host_runtime_pytest_lane: pytest_args=" + " ".join(pytest_args))

    command = build_pytest_command(python_executable=args.python, test_files=test_files, pytest_args=pytest_args)
    proc = subprocess.run(
        command,
        cwd=str(REPO_ROOT),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=False,
    )

    output = proc.stdout or ""
    if output:
        sys.stdout.write(output)
        if not output.endswith("\n"):
            sys.stdout.write("\n")

    counts = extract_pytest_counts(output)
    print(
        "viewer_host_runtime_pytest_lane: summary="
        f"passed={counts.get('passed', 0)} "
        f"failed={counts.get('failed', 0)} "
        f"skipped={counts.get('skipped', 0)} "
        f"errors={counts.get('errors', 0)}"
    )

    if proc.returncode != 0:
        return int(proc.returncode)

    if counts.get("passed", 0) < 1:
        print(
            "viewer_host_runtime_pytest_lane: ambiguous runtime pytest result: zero tests passed; "
            "likely skipped due to missing runtime preconditions or an invalid published runtime"
        )
        return 2

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
