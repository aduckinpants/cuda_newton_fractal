@echo off
setlocal

REM Reuse the existing VS dev environment path used by the CUDA demo
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
if errorlevel 1 exit /b 1

where cl >NUL 2>NUL
if errorlevel 1 (
  echo cl.exe not found after VsDevCmd
  exit /b 1
)

cd /d C:\code\cuda_newton_fractal_clone\ui_app

if not exist third_party\imgui\imgui.h (
  echo Dear ImGui not staged. Run: powershell -ExecutionPolicy Bypass -File .\setup_imgui.ps1
  exit /b 1
)

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set OUTROOT=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\runtime
set BUILDROOT=%OUTROOT%\build
set PRIMARY_EXE=%OUTROOT%\fractal_ui.exe
set PRIMARY_EXE_NAME=fractal_ui.exe
set FALLBACK_EXE=%OUTROOT%\fractal_ui_dev.exe
set FALLBACK_EXE_NAME=fractal_ui_dev.exe
set LINK_LOG=%BUILDROOT%\link_runtime.log
for %%I in (..) do set REPO_ROOT=%%~fI

if not exist "%OUTROOT%" mkdir "%OUTROOT%"
if not exist "%BUILDROOT%" mkdir "%BUILDROOT%"

set IMGUI=third_party\imgui

REM Compile CUDA renderer
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I"%IMGUI%" -I"%IMGUI%\backends" ^
  -c .\src\fractal_renderer.cu -o "%BUILDROOT%\fractal_renderer.obj"
if errorlevel 1 exit /b 1

REM Compile app + ImGui sources
cl /nologo /EHsc /MD /std:c++17 /O2 ^
  /I"%IMGUI%" /I"%IMGUI%\backends" ^
  /I"%CUDA_PATH%\include" ^
  /c .\src\main.cpp .\src\json_min.cpp .\src\ui_schema.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\explaino_seed_dynamics.cpp .\src\fractal_derived_fields.cpp .\src\diagnostics_capture.cpp .\src\diagnostics_state_io.cpp .\src\finding_state_actions.cpp .\src\finding_archive_actions.cpp .\src\schema_startup_policy.cpp .\src\sweep_player.cpp .\src\safe_mode_schema.cpp .\src\schema_binding.cpp .\src\lens_sdf.cpp .\src\runtime_reset.cpp .\src\viewer_shutdown.cpp .\src\viewer_sweep.cpp ^
  "%IMGUI%\imgui.cpp" "%IMGUI%\imgui_draw.cpp" "%IMGUI%\imgui_tables.cpp" "%IMGUI%\imgui_widgets.cpp" ^
  "%IMGUI%\backends\imgui_impl_win32.cpp" "%IMGUI%\backends\imgui_impl_dx11.cpp" ^
  /Fo"%BUILDROOT%\\"
if errorlevel 1 exit /b 1

REM Link (keep /MD dynamic CRT; link CUDA runtime explicitly)
call :link_runtime "%PRIMARY_EXE%" "%LINK_LOG%"
if errorlevel 1 (
  call :is_locked_primary_runtime_failure "%LINK_LOG%" "%PRIMARY_EXE%"
  if errorlevel 1 exit /b 1
  echo Published runtime is locked; retrying link as %FALLBACK_EXE_NAME%
  call :link_runtime "%FALLBACK_EXE%" "%LINK_LOG%"
  if errorlevel 1 exit /b 1
  set ACTIVE_EXE=%FALLBACK_EXE%
  set ACTIVE_EXE_NAME=%FALLBACK_EXE_NAME%
) else (
  set ACTIVE_EXE=%PRIMARY_EXE%
  set ACTIVE_EXE_NAME=%PRIMARY_EXE_NAME%
)
del "%LINK_LOG%" >NUL 2>NUL

REM Stage schema beside the published runtime so the external exe can validate and render without touching the repo tree.
if not exist "%OUTROOT%\ui" mkdir "%OUTROOT%\ui"
if exist ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json (
  copy /Y ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json "%OUTROOT%\ui\fractal_binding_surface_v1.ui_schema.canonical.json" >NUL
) else (
  echo WARNING: canonical schema missing at ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json
)

copy /Y .\fractal_ui_launcher_template.cmd "%OUTROOT%\fractal_ui.cmd" >NUL
if errorlevel 1 (
  echo Failed to stage runtime launcher script to %OUTROOT%\fractal_ui.cmd
  exit /b 1
)

> "%OUTROOT%\fractal_ui_active.txt" echo %ACTIVE_EXE_NAME%
if errorlevel 1 (
  echo Failed to write runtime launcher metadata to %OUTROOT%\fractal_ui_active.txt
  exit /b 1
)

> "%OUTROOT%\fractal_ui_repo_root.txt" echo %REPO_ROOT%
if errorlevel 1 (
  echo Failed to write runtime repo-root metadata to %OUTROOT%\fractal_ui_repo_root.txt
  exit /b 1
)

echo Active runtime: %ACTIVE_EXE%
exit /b 0

:link_runtime
link /nologo /SUBSYSTEM:WINDOWS /OUT:"%~1" ^
  "%BUILDROOT%\main.obj" "%BUILDROOT%\json_min.obj" "%BUILDROOT%\ui_schema.obj" "%BUILDROOT%\view_hp_sync.obj" "%BUILDROOT%\explaino_seed.obj" "%BUILDROOT%\explaino_seed_dynamics.obj" "%BUILDROOT%\fractal_derived_fields.obj" "%BUILDROOT%\diagnostics_capture.obj" "%BUILDROOT%\diagnostics_state_io.obj" "%BUILDROOT%\finding_state_actions.obj" "%BUILDROOT%\finding_archive_actions.obj" "%BUILDROOT%\schema_startup_policy.obj" "%BUILDROOT%\sweep_player.obj" "%BUILDROOT%\safe_mode_schema.obj" "%BUILDROOT%\schema_binding.obj" "%BUILDROOT%\lens_sdf.obj" "%BUILDROOT%\runtime_reset.obj" "%BUILDROOT%\viewer_shutdown.obj" "%BUILDROOT%\viewer_sweep.obj" ^
  "%BUILDROOT%\imgui.obj" "%BUILDROOT%\imgui_draw.obj" "%BUILDROOT%\imgui_tables.obj" "%BUILDROOT%\imgui_widgets.obj" ^
  "%BUILDROOT%\imgui_impl_win32.obj" "%BUILDROOT%\imgui_impl_dx11.obj" "%BUILDROOT%\fractal_renderer.obj" ^
  /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib ^
  d3d11.lib dxgi.lib d3dcompiler.lib user32.lib gdi32.lib shell32.lib >"%~2" 2>&1
set LINK_EXIT=%ERRORLEVEL%
type "%~2"
exit /b %LINK_EXIT%

:is_locked_primary_runtime_failure
findstr /C:"LNK1104" "%~1" >NUL
if errorlevel 1 exit /b 1
findstr /C:"cannot open file" "%~1" >NUL
if errorlevel 1 exit /b 1
findstr /C:"%~2" "%~1" >NUL
if errorlevel 1 exit /b 1
exit /b 0
