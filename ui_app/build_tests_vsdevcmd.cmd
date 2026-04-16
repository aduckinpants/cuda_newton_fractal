@echo off
setlocal

call "%~dp0..\tools\call_vsdevcmd.cmd"
if errorlevel 1 exit /b 1

cd /d C:\code\cuda_newton_fractal_clone\ui_app

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set TESTROOT=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\build_tests

if not exist "%TESTROOT%" mkdir "%TESTROOT%"

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\cli_args.cpp .\tests\test_cli_args.cpp ^
  /Fe:"%TESTROOT%\test_cli_args.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\cli_args.cpp .\src\viewer_cli.cpp .\tests\test_viewer_cli.cpp ^
  /Fe:"%TESTROOT%\test_viewer_cli.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\flashlight_probe.cpp .\src\lens_sdf.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_flashlight_probe.cpp .\tests\test_flashlight_render_stub.cpp .\tests\test_flashlight_capture_stub.cpp ^
  /Fe:"%TESTROOT%\test_flashlight_probe.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\cli_args.cpp .\src\viewer_cli.cpp .\src\viewer_state_init.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_viewer_state_init.cpp ^
  /Fe:"%TESTROOT%\test_viewer_state_init.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\safe_mode_schema.cpp .\src\schema_startup_policy.cpp .\src\viewer_schema_load.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_viewer_schema_load.cpp ^
  /Fe:"%TESTROOT%\test_viewer_schema_load.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\tests\test_json_min.cpp ^
  /Fe:"%TESTROOT%\test_json_min.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\viewport_interaction.cpp .\tests\test_viewport_interaction.cpp ^
  /Fe:"%TESTROOT%\test_viewport_interaction.exe"
if errorlevel 1 exit /b 1

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
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\sample_tier_resolver.cpp .\src\explaino_variant_benchmark.cpp .\tests\test_explaino_variant_benchmark.cpp ^
  /Fe:"%TESTROOT%\test_explaino_variant_benchmark.exe"
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

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_schema_binding.cpp ^
  /Fe:"%TESTROOT%\test_schema_binding.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\finding_capture_state.cpp .\tests\test_finding_capture_state.cpp ^
  /Fe:"%TESTROOT%\test_finding_capture_state.exe"
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
  .\tests\test_polynomial_eval_real_coeffs.cpp ^
  /Fe:"%TESTROOT%\test_polynomial_eval_real_coeffs.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_basin_coloring.cpp ^
  /Fe:"%TESTROOT%\test_basin_coloring.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_escape_time_direct_formulas.cpp ^
  /Fe:"%TESTROOT%\test_escape_time_direct_formulas.exe"
if errorlevel 1 exit /b 1

set GENERIC_SAMPLE_CORE_OBJ=%TESTROOT%\generic_sample_core_runner.obj
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  -c .\src\generic_sample_core.cu -o "%GENERIC_SAMPLE_CORE_OBJ%"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_fractal_probe.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_fractal_probe.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_fractal_sample_pipeline.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_fractal_sample_pipeline.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_fractal_probe_coverage.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_fractal_probe_coverage.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp .\src\safe_mode_schema.cpp .\src\schema_startup_policy.cpp .\src\viewer_schema_load.cpp .\src\explaino_sidecar_model.cpp .\src\explaino_sidecar_measurement.cpp .\src\explaino_sidecar_budget.cpp .\src\explaino_sidecar_lens.cpp .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_action.cpp .\src\explaino_sidecar_trace.cpp .\src\explaino_sidecar_controller.cpp .\src\explaino_sidecar_divergence.cpp .\src\explaino_sidecar_completeness.cpp .\src\explaino_sidecar_window.cpp .\src\explaino_exploration_advisor.cpp .\src\flashlight_probe.cpp .\src\lens_sdf.cpp .\src\headless_modes.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_headless_modes.cpp .\tests\test_flashlight_render_stub.cpp .\tests\test_flashlight_capture_stub.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_headless_modes.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
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
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_function_descriptor.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_function_descriptor.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\function_descriptor.cpp .\src\explaino_sidecar_model.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_explaino_sidecar_model.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_model.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\explaino_sidecar_budget.cpp .\tests\test_explaino_sidecar_budget.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_budget.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\explaino_sidecar_lens.cpp .\tests\test_explaino_sidecar_lens.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_lens.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\function_descriptor.cpp .\src\explaino_sidecar_model.cpp .\src\explaino_sidecar_measurement.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_explaino_sidecar_measurement.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_measurement.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_action.cpp .\tests\test_explaino_sidecar_action.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_action.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\explaino_sidecar_energy.cpp .\tests\test_explaino_sidecar_energy.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_energy.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_trace.cpp .\tests\test_explaino_sidecar_trace.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_trace.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\explaino_sidecar_controller.cpp .\src\explaino_sidecar_refresh.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_explaino_sidecar_controller.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_controller.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\explaino_sidecar_divergence.cpp .\tests\test_explaino_sidecar_divergence.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_divergence.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\explaino_sidecar_completeness.cpp .\tests\test_explaino_sidecar_completeness.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_completeness.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\function_descriptor.cpp .\src\explaino_sidecar_model.cpp .\src\explaino_sidecar_measurement.cpp .\src\explaino_sidecar_budget.cpp .\src\explaino_sidecar_lens.cpp .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_action.cpp .\src\explaino_sidecar_trace.cpp .\src\explaino_sidecar_controller.cpp .\src\explaino_sidecar_divergence.cpp .\src\explaino_sidecar_completeness.cpp .\src\explaino_sidecar_window.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_explaino_sidecar_window.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_window.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp .\src\safe_mode_schema.cpp .\src\schema_startup_policy.cpp .\src\viewer_schema_load.cpp .\src\explaino_sidecar_model.cpp .\src\explaino_sidecar_measurement.cpp .\src\explaino_sidecar_budget.cpp .\src\explaino_sidecar_lens.cpp .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_action.cpp .\src\explaino_sidecar_trace.cpp .\src\explaino_sidecar_controller.cpp .\src\explaino_sidecar_divergence.cpp .\src\explaino_sidecar_completeness.cpp .\src\explaino_sidecar_window.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_explaino_sidecar_schema_contract.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_schema_contract.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp .\src\safe_mode_schema.cpp .\src\schema_startup_policy.cpp .\src\viewer_schema_load.cpp .\src\explaino_sidecar_model.cpp .\src\explaino_sidecar_measurement.cpp .\src\explaino_sidecar_budget.cpp .\src\explaino_sidecar_lens.cpp .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_action.cpp .\src\explaino_sidecar_trace.cpp .\src\explaino_sidecar_controller.cpp .\src\explaino_sidecar_divergence.cpp .\src\explaino_sidecar_completeness.cpp .\src\explaino_sidecar_window.cpp .\src\explaino_exploration_advisor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_explaino_exploration_advisor.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_explaino_exploration_advisor.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

"%TESTROOT%\test_cli_args.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_viewer_cli.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_viewer_state_init.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_viewer_schema_load.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_json_min.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_viewport_interaction.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_seed.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_seed_dynamics.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_budget.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_lens.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_action.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_energy.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_trace.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_controller.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_divergence.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_completeness.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_measurement.exe"
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

"%TESTROOT%\test_explaino_variant_benchmark.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_diagnostics_state_io.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_finding_state_actions.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_schema_startup_policy.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_schema_binding.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_finding_capture_state.exe"
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

"%TESTROOT%\test_polynomial_eval_real_coeffs.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_basin_coloring.exe"
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

"%TESTROOT%\test_explaino_sidecar_model.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_window.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_sidecar_schema_contract.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_exploration_advisor.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\sample_tier_resolver.cpp .\tests\test_sample_tier_resolver.cpp ^
  /Fe:"%TESTROOT%\test_sample_tier_resolver.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\fractal_renderer.cu .\src\fractal_sample_core.cu .\src\sample_tier_resolver.cpp .\tests\test_escape_time_sample_tier.cu ^
  -o "%TESTROOT%\test_escape_time_sample_tier.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\fractal_renderer.cu .\src\fractal_sample_core.cu .\src\sample_tier_resolver.cpp .\tests\test_newton_basin_regression.cu ^
  -o "%TESTROOT%\test_newton_basin_regression.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\fractal_renderer.cu .\src\fractal_sample_core.cu .\src\sample_tier_resolver.cpp .\tests\test_fractal_sample_device.cu ^
  -o "%TESTROOT%\test_fractal_sample_device.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\fractal_renderer.cu .\src\fractal_sample_core.cu .\src\sample_tier_resolver.cpp .\tests\test_fractal_sample_kernel.cu ^
  -o "%TESTROOT%\test_fractal_sample_kernel.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\fractal_renderer.cu .\src\fractal_sample_core.cu .\src\sample_tier_resolver.cpp .\tests\test_fractal_sample_equivalence.cu ^
  -o "%TESTROOT%\test_fractal_sample_equivalence.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\fractal_renderer.cu .\src\fractal_sample_core.cu .\src\sample_tier_resolver.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_explaino_zero_axis_equivalence.cu ^
  -o "%TESTROOT%\test_explaino_zero_axis_equivalence.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_sample_tier_resolver.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_escape_time_sample_tier.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_newton_basin_regression.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_sample_device.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_sample_kernel.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_sample_equivalence.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_zero_axis_equivalence.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_nova_iteration.cpp ^
  /Fe:"%TESTROOT%\test_nova_iteration.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_nova_iteration.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_joy_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_joy_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_joy_continuity.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\\src ^
  .\\tests\\test_explaino_fold_continuity.cpp ^
  /Fe:"%TESTROOT%\\test_explaino_fold_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\\test_explaino_fold_continuity.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\\src ^
  .\\tests\\test_explaino_bell_continuity.cpp ^
  /Fe:"%TESTROOT%\\test_explaino_bell_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\\test_explaino_bell_continuity.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_ripple_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_ripple_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_ripple_continuity.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_splice_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_splice_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_splice_continuity.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_vortex_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_vortex_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_vortex_continuity.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_tension_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_tension_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_tension_continuity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_sample_pipeline.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_probe_coverage.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\param_anim_dynamics.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_param_anim_dynamics.cpp ^
  /Fe:"%TESTROOT%\test_param_anim_dynamics.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_headless_modes.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_param_anim_dynamics.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\param_anim_dynamics.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_param_anim_generic.cpp ^
  /Fe:"%TESTROOT%\test_param_anim_generic.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_param_anim_generic.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_generic_function_math.cpp ^
  /Fe:"%TESTROOT%\test_generic_function_math.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_generic_function_math.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_generic_function_eval.cu ^
  -o "%TESTROOT%\test_generic_function_eval.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_generic_function_eval.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\generic_sample_core.cu .\tests\test_generic_sample_core.cu ^
  -o "%TESTROOT%\test_generic_sample_core.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_generic_sample_core.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\generic_sample_core.cu .\tests\test_generic_sample_parity.cu ^
  -o "%TESTROOT%\test_generic_sample_parity.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_generic_sample_parity.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_generic_function_parser.cpp ^
  /Fe:"%TESTROOT%\test_generic_function_parser.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_generic_function_parser.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_callable_engine_adversarial.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_callable_engine_adversarial.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

"%TESTROOT%\test_callable_engine_adversarial.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_generic_probe.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_generic_probe.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

"%TESTROOT%\test_generic_probe.exe"
if errorlevel 1 exit /b 1

echo All helper tests passed.
exit /b 0
