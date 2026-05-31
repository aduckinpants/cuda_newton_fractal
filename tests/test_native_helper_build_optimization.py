from __future__ import annotations

from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
BUILD_SCRIPT = REPO_ROOT / "ui_app" / "build_tests_vsdevcmd.cmd"


def _script_text() -> str:
    return BUILD_SCRIPT.read_text(encoding="utf-8")


def test_focused_helper_runs_multiple_targets_in_one_setup() -> None:
    script = _script_text()

    assert ":focused_arg_loop" in script
    assert 'call :dispatch_focused "%~1"' in script
    assert "shift" in script
    assert ":dispatch_focused" in script
    assert " & exit /b %ERRORLEVEL%" not in script


def test_full_helper_reuses_heavy_cuda_objects_instead_of_recompiling_per_test() -> None:
    script = _script_text()

    repeated_fractal_combo = (
        r".\src\fractal_renderer.cu .\src\fractal_sample_core.cu "
        r".\src\sample_tier_resolver.cpp .\tests"
    )
    repeated_generic_combo = r".\src\generic_sample_core.cu .\tests"

    assert ":build_fractal_cuda_common_objects" in script
    assert ":build_generic_sample_core_object" in script
    assert repeated_fractal_combo not in script
    assert repeated_generic_combo not in script


def test_cuda_arch_flags_are_centralized_for_native_helper() -> None:
    script = _script_text()

    assert "set CUDA_GENCODE_FLAGS=" in script
    assert script.count("-gencode=arch=compute_86,code=sm_86") == 1
    assert script.count("-gencode=arch=compute_120,code=sm_120") == 1
    assert script.count("-gencode=arch=compute_121,code=sm_121") == 1


def test_validator_requires_current_optimized_full_helper_measurement() -> None:
    validator = (REPO_ROOT / "tools" / "viewer_host_validate_native_helper_build.py").read_text(encoding="utf-8")

    assert "native_helper_full_optimized_measured.json" in validator
    assert "optimized_full_helper_improved" in validator
    assert "performance_summary" in validator


def test_validator_requires_grouped_one_shell_proofs() -> None:
    validator = (REPO_ROOT / "tools" / "viewer_host_validate_native_helper_build.py").read_text(encoding="utf-8")

    assert "native_helper_multi_target_non_cuda_after_audit.json" in validator
    assert "native_helper_fractal_cuda_shared_objects.json" in validator
    assert "native_helper_generic_cuda_shared_object.json" in validator
    assert "grouped_proofs_success" in validator
    assert "native_helper_unknown_target_negative.json" in validator
    assert "negative_proofs_rejected" in validator
