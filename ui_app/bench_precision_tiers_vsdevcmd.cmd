@echo off
setlocal

call "%~dp0..\tools\call_vsdevcmd.cmd"
if errorlevel 1 exit /b 1

cd /d C:\code\cuda_newton_fractal_clone\ui_app

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set TESTROOT=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\build_tests
set OUTCSV=%TESTROOT%\bench_precision_tiers.csv

if not exist "%TESTROOT%" mkdir "%TESTROOT%"

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" ^
  -I. -I.\src ^
  .\src\fractal_renderer.cu .\src\sample_tier_resolver.cpp .\tests\bench_precision_tiers.cu ^
  -o "%TESTROOT%\bench_precision_tiers.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\bench_precision_tiers.exe" "%OUTCSV%"
if errorlevel 1 exit /b 1

echo Wrote benchmark CSV: %OUTCSV%
exit /b 0
