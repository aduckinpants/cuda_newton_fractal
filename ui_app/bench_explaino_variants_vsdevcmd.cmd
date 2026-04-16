@echo off
setlocal

call "%~dp0..\tools\call_vsdevcmd.cmd"
if errorlevel 1 exit /b 1

cd /d C:\code\cuda_newton_fractal_clone\ui_app

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set TESTROOT=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\build_tests
set OUTCSV=%TESTROOT%\bench_explaino_variants.csv
set PTXLOG=%TESTROOT%\bench_explaino_variants_ptxas.log

if not exist "%TESTROOT%" mkdir "%TESTROOT%"

nvcc -allow-unsupported-compiler -O2 -std=c++17 ^
  -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 ^
  -Xcompiler "/EHsc /MD" -Xptxas -v ^
  -I. -I.\src ^
  .\src\fractal_sample_core.cu .\src\sample_tier_resolver.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\explaino_variant_benchmark.cpp .\tests\bench_explaino_variants.cu ^
  -o "%TESTROOT%\bench_explaino_variants.exe" > "%PTXLOG%" 2>&1
if errorlevel 1 (
  type "%PTXLOG%"
  exit /b 1
)

findstr /C:"Used " "%PTXLOG%"

"%TESTROOT%\bench_explaino_variants.exe" "%OUTCSV%"
if errorlevel 1 exit /b 1

echo Wrote benchmark CSV: %OUTCSV%
echo Wrote PTXAS log: %PTXLOG%
exit /b 0
