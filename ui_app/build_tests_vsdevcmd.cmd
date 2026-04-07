@echo off
setlocal

call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
if errorlevel 1 exit /b 1

where cl >NUL 2>NUL
if errorlevel 1 (
  echo cl.exe not found after VsDevCmd
  exit /b 1
)

cd /d C:\code\cuda_newton_fractal_clone\ui_app

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set TESTROOT=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\build_tests

if not exist "%TESTROOT%" mkdir "%TESTROOT%"

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\tests\test_explaino_seed.cpp ^
  /Fe:"%TESTROOT%\test_explaino_seed.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\explaino_seed.cpp .\src\explaino_seed_dynamics.cpp .\tests\test_explaino_seed_dynamics.cpp ^
  /Fe:"%TESTROOT%\test_explaino_seed_dynamics.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_seed_tween_continuity.cpp ^
  /Fe:"%TESTROOT%\test_seed_tween_continuity.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_fractal_derived_fields.cpp ^
  /Fe:"%TESTROOT%\test_fractal_derived_fields.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\tests\test_runtime_reset.cpp ^
  /Fe:"%TESTROOT%\test_runtime_reset.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\lens_sdf.cpp .\tests\test_lens_sdf.cpp ^
  /Fe:"%TESTROOT%\test_lens_sdf.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\viewer_shutdown.cpp .\tests\test_viewer_shutdown.cpp ^
  /Fe:"%TESTROOT%\test_viewer_shutdown.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\sweep_player.cpp .\src\viewer_sweep.cpp .\tests\test_viewer_sweep.cpp ^
  /Fe:"%TESTROOT%\test_viewer_sweep.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\sweep_player.cpp .\tests\test_sweep_player.cpp ^
  /Fe:"%TESTROOT%\test_sweep_player.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\tests\test_view_hp_sync.cpp ^
  /Fe:"%TESTROOT%\test_view_hp_sync.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\explaino_seed.cpp .\src\diagnostics_state_io.cpp .\tests\test_diagnostics_state_io.cpp ^
  /Fe:"%TESTROOT%\test_diagnostics_state_io.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\diagnostics_state_io.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\finding_state_actions.cpp .\tests\test_finding_state_actions.cpp ^
  /Fe:"%TESTROOT%\test_finding_state_actions.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\schema_startup_policy.cpp .\tests\test_schema_startup_policy.cpp ^
  /Fe:"%TESTROOT%\test_schema_startup_policy.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\finding_archive_actions.cpp .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_finding_archive_actions.cpp ^
  /Fe:"%TESTROOT%\test_finding_archive_actions.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\render_capture_guard.cpp .\tests\test_render_capture_guard.cpp ^
  /Fe:"%TESTROOT%\test_render_capture_guard.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\viewer_render_pacing.cpp .\tests\test_viewer_render_pacing.cpp ^
  /Fe:"%TESTROOT%\test_viewer_render_pacing.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_runtime_validation.cpp ^
  /Fe:"%TESTROOT%\test_fractal_runtime_validation.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_escape_time_specialized_formulas.cpp ^
  /Fe:"%TESTROOT%\test_escape_time_specialized_formulas.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_perturbation_reference_orbit.cpp ^
  /Fe:"%TESTROOT%\test_perturbation_reference_orbit.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_escape_time_coloring.cpp ^
  /Fe:"%TESTROOT%\test_escape_time_coloring.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_collatz_formulas.cpp ^
  /Fe:"%TESTROOT%\test_explaino_collatz_formulas.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_escape_time_direct_formulas.cpp ^
  /Fe:"%TESTROOT%\test_escape_time_direct_formulas.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_fractal_probe.cpp ^
  /Fe:"%TESTROOT%\test_fractal_probe.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_family_rules.cpp ^
  /Fe:"%TESTROOT%\test_fractal_family_rules.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\safe_mode_schema.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_ui_schema.cpp ^
  /Fe:"%TESTROOT%\test_ui_schema.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_function_descriptor.cpp ^
  /Fe:"%TESTROOT%\test_function_descriptor.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_seed.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_seed_dynamics.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_derived_fields.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_seed_tween_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_runtime_reset.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_lens_sdf.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_viewer_shutdown.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_viewer_sweep.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_sweep_player.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_view_hp_sync.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_diagnostics_state_io.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_finding_state_actions.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_schema_startup_policy.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_finding_archive_actions.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_render_capture_guard.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_viewer_render_pacing.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_runtime_validation.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_escape_time_specialized_formulas.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_perturbation_reference_orbit.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_escape_time_coloring.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_collatz_formulas.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_escape_time_direct_formulas.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_probe.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_family_rules.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_ui_schema.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_function_descriptor.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\sample_tier_resolver.cpp .\tests\test_sample_tier_resolver.cpp ^
  /Fe:"%TESTROOT%\test_sample_tier_resolver.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\fractal_renderer.cu .\src\sample_tier_resolver.cpp .\tests\test_escape_time_sample_tier.cu ^
  -o "%TESTROOT%\test_escape_time_sample_tier.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_sample_tier_resolver.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_escape_time_sample_tier.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_nova_iteration.cpp ^
  /Fe:"%TESTROOT%\test_nova_iteration.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_nova_iteration.exe"
if errorlevel 1 exit /b 1

echo All helper tests passed.
exit /b 0