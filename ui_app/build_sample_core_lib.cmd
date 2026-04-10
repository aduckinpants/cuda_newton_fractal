@echo off
setlocal

REM Build fractal_sample_core as a standalone linkable static library.
REM Output: fractal_sample_core.lib (CUDA + host code, no DX11/ImGui deps).
REM
REM Downstream consumers:
REM   1. Include ui_app/src/fractal_types.h, fractal_sample_result.h
REM   2. Link fractal_sample_core.lib + cudart.lib + cuda.lib
REM   3. Call SampleFractalPoints() from host code

call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
if errorlevel 1 exit /b 1

cd /d C:\code\cuda_newton_fractal_clone\ui_app

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set OUTDIR=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\lib
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

echo --- Compiling fractal_sample_core.cu ---
nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I.\src ^
  -c .\src\fractal_sample_core.cu -o "%OUTDIR%\fractal_sample_core.obj"
if errorlevel 1 (
  echo FAIL: fractal_sample_core.cu compilation failed.
  exit /b 1
)

echo --- Compiling sample_tier_resolver.cpp ---
cl /nologo /EHsc /MD /std:c++17 /O2 /I.\src ^
  /c .\src\sample_tier_resolver.cpp /Fo"%OUTDIR%\sample_tier_resolver.obj"
if errorlevel 1 (
  echo FAIL: sample_tier_resolver.cpp compilation failed.
  exit /b 1
)

echo --- Creating fractal_sample_core.lib ---
lib /nologo /OUT:"%OUTDIR%\fractal_sample_core.lib" ^
  "%OUTDIR%\fractal_sample_core.obj" "%OUTDIR%\sample_tier_resolver.obj"
if errorlevel 1 (
  echo FAIL: lib.exe failed.
  exit /b 1
)

echo.
echo OK: %OUTDIR%\fractal_sample_core.lib
echo.
echo Link with: fractal_sample_core.lib cudart.lib cuda.lib
echo Include:   ui_app/src/fractal_types.h  ui_app/src/fractal_sample_result.h
exit /b 0
