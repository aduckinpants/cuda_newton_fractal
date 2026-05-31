@echo off
setlocal

call "%~dp0..\tools\call_vsdevcmd.cmd"
if errorlevel 1 exit /b 1

cd /d C:\code\cuda_newton_fractal_clone\ui_app

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set TESTROOT=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\build_tests
set OBJROOT=%TESTROOT%\obj
set PDBROOT=%TESTROOT%\pdb
set CUDA_GENCODE_FLAGS=-gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121

if not exist "%TESTROOT%" mkdir "%TESTROOT%"

for /L %%R in (1,1,30) do (
  if not exist "%OBJROOT%" goto objroot_clean
  rmdir /s /q "%OBJROOT%" 2>nul
  if not exist "%OBJROOT%" goto objroot_clean
  ping -n 2 127.0.0.1 >nul
)
echo [build_tests_vsdevcmd] Failed to remove "%OBJROOT%" after cleanup retries
exit /b 1
:objroot_clean

for /L %%R in (1,1,30) do (
  if not exist "%PDBROOT%" goto pdbroot_clean
  rmdir /s /q "%PDBROOT%" 2>nul
  if not exist "%PDBROOT%" goto pdbroot_clean
  ping -n 2 127.0.0.1 >nul
)
echo [build_tests_vsdevcmd] Failed to remove "%PDBROOT%" after cleanup retries
exit /b 1
:pdbroot_clean

mkdir "%OBJROOT%"
if errorlevel 1 exit /b 1
mkdir "%PDBROOT%"
if errorlevel 1 exit /b 1

set CL=/FS /Fo"%OBJROOT%\\" /Fd"%PDBROOT%\\build_tests.pdb"
goto after_test_helpers
:run_test
%*
set "RUN_TEST_RC=%ERRORLEVEL%"
if not "%RUN_TEST_RC%"=="0" (
  echo [build_tests_vsdevcmd] Test command failed with exit %RUN_TEST_RC%: %*
  exit /b 1
)
exit /b 0

:build_fractal_cuda_common_objects
if defined FRACTAL_CUDA_COMMON_READY exit /b 0
set FRACTAL_RENDERER_OBJ=%OBJROOT%\fractal_renderer_common.obj
set FRACTAL_SAMPLE_CORE_OBJ=%OBJROOT%\fractal_sample_core_common.obj
set SAMPLE_TIER_RESOLVER_OBJ=%OBJROOT%\sample_tier_resolver_common.obj
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  -c .\src\fractal_renderer.cu -o "%FRACTAL_RENDERER_OBJ%"
if errorlevel 1 exit /b 1
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  -c .\src\fractal_sample_core.cu -o "%FRACTAL_SAMPLE_CORE_OBJ%"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  /c .\src\sample_tier_resolver.cpp /Fo"%SAMPLE_TIER_RESOLVER_OBJ%"
if errorlevel 1 exit /b 1
set FRACTAL_CUDA_COMMON_READY=1
exit /b 0

:build_generic_sample_core_object
if defined GENERIC_SAMPLE_CORE_READY exit /b 0
set GENERIC_SAMPLE_CORE_OBJ=%OBJROOT%\generic_sample_core_common.obj
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  -c .\src\generic_sample_core.cu -o "%GENERIC_SAMPLE_CORE_OBJ%"
if errorlevel 1 exit /b 1
set GENERIC_SAMPLE_CORE_READY=1
exit /b 0
:after_test_helpers

if not "%~1"=="" goto focused_arg_loop
goto full_build_start

:focused_arg_loop
if "%~1"=="" exit /b 0
call :dispatch_focused "%~1"
if errorlevel 1 exit /b 1
shift
goto focused_arg_loop

:dispatch_focused
set FOCUSED_TEST=%~1
if /I "%FOCUSED_TEST%"=="advanced_color_grading_red" call :focused_advanced_color_grading_red & exit /b
if /I "%FOCUSED_TEST%"=="advanced_color_grading_owner" call :focused_advanced_color_grading_owner & exit /b
if /I "%FOCUSED_TEST%"=="serializer_owner_fast" call :focused_serializer_owner_fast & exit /b
if /I "%FOCUSED_TEST%"=="test_viewer_ui_automation_report" call :focused_test_viewer_ui_automation_report & exit /b
if /I "%FOCUSED_TEST%"=="test_viewer_cli" call :focused_test_viewer_cli & exit /b
if /I "%FOCUSED_TEST%"=="test_viewer_state_init" call :focused_test_viewer_state_init & exit /b
if /I "%FOCUSED_TEST%"=="test_flashlight_probe" call :focused_test_flashlight_probe & exit /b
if /I "%FOCUSED_TEST%"=="test_diagnostics_state_io" call :focused_test_diagnostics_state_io & exit /b
if /I "%FOCUSED_TEST%"=="test_diagnostics_capture" call :focused_test_diagnostics_capture & exit /b
if /I "%FOCUSED_TEST%"=="test_lens_sdf" call :focused_test_lens_sdf & exit /b
if /I "%FOCUSED_TEST%"=="test_lens_sdf_cuda" call :focused_test_lens_sdf_cuda & exit /b
if /I "%FOCUSED_TEST%"=="test_finding_archive_actions" call :focused_test_finding_archive_actions & exit /b
if /I "%FOCUSED_TEST%"=="test_finding_state_actions" call :focused_test_finding_state_actions & exit /b
if /I "%FOCUSED_TEST%"=="test_viewer_render_pacing" call :focused_test_viewer_render_pacing & exit /b
if /I "%FOCUSED_TEST%"=="test_viewport_interaction" call :focused_test_viewport_interaction & exit /b
if /I "%FOCUSED_TEST%"=="test_sample_tier_resolver" call :focused_test_sample_tier_resolver & exit /b
if /I "%FOCUSED_TEST%"=="test_fractal_renderer" call :focused_test_fractal_renderer & exit /b
if /I "%FOCUSED_TEST%"=="test_fractal_sample_kernel" call :focused_test_fractal_sample_kernel & exit /b
if /I "%FOCUSED_TEST%"=="test_runtime_walk_headless" call :focused_test_runtime_walk_headless & exit /b
if /I "%FOCUSED_TEST%"=="test_ui_schema" call :focused_test_ui_schema & exit /b
if /I "%FOCUSED_TEST%"=="test_safe_mode_schema" call :focused_test_safe_mode_schema & exit /b
if /I "%FOCUSED_TEST%"=="test_color_pipeline_core" call :focused_test_color_pipeline_core & exit /b
if /I "%FOCUSED_TEST%"=="test_color_pipeline_window" call :focused_test_color_pipeline_window & exit /b
if /I "%FOCUSED_TEST%"=="test_color_pipeline_sdf_field_groups" call :focused_test_color_pipeline_sdf_field_groups & exit /b
if /I "%FOCUSED_TEST%"=="test_color_pipeline_sdf_postprocess" call :focused_test_color_pipeline_sdf_postprocess & exit /b
if /I "%FOCUSED_TEST%"=="test_color_pipeline_sdf_postprocess_cuda" call :focused_test_color_pipeline_sdf_postprocess_cuda & exit /b
if /I "%FOCUSED_TEST%"=="test_escape_time_coloring" call :focused_test_escape_time_coloring & exit /b
if /I "%FOCUSED_TEST%"=="test_fractal_parameter_surface_descriptor" call :focused_test_fractal_parameter_surface_descriptor & exit /b
if /I "%FOCUSED_TEST%"=="test_fractal_catalog_authority" call :focused_test_fractal_catalog_authority & exit /b
if /I "%FOCUSED_TEST%"=="test_fractal_types" call :focused_test_fractal_types & exit /b
if /I "%FOCUSED_TEST%"=="test_fractal_derived_fields" call :focused_test_fractal_derived_fields & exit /b
if /I "%FOCUSED_TEST%"=="test_fractal_family_rules" call :focused_test_fractal_family_rules & exit /b
if /I "%FOCUSED_TEST%"=="test_schema_binding" call :focused_test_schema_binding & exit /b
if /I "%FOCUSED_TEST%"=="test_explaino_counterfactual_repair" call :focused_test_explaino_counterfactual_repair & exit /b
if /I "%FOCUSED_TEST%"=="test_generic_equation_pack_workbench_ui" call :focused_test_generic_equation_pack_workbench_ui & exit /b
if /I "%FOCUSED_TEST%"=="test_generic_equation_pack_live" call :focused_test_generic_equation_pack_live & exit /b
if /I "%FOCUSED_TEST%"=="test_generic_equation_pack" call :focused_test_generic_equation_pack & exit /b
if /I "%FOCUSED_TEST%"=="test_sdf_pack" call :focused_test_sdf_pack & exit /b
if /I "%FOCUSED_TEST%"=="test_sdf_pack_cuda" call :focused_test_sdf_pack_cuda & exit /b
if /I "%FOCUSED_TEST%"=="test_sdf_pack_field_producer" call :focused_test_sdf_pack_field_producer & exit /b
if /I "%FOCUSED_TEST%"=="test_sdf_pack_field_producer_cuda" call :focused_test_sdf_pack_field_producer_cuda & exit /b
if /I "%FOCUSED_TEST%"=="test_sdf_pack_viewer_ui" call :focused_test_sdf_pack_viewer_ui & exit /b
if /I "%FOCUSED_TEST%"=="test_generic_probe" call :focused_test_generic_probe & exit /b
echo [build_tests_vsdevcmd] Unknown focused test target "%FOCUSED_TEST%"
exit /b 1

:full_build_start
cl /nologo /EHsc /MD /std:c++17 /O2 /D COLOR_PIPELINE_WINDOW_NO_IMGUI /I. /I.\src ^
  .\src\viewer_ui_automation_report.cpp .\tests\test_viewer_ui_automation_report.cpp ^
  /Fe:"%TESTROOT%\test_viewer_ui_automation_report.exe" ^
  /link user32.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\cli_args.cpp .\tests\test_cli_args.cpp ^
  /Fe:"%TESTROOT%\test_cli_args.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\cli_args.cpp .\src\viewer_cli.cpp .\tests\test_viewer_cli.cpp ^
  /Fe:"%TESTROOT%\test_viewer_cli.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_main.cpp ^
  /Fe:"%TESTROOT%\test_main.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\flashlight_probe.cpp .\src\lens_sdf.cpp .\src\sdf_field_signal.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_flashlight_probe.cpp .\tests\test_flashlight_render_stub.cpp .\tests\test_flashlight_capture_stub.cpp ^
  /Fe:"%TESTROOT%\test_flashlight_probe.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\runtime_walk.cpp .\src\json_min.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\tests\test_runtime_walk.cpp ^
  /Fe:"%TESTROOT%\test_runtime_walk.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\runtime_walk_headless.cpp .\src\sdf_field_signal.cpp .\tests\test_runtime_walk_headless.cpp ^
  /Fe:"%TESTROOT%\test_runtime_walk_headless.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\runtime_walk_bootstrap.cpp .\src\schema_binding.cpp .\src\json_min.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_runtime_walk_bootstrap.cpp ^
  /Fe:"%TESTROOT%\test_runtime_walk_bootstrap.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\runtime_walk.cpp .\src\runtime_walk_viewer.cpp .\src\json_min.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_runtime_walk_viewer.cpp ^
  /Fe:"%TESTROOT%\test_runtime_walk_viewer.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\runtime_walk_field_slime.cpp .\tests\test_runtime_walk_field_slime.cpp ^
  /Fe:"%TESTROOT%\test_runtime_walk_field_slime.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\runtime_walk.cpp .\src\runtime_walk_bootstrap.cpp .\src\runtime_walk_viewer_import.cpp .\src\schema_binding.cpp .\src\json_min.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_runtime_walk_viewer_import.cpp ^
  /Fe:"%TESTROOT%\test_runtime_walk_viewer_import.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\runtime_walk.cpp .\src\runtime_walk_viewer.cpp .\src\runtime_walk_viewer_session.cpp .\src\runtime_walk_bootstrap.cpp .\src\runtime_walk_viewer_import.cpp .\src\schema_binding.cpp .\src\json_min.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_runtime_walk_viewer_session.cpp ^
  /Fe:"%TESTROOT%\test_runtime_walk_viewer_session.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\cli_args.cpp .\src\viewer_cli.cpp .\src\json_min.cpp .\src\viewer_state_init.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_viewer_state_init.cpp ^
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
  .\src\lens_sdf.cpp .\src\sdf_field_signal.cpp .\tests\test_lens_sdf.cpp ^
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
  .\src\json_min.cpp .\src\explaino_seed.cpp .\src\diagnostics_state_io.cpp .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_diagnostics_state_io.cpp ^
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
  .\src\json_min.cpp .\src\color_pipeline_metadata_contract.cpp .\src\color_pipeline_metadata_parity.cpp .\tests\test_color_pipeline_core.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_core.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\tests\test_color_pipeline_window.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_window.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\lens_sdf.cpp .\src\sdf_field_signal.cpp .\src\color_pipeline_sdf_postprocess.cpp .\tests\test_color_pipeline_sdf_postprocess.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_sdf_postprocess.exe"
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
  .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_diagnostics_capture.cpp ^
  /Fe:"%TESTROOT%\test_diagnostics_capture.exe"
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

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\fractal_parameter_surface_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_fractal_parameter_surface_descriptor.cpp ^
  /Fe:"%TESTROOT%\test_fractal_parameter_surface_descriptor.exe"
if errorlevel 1 exit /b 1

call :build_generic_sample_core_object || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_fractal_probe.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_fractal_probe.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_fractal_sample_pipeline.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_fractal_sample_pipeline.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_fractal_probe_coverage.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_fractal_probe_coverage.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp .\src\fractal_parameter_surface_descriptor.cpp .\src\safe_mode_schema.cpp .\src\schema_startup_policy.cpp .\src\viewer_schema_load.cpp .\src\explaino_sidecar_model.cpp .\src\explaino_sidecar_measurement.cpp .\src\explaino_sidecar_budget.cpp .\src\explaino_sidecar_lens.cpp .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_action.cpp .\src\explaino_sidecar_trace.cpp .\src\explaino_sidecar_controller.cpp .\src\explaino_sidecar_divergence.cpp .\src\explaino_sidecar_completeness.cpp .\src\explaino_sidecar_window.cpp .\src\explaino_exploration_advisor.cpp .\src\flashlight_probe.cpp .\src\runtime_walk.cpp .\src\lens_sdf.cpp .\src\sdf_field_signal.cpp .\src\headless_modes.cpp ^
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

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_ui_schema_grouping.cpp ^
  /Fe:"%TESTROOT%\test_ui_schema_grouping.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_enum_id_utils.cpp ^
  /Fe:"%TESTROOT%\test_enum_id_utils.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_sample_result.cpp ^
  /Fe:"%TESTROOT%\test_fractal_sample_result.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_types.cpp ^
  /Fe:"%TESTROOT%\test_fractal_types.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\explaino_sidecar_refresh.cpp .\tests\test_explaino_sidecar_refresh.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_refresh.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\explaino_sidecar_cuda_sample_host.cpp .\tests\test_explaino_sidecar_cuda_sample_host.cpp ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_cuda_sample_host.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_generic_function_types.cpp ^
  /Fe:"%TESTROOT%\test_generic_function_types.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_generic_function_cpu_eval.cpp ^
  /Fe:"%TESTROOT%\test_generic_function_cpu_eval.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_seed_curve.cpp ^
  /Fe:"%TESTROOT%\test_explaino_seed_curve.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_fractal_probe_runner.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_fractal_probe_runner.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\fractal_probe_contract.cpp .\tests\test_fractal_probe_contract.cpp ^
  /Fe:"%TESTROOT%\test_fractal_probe_contract.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\safe_mode_schema.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_safe_mode_schema.cpp ^
  /Fe:"%TESTROOT%\test_safe_mode_schema.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp ^
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
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp .\src\safe_mode_schema.cpp .\src\schema_startup_policy.cpp .\src\viewer_schema_load.cpp .\src\explaino_sidecar_model.cpp .\src\explaino_sidecar_measurement.cpp .\src\explaino_sidecar_budget.cpp .\src\explaino_sidecar_lens.cpp .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_action.cpp .\src\explaino_sidecar_trace.cpp .\src\explaino_sidecar_controller.cpp .\src\explaino_sidecar_divergence.cpp .\src\explaino_sidecar_completeness.cpp .\src\explaino_sidecar_window.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_explaino_sidecar_schema_contract.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_explaino_sidecar_schema_contract.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp .\src\safe_mode_schema.cpp .\src\schema_startup_policy.cpp .\src\viewer_schema_load.cpp .\src\explaino_sidecar_model.cpp .\src\explaino_sidecar_measurement.cpp .\src\explaino_sidecar_budget.cpp .\src\explaino_sidecar_lens.cpp .\src\explaino_sidecar_energy.cpp .\src\explaino_sidecar_action.cpp .\src\explaino_sidecar_trace.cpp .\src\explaino_sidecar_controller.cpp .\src\explaino_sidecar_divergence.cpp .\src\explaino_sidecar_completeness.cpp .\src\explaino_sidecar_window.cpp .\src\explaino_exploration_advisor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_explaino_exploration_advisor.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_explaino_exploration_advisor.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

goto full_test_run

:focused_test_generic_equation_pack_workbench_ui
cl /nologo /EHsc /MD /std:c++17 /O2 /D COLOR_PIPELINE_WINDOW_NO_IMGUI /D GENERIC_EQUATION_PACK_WORKBENCH_NO_IMGUI /I. /I.\src ^
  .\src\json_min.cpp .\src\generic_equation_pack.cpp .\src\generic_equation_pack_workbench.cpp .\tests\test_generic_equation_pack_workbench_ui.cpp ^
  /Fe:"%TESTROOT%\test_generic_equation_pack_workbench_ui.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_generic_equation_pack_workbench_ui.exe" || exit /b 1
exit /b 0

:focused_test_generic_equation_pack_live
call :build_generic_sample_core_object || exit /b 1
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\json_min.cpp .\src\generic_equation_pack.cpp .\src\generic_equation_pack_live.cpp .\tests\test_generic_equation_pack_live.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  -o "%TESTROOT%\test_generic_equation_pack_live.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_generic_equation_pack_live.exe" || exit /b 1
exit /b 0

:focused_test_generic_equation_pack
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\generic_equation_pack.cpp .\tests\test_generic_equation_pack.cpp ^
  /Fe:"%TESTROOT%\test_generic_equation_pack.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_generic_equation_pack.exe" || exit /b 1

call :build_generic_sample_core_object || exit /b 1
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\json_min.cpp .\src\generic_equation_pack.cpp .\tests\test_generic_equation_pack_cuda.cu "%GENERIC_SAMPLE_CORE_OBJ%" ^
  -o "%TESTROOT%\test_generic_equation_pack_cuda.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_generic_equation_pack_cuda.exe" || exit /b 1
exit /b 0

:focused_test_viewport_interaction
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\viewport_interaction.cpp .\tests\test_viewport_interaction.cpp ^
  /Fe:"%TESTROOT%\test_viewport_interaction.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_viewport_interaction.exe" || exit /b 1
exit /b 0

:focused_test_sdf_pack
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\tests\test_sdf_pack.cpp ^
  /Fe:"%TESTROOT%\test_sdf_pack.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack.exe" || exit /b 1
exit /b 0

:focused_test_sdf_pack_cuda
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\src\sdf_pack_cuda.cu .\tests\test_sdf_pack_cuda.cu ^
  -o "%TESTROOT%\test_sdf_pack_cuda.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack_cuda.exe" || exit /b 1
exit /b 0

:focused_test_sdf_pack_field_producer
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\src\lens_sdf.cpp .\src\sdf_pack_field_producer.cpp .\tests\test_sdf_pack_field_producer.cpp ^
  /Fe:"%TESTROOT%\test_sdf_pack_field_producer.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack_field_producer.exe" || exit /b 1
exit /b 0

:focused_test_sdf_pack_field_producer_cuda
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\src\lens_sdf.cpp .\src\sdf_pack_cuda.cu .\src\sdf_pack_field_producer.cpp .\src\sdf_pack_field_producer_cuda.cu .\tests\test_sdf_pack_field_producer_cuda.cu ^
  -o "%TESTROOT%\test_sdf_pack_field_producer_cuda.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack_field_producer_cuda.exe" || exit /b 1
exit /b 0

:focused_test_sdf_pack_viewer_ui
cl /nologo /EHsc /MD /std:c++17 /O2 /D SDF_PACK_VIEWER_UI_NO_IMGUI /I. /I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\src\lens_sdf.cpp .\src\sdf_pack_field_producer.cpp .\src\sdf_pack_viewer_ui.cpp .\tests\test_sdf_pack_viewer_ui.cpp ^
  /Fe:"%TESTROOT%\test_sdf_pack_viewer_ui.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack_viewer_ui.exe" || exit /b 1
exit /b 0

:focused_test_generic_probe
call :build_generic_sample_core_object || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_generic_probe.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_generic_probe.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_generic_probe.exe" || exit /b 1
exit /b 0

:focused_test_viewer_ui_automation_report
cl /nologo /EHsc /MD /std:c++17 /O2 /D COLOR_PIPELINE_WINDOW_NO_IMGUI /I. /I.\src ^
  .\src\viewer_ui_automation_report.cpp .\tests\test_viewer_ui_automation_report.cpp ^
  /Fe:"%TESTROOT%\test_viewer_ui_automation_report.exe" ^
  /link user32.lib
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_viewer_ui_automation_report.exe" || exit /b 1
exit /b 0

:focused_test_viewer_cli
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\cli_args.cpp .\src\viewer_cli.cpp .\tests\test_viewer_cli.cpp ^
  /Fe:"%TESTROOT%\test_viewer_cli.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_viewer_cli.exe" || exit /b 1
exit /b 0

:focused_test_flashlight_probe
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\flashlight_probe.cpp .\src\lens_sdf.cpp .\src\sdf_field_signal.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_flashlight_probe.cpp .\tests\test_flashlight_render_stub.cpp .\tests\test_flashlight_capture_stub.cpp ^
  /Fe:"%TESTROOT%\test_flashlight_probe.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_flashlight_probe.exe" || exit /b 1
exit /b 0

:focused_test_diagnostics_state_io
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\explaino_seed.cpp .\src\diagnostics_state_io.cpp .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_diagnostics_state_io.cpp ^
  /Fe:"%TESTROOT%\test_diagnostics_state_io.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_diagnostics_state_io.exe" || exit /b 1
exit /b 0

:focused_test_diagnostics_capture
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_diagnostics_capture.cpp ^
  /Fe:"%TESTROOT%\test_diagnostics_capture.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_diagnostics_capture.exe" || exit /b 1
exit /b 0

:focused_test_lens_sdf
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\lens_sdf.cpp .\src\sdf_field_signal.cpp .\tests\test_lens_sdf.cpp ^
  /Fe:"%TESTROOT%\test_lens_sdf.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_lens_sdf.exe" || exit /b 1
exit /b 0

:focused_test_lens_sdf_cuda
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\lens_sdf.cpp .\src\lens_sdf_cuda.cu .\tests\test_lens_sdf_cuda.cu ^
  -o "%TESTROOT%\test_lens_sdf_cuda.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_lens_sdf_cuda.exe" || exit /b 1
exit /b 0

:focused_test_finding_archive_actions
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\finding_archive_actions.cpp .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_finding_archive_actions.cpp ^
  /Fe:"%TESTROOT%\test_finding_archive_actions.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_finding_archive_actions.exe" || exit /b 1
exit /b 0

:focused_test_finding_state_actions
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\diagnostics_state_io.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\finding_state_actions.cpp .\tests\test_finding_state_actions.cpp ^
  /Fe:"%TESTROOT%\test_finding_state_actions.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_finding_state_actions.exe" || exit /b 1
exit /b 0

:focused_test_viewer_state_init
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\cli_args.cpp .\src\viewer_cli.cpp .\src\json_min.cpp .\src\viewer_state_init.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_viewer_state_init.cpp ^
  /Fe:"%TESTROOT%\test_viewer_state_init.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_viewer_state_init.exe" || exit /b 1
exit /b 0

:focused_test_viewer_render_pacing
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\viewer_render_pacing.cpp .\tests\test_viewer_render_pacing.cpp ^
  /Fe:"%TESTROOT%\test_viewer_render_pacing.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_viewer_render_pacing.exe" || exit /b 1
exit /b 0

:focused_serializer_owner_fast
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\explaino_seed.cpp .\src\diagnostics_state_io.cpp .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_diagnostics_state_io.cpp ^
  /Fe:"%TESTROOT%\test_diagnostics_state_io.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\finding_archive_actions.cpp .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_finding_archive_actions.cpp ^
  /Fe:"%TESTROOT%\test_finding_archive_actions.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_diagnostics_state_io.exe" || exit /b 1
call :run_test "%TESTROOT%\test_finding_archive_actions.exe" || exit /b 1
exit /b 0

:focused_test_sample_tier_resolver
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\sample_tier_resolver.cpp .\tests\test_sample_tier_resolver.cpp ^
  /Fe:"%TESTROOT%\test_sample_tier_resolver.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_sample_tier_resolver.exe" || exit /b 1
exit /b 0

:focused_test_fractal_renderer
call :build_fractal_cuda_common_objects || exit /b 1
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_fractal_renderer.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_fractal_renderer.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_fractal_renderer.exe" || exit /b 1
exit /b 0

:focused_test_fractal_sample_kernel
call :build_fractal_cuda_common_objects || exit /b 1
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_fractal_sample_kernel.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_fractal_sample_kernel.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_fractal_sample_kernel.exe" || exit /b 1
exit /b 0

:focused_test_runtime_walk_headless
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\runtime_walk_headless.cpp .\src\sdf_field_signal.cpp .\tests\test_runtime_walk_headless.cpp ^
  /Fe:"%TESTROOT%\test_runtime_walk_headless.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_runtime_walk_headless.exe" || exit /b 1
exit /b 0

:focused_test_explaino_counterfactual_repair
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_fractal_derived_fields.cpp ^
  /Fe:"%TESTROOT%\test_fractal_derived_fields.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_schema_binding.cpp ^
  /Fe:"%TESTROOT%\test_schema_binding.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_runtime_validation.cpp ^
  /Fe:"%TESTROOT%\test_fractal_runtime_validation.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\safe_mode_schema.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_ui_schema.cpp ^
  /Fe:"%TESTROOT%\test_ui_schema.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\safe_mode_schema.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_safe_mode_schema.cpp ^
  /Fe:"%TESTROOT%\test_safe_mode_schema.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_fractal_derived_fields.exe" || exit /b 1
call :run_test "%TESTROOT%\test_schema_binding.exe" || exit /b 1
call :run_test "%TESTROOT%\test_fractal_runtime_validation.exe" || exit /b 1
call :run_test "%TESTROOT%\test_ui_schema.exe" || exit /b 1
call :run_test "%TESTROOT%\test_safe_mode_schema.exe" || exit /b 1
exit /b 0

:focused_test_ui_schema
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\safe_mode_schema.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_ui_schema.cpp ^
  /Fe:"%TESTROOT%\test_ui_schema.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_ui_schema.exe" || exit /b 1
exit /b 0

:focused_test_safe_mode_schema
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\safe_mode_schema.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_safe_mode_schema.cpp ^
  /Fe:"%TESTROOT%\test_safe_mode_schema.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_safe_mode_schema.exe" || exit /b 1
exit /b 0

:focused_test_color_pipeline_core
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\color_pipeline_metadata_contract.cpp .\src\color_pipeline_metadata_parity.cpp .\tests\test_color_pipeline_core.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_core.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_core.exe" || exit /b 1
exit /b 0

:focused_test_color_pipeline_window
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\tests\test_color_pipeline_window.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_window.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_window.exe" || exit /b 1
exit /b 0

:focused_test_color_pipeline_sdf_field_groups
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_color_pipeline_sdf_field_groups.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_sdf_field_groups.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_sdf_field_groups.exe" || exit /b 1
exit /b 0

:focused_test_color_pipeline_sdf_postprocess
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\lens_sdf.cpp .\src\sdf_field_signal.cpp .\src\color_pipeline_sdf_postprocess.cpp .\tests\test_color_pipeline_sdf_postprocess.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_sdf_postprocess.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_sdf_postprocess.exe" || exit /b 1
exit /b 0

:focused_test_color_pipeline_sdf_postprocess_cuda
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\lens_sdf.cpp .\src\sdf_field_signal.cpp .\src\color_pipeline_sdf_postprocess.cpp .\src\color_pipeline_sdf_postprocess_cuda.cu .\tests\test_color_pipeline_sdf_postprocess_cuda.cu ^
  -o "%TESTROOT%\test_color_pipeline_sdf_postprocess_cuda.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_sdf_postprocess_cuda.exe" || exit /b 1
exit /b 0

:focused_test_escape_time_coloring
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_escape_time_coloring.cpp ^
  /Fe:"%TESTROOT%\test_escape_time_coloring.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_escape_time_coloring.exe" || exit /b 1
exit /b 0

:focused_advanced_color_grading_red
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\color_pipeline_metadata_contract.cpp .\src\color_pipeline_metadata_parity.cpp .\tests\test_color_pipeline_core.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_core.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\tests\test_color_pipeline_window.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_window.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_schema_binding.cpp ^
  /Fe:"%TESTROOT%\test_schema_binding.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_escape_time_coloring.cpp ^
  /Fe:"%TESTROOT%\test_escape_time_coloring.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_family_rules.cpp ^
  /Fe:"%TESTROOT%\test_fractal_family_rules.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_core.exe" || exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_window.exe" || exit /b 1
call :run_test "%TESTROOT%\test_schema_binding.exe" || exit /b 1
call :run_test "%TESTROOT%\test_escape_time_coloring.exe" || exit /b 1
call :run_test "%TESTROOT%\test_fractal_family_rules.exe" || exit /b 1
exit /b 0

:focused_advanced_color_grading_owner
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\color_pipeline_metadata_contract.cpp .\src\color_pipeline_metadata_parity.cpp .\tests\test_color_pipeline_core.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_core.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\tests\test_color_pipeline_window.cpp ^
  /Fe:"%TESTROOT%\test_color_pipeline_window.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_schema_binding.cpp ^
  /Fe:"%TESTROOT%\test_schema_binding.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_escape_time_coloring.cpp ^
  /Fe:"%TESTROOT%\test_escape_time_coloring.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_family_rules.cpp ^
  /Fe:"%TESTROOT%\test_fractal_family_rules.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\explaino_seed.cpp .\src\diagnostics_state_io.cpp .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_diagnostics_state_io.cpp ^
  /Fe:"%TESTROOT%\test_diagnostics_state_io.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\finding_archive_actions.cpp .\src\diagnostics_capture.cpp .\src\render_capture_guard.cpp .\tests\test_finding_archive_actions.cpp ^
  /Fe:"%TESTROOT%\test_finding_archive_actions.exe"
if errorlevel 1 exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\tests\test_runtime_reset.cpp ^
  /Fe:"%TESTROOT%\test_runtime_reset.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_core.exe" || exit /b 1
call :run_test "%TESTROOT%\test_color_pipeline_window.exe" || exit /b 1
call :run_test "%TESTROOT%\test_schema_binding.exe" || exit /b 1
call :run_test "%TESTROOT%\test_escape_time_coloring.exe" || exit /b 1
call :run_test "%TESTROOT%\test_fractal_family_rules.exe" || exit /b 1
call :run_test "%TESTROOT%\test_diagnostics_state_io.exe" || exit /b 1
call :run_test "%TESTROOT%\test_finding_archive_actions.exe" || exit /b 1
call :run_test "%TESTROOT%\test_runtime_reset.exe" || exit /b 1
exit /b 0

:focused_test_fractal_parameter_surface_descriptor
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\fractal_parameter_surface_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_fractal_parameter_surface_descriptor.cpp ^
  /Fe:"%TESTROOT%\test_fractal_parameter_surface_descriptor.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_fractal_parameter_surface_descriptor.exe" || exit /b 1
exit /b 0

:focused_test_fractal_catalog_authority
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\function_descriptor.cpp .\tests\test_fractal_catalog_authority.cpp ^
  /Fe:"%TESTROOT%\test_fractal_catalog_authority.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_fractal_catalog_authority.exe" || exit /b 1
exit /b 0

:focused_test_fractal_types
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_types.cpp ^
  /Fe:"%TESTROOT%\test_fractal_types.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_fractal_types.exe" || exit /b 1
exit /b 0

:focused_test_fractal_derived_fields
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_fractal_derived_fields.cpp ^
  /Fe:"%TESTROOT%\test_fractal_derived_fields.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_fractal_derived_fields.exe" || exit /b 1
exit /b 0

:focused_test_fractal_family_rules
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_family_rules.cpp ^
  /Fe:"%TESTROOT%\test_fractal_family_rules.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_fractal_family_rules.exe" || exit /b 1
exit /b 0

:focused_test_schema_binding
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_schema_binding.cpp ^
  /Fe:"%TESTROOT%\test_schema_binding.exe"
if errorlevel 1 exit /b 1
call :run_test "%TESTROOT%\test_schema_binding.exe" || exit /b 1
exit /b 0

:full_test_run

call :run_test "%TESTROOT%\test_viewer_ui_automation_report.exe" || exit /b 1
call :run_test "%TESTROOT%\test_cli_args.exe" || exit /b 1

call :run_test "%TESTROOT%\test_viewer_cli.exe" || exit /b 1

call :run_test "%TESTROOT%\test_main.exe" || exit /b 1

call :run_test "%TESTROOT%\test_viewer_state_init.exe" || exit /b 1

call :run_test "%TESTROOT%\test_viewer_schema_load.exe" || exit /b 1

call :run_test "%TESTROOT%\test_json_min.exe" || exit /b 1

call :run_test "%TESTROOT%\test_viewport_interaction.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_seed.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_seed_dynamics.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_budget.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_lens.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_action.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_energy.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_trace.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_controller.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_divergence.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_completeness.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_measurement.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_derived_fields.exe" || exit /b 1

call :run_test "%TESTROOT%\test_seed_tween_continuity.exe" || exit /b 1

call :run_test "%TESTROOT%\test_runtime_reset.exe" || exit /b 1

call :run_test "%TESTROOT%\test_lens_sdf.exe" || exit /b 1

call :run_test "%TESTROOT%\test_runtime_walk.exe" || exit /b 1

call :run_test "%TESTROOT%\test_runtime_walk_headless.exe" || exit /b 1

call :run_test "%TESTROOT%\test_runtime_walk_bootstrap.exe" || exit /b 1

call :run_test "%TESTROOT%\test_runtime_walk_viewer.exe" || exit /b 1

call :run_test "%TESTROOT%\test_runtime_walk_field_slime.exe" || exit /b 1

call :run_test "%TESTROOT%\test_runtime_walk_viewer_import.exe" || exit /b 1

call :run_test "%TESTROOT%\test_runtime_walk_viewer_session.exe" || exit /b 1

call :run_test "%TESTROOT%\test_viewer_shutdown.exe" || exit /b 1

call :run_test "%TESTROOT%\test_viewer_sweep.exe" || exit /b 1

call :run_test "%TESTROOT%\test_sweep_player.exe" || exit /b 1

call :run_test "%TESTROOT%\test_view_hp_sync.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_variant_benchmark.exe" || exit /b 1

call :run_test "%TESTROOT%\test_diagnostics_state_io.exe" || exit /b 1

call :run_test "%TESTROOT%\test_finding_state_actions.exe" || exit /b 1

call :run_test "%TESTROOT%\test_schema_startup_policy.exe" || exit /b 1

call :run_test "%TESTROOT%\test_schema_binding.exe" || exit /b 1

call :run_test "%TESTROOT%\test_color_pipeline_core.exe" || exit /b 1

call :run_test "%TESTROOT%\test_color_pipeline_window.exe" || exit /b 1

call :run_test "%TESTROOT%\test_color_pipeline_sdf_postprocess.exe" || exit /b 1

call :run_test "%TESTROOT%\test_finding_capture_state.exe" || exit /b 1

call :run_test "%TESTROOT%\test_finding_archive_actions.exe" || exit /b 1

call :run_test "%TESTROOT%\test_diagnostics_capture.exe" || exit /b 1

call :run_test "%TESTROOT%\test_render_capture_guard.exe" || exit /b 1

call :run_test "%TESTROOT%\test_viewer_render_pacing.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_runtime_validation.exe" || exit /b 1

call :run_test "%TESTROOT%\test_escape_time_specialized_formulas.exe" || exit /b 1

call :run_test "%TESTROOT%\test_perturbation_reference_orbit.exe" || exit /b 1

call :run_test "%TESTROOT%\test_escape_time_coloring.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_collatz_formulas.exe" || exit /b 1

call :run_test "%TESTROOT%\test_polynomial_eval_real_coeffs.exe" || exit /b 1

call :run_test "%TESTROOT%\test_basin_coloring.exe" || exit /b 1

call :run_test "%TESTROOT%\test_escape_time_direct_formulas.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_parameter_surface_descriptor.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_probe.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_family_rules.exe" || exit /b 1

call :run_test "%TESTROOT%\test_ui_schema.exe" || exit /b 1

call :run_test "%TESTROOT%\test_ui_schema_grouping.exe" || exit /b 1

call :run_test "%TESTROOT%\test_enum_id_utils.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_sample_result.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_types.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_refresh.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_cuda_sample_host.exe" || exit /b 1

call :run_test "%TESTROOT%\test_generic_function_types.exe" || exit /b 1

call :run_test "%TESTROOT%\test_generic_function_cpu_eval.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_seed_curve.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_probe_runner.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_probe_contract.exe" || exit /b 1

call :run_test "%TESTROOT%\test_safe_mode_schema.exe" || exit /b 1

call :run_test "%TESTROOT%\test_function_descriptor.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_model.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_window.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_sidecar_schema_contract.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_exploration_advisor.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\sample_tier_resolver.cpp .\tests\test_sample_tier_resolver.cpp ^
  /Fe:"%TESTROOT%\test_sample_tier_resolver.exe"
if errorlevel 1 exit /b 1

call :build_fractal_cuda_common_objects || exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_escape_time_sample_tier.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_escape_time_sample_tier.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_newton_basin_regression.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_newton_basin_regression.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_fractal_sample_device.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_fractal_sample_device.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_fractal_renderer.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_fractal_renderer.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_fractal_sample_core.cu "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_fractal_sample_core.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_fractal_sample_kernel.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_fractal_sample_kernel.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_fractal_sample_equivalence.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_fractal_sample_equivalence.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_explaino_zero_axis_equivalence.cu "%FRACTAL_RENDERER_OBJ%" "%FRACTAL_SAMPLE_CORE_OBJ%" "%SAMPLE_TIER_RESOLVER_OBJ%" ^
  -o "%TESTROOT%\test_explaino_zero_axis_equivalence.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_sample_tier_resolver.exe" || exit /b 1

call :run_test "%TESTROOT%\test_escape_time_sample_tier.exe" || exit /b 1

call :run_test "%TESTROOT%\test_newton_basin_regression.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_sample_device.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_renderer.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_sample_core.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_sample_kernel.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_sample_equivalence.exe" || exit /b 1

call :run_test "%TESTROOT%\test_explaino_zero_axis_equivalence.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_nova_iteration.cpp ^
  /Fe:"%TESTROOT%\test_nova_iteration.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_nova_iteration.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_joy_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_joy_continuity.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_explaino_joy_continuity.exe" || exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\\src ^
  .\\tests\\test_explaino_fold_continuity.cpp ^
  /Fe:"%TESTROOT%\\test_explaino_fold_continuity.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\\test_explaino_fold_continuity.exe" || exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\\src ^
  .\\tests\\test_explaino_bell_continuity.cpp ^
  /Fe:"%TESTROOT%\\test_explaino_bell_continuity.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\\test_explaino_bell_continuity.exe" || exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_ripple_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_ripple_continuity.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_explaino_ripple_continuity.exe" || exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_splice_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_splice_continuity.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_explaino_splice_continuity.exe" || exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_vortex_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_vortex_continuity.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_explaino_vortex_continuity.exe" || exit /b 1
cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_explaino_tension_continuity.cpp ^
  /Fe:"%TESTROOT%\test_explaino_tension_continuity.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_explaino_tension_continuity.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_sample_pipeline.exe" || exit /b 1

call :run_test "%TESTROOT%\test_fractal_probe_coverage.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\param_anim_dynamics.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_param_anim_dynamics.cpp ^
  /Fe:"%TESTROOT%\test_param_anim_dynamics.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_headless_modes.exe" || exit /b 1

call :run_test "%TESTROOT%\test_param_anim_dynamics.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\explaino_seed.cpp .\src\param_anim_dynamics.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp ^
  .\tests\test_param_anim_generic.cpp ^
  /Fe:"%TESTROOT%\test_param_anim_generic.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_param_anim_generic.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_generic_function_math.cpp ^
  /Fe:"%TESTROOT%\test_generic_function_math.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_function_math.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\generic_equation_pack.cpp .\tests\test_generic_equation_pack.cpp ^
  /Fe:"%TESTROOT%\test_generic_equation_pack.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\tests\test_sdf_pack.cpp ^
  /Fe:"%TESTROOT%\test_sdf_pack.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\src\lens_sdf.cpp .\src\sdf_pack_field_producer.cpp .\tests\test_sdf_pack_field_producer.cpp ^
  /Fe:"%TESTROOT%\test_sdf_pack_field_producer.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\src\sdf_pack_cuda.cu .\tests\test_sdf_pack_cuda.cu ^
  -o "%TESTROOT%\test_sdf_pack_cuda.exe"
if errorlevel 1 exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\json_min.cpp .\src\sdf_pack.cpp .\src\lens_sdf.cpp .\src\sdf_pack_cuda.cu .\src\sdf_pack_field_producer.cpp .\src\sdf_pack_field_producer_cuda.cu .\tests\test_sdf_pack_field_producer_cuda.cu ^
  -o "%TESTROOT%\test_sdf_pack_field_producer_cuda.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /D COLOR_PIPELINE_WINDOW_NO_IMGUI /D GENERIC_EQUATION_PACK_WORKBENCH_NO_IMGUI /I. /I.\src ^
  .\src\json_min.cpp .\src\generic_equation_pack.cpp .\src\generic_equation_pack_workbench.cpp .\tests\test_generic_equation_pack_workbench_ui.cpp ^
  /Fe:"%TESTROOT%\test_generic_equation_pack_workbench_ui.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_equation_pack.exe" || exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack.exe" || exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack_cuda.exe" || exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack_field_producer.exe" || exit /b 1
call :run_test "%TESTROOT%\test_sdf_pack_field_producer_cuda.exe" || exit /b 1
call :run_test "%TESTROOT%\test_generic_equation_pack_workbench_ui.exe" || exit /b 1

call :build_generic_sample_core_object || exit /b 1
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\json_min.cpp .\src\generic_equation_pack.cpp .\src\generic_equation_pack_live.cpp .\tests\test_generic_equation_pack_live.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  -o "%TESTROOT%\test_generic_equation_pack_live.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_equation_pack_live.exe" || exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_generic_function_eval.cu ^
  -o "%TESTROOT%\test_generic_function_eval.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_function_eval.exe" || exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_generic_sample_core.cu "%GENERIC_SAMPLE_CORE_OBJ%" ^
  -o "%TESTROOT%\test_generic_sample_core.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_sample_core.exe" || exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\tests\test_generic_sample_parity.cu "%GENERIC_SAMPLE_CORE_OBJ%" ^
  -o "%TESTROOT%\test_generic_sample_parity.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_sample_parity.exe" || exit /b 1

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  %CUDA_GENCODE_FLAGS% ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\json_min.cpp .\src\generic_equation_pack.cpp .\tests\test_generic_equation_pack_cuda.cu "%GENERIC_SAMPLE_CORE_OBJ%" ^
  -o "%TESTROOT%\test_generic_equation_pack_cuda.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_equation_pack_cuda.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_generic_function_parser.cpp ^
  /Fe:"%TESTROOT%\test_generic_function_parser.exe"
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_function_parser.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_callable_engine_adversarial.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_callable_engine_adversarial.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_callable_engine_adversarial.exe" || exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src /I.\third_party\imgui ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\src\schema_binding.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\fractal_probe_contract.cpp .\src\fractal_probe_runner.cpp .\src\generic_equation_pack.cpp .\src\function_descriptor.cpp ^
  .\third_party\imgui\imgui.cpp .\third_party\imgui\imgui_draw.cpp .\third_party\imgui\imgui_tables.cpp .\third_party\imgui\imgui_widgets.cpp .\tests\test_generic_probe.cpp "%GENERIC_SAMPLE_CORE_OBJ%" ^
  /Fe:"%TESTROOT%\test_generic_probe.exe" ^
  /link /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib
if errorlevel 1 exit /b 1

call :run_test "%TESTROOT%\test_generic_probe.exe" || exit /b 1

echo All helper tests passed.
exit /b 0
