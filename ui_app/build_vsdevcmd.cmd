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
  /c .\src\main.cpp .\src\json_min.cpp .\src\ui_schema.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\diagnostics_capture.cpp .\src\sweep_player.cpp ^
  "%IMGUI%\imgui.cpp" "%IMGUI%\imgui_draw.cpp" "%IMGUI%\imgui_tables.cpp" "%IMGUI%\imgui_widgets.cpp" ^
  "%IMGUI%\backends\imgui_impl_win32.cpp" "%IMGUI%\backends\imgui_impl_dx11.cpp" ^
  /Fo"%BUILDROOT%\\"
if errorlevel 1 exit /b 1

REM Link (keep /MD dynamic CRT; link CUDA runtime explicitly)
link /nologo /SUBSYSTEM:WINDOWS /OUT:"%OUTROOT%\fractal_ui.exe" ^
  "%BUILDROOT%\main.obj" "%BUILDROOT%\json_min.obj" "%BUILDROOT%\ui_schema.obj" "%BUILDROOT%\view_hp_sync.obj" "%BUILDROOT%\explaino_seed.obj" "%BUILDROOT%\fractal_derived_fields.obj" "%BUILDROOT%\diagnostics_capture.obj" "%BUILDROOT%\sweep_player.obj" ^
  "%BUILDROOT%\imgui.obj" "%BUILDROOT%\imgui_draw.obj" "%BUILDROOT%\imgui_tables.obj" "%BUILDROOT%\imgui_widgets.obj" ^
  "%BUILDROOT%\imgui_impl_win32.obj" "%BUILDROOT%\imgui_impl_dx11.obj" "%BUILDROOT%\fractal_renderer.obj" ^
  /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib ^
  d3d11.lib dxgi.lib d3dcompiler.lib user32.lib gdi32.lib shell32.lib

REM Stage schema beside the published runtime so the external exe can validate and render without touching the repo tree.
if not exist "%OUTROOT%\ui" mkdir "%OUTROOT%\ui"
if exist ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json (
  copy /Y ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json "%OUTROOT%\ui\fractal_binding_surface_v1.ui_schema.canonical.json" >NUL
) else (
  echo WARNING: canonical schema missing at ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json
)

exit /b %errorlevel%
