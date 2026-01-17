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

cd /d C:\artifacts\cuda_newton_fractal\ui_app

if not exist third_party\imgui\imgui.h (
  echo Dear ImGui not staged. Run: powershell -ExecutionPolicy Bypass -File .\setup_imgui.ps1
  exit /b 1
)

if not exist build mkdir build

set IMGUI=third_party\imgui

REM Compile CUDA renderer
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I"%IMGUI%" -I"%IMGUI%\backends" ^
  -c .\src\fractal_renderer.cu -o .\build\fractal_renderer.obj
if errorlevel 1 exit /b 1

REM Compile app + ImGui sources
cl /nologo /EHsc /MD /std:c++17 /O2 ^
  /I"%IMGUI%" /I"%IMGUI%\backends" ^
  /I"%CUDA_PATH%\include" ^
  /c .\src\main.cpp .\src\json_min.cpp .\src\ui_schema.cpp ^
  "%IMGUI%\imgui.cpp" "%IMGUI%\imgui_draw.cpp" "%IMGUI%\imgui_tables.cpp" "%IMGUI%\imgui_widgets.cpp" ^
  "%IMGUI%\backends\imgui_impl_win32.cpp" "%IMGUI%\backends\imgui_impl_dx11.cpp" ^
  /Fo.\build\
if errorlevel 1 exit /b 1

REM Link (keep /MD dynamic CRT; link CUDA runtime explicitly)
link /nologo /SUBSYSTEM:WINDOWS /OUT:fractal_ui.exe ^
  .\build\main.obj .\build\json_min.obj .\build\ui_schema.obj ^
  .\build\imgui.obj .\build\imgui_draw.obj .\build\imgui_tables.obj .\build\imgui_widgets.obj ^
  .\build\imgui_impl_win32.obj .\build\imgui_impl_dx11.obj .\build\fractal_renderer.obj ^
  /LIBPATH:"%CUDA_PATH%\lib\x64" cudart.lib cuda.lib ^
  d3d11.lib dxgi.lib d3dcompiler.lib user32.lib gdi32.lib shell32.lib

REM Stage UI schema artifact next to the exe (best-effort; Safe Mode handles missing/invalid schema during edits)
if not exist ui mkdir ui
if exist ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json (
  copy /Y ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json ui\fractal_binding_surface_v1.ui_schema.canonical.json >NUL
) else (
  echo WARNING: canonical schema missing at ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json
)

exit /b %errorlevel%
