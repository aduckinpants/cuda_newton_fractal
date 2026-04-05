@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
if errorlevel 1 exit /b 1
where cl
if errorlevel 1 (
  echo cl.exe still not found after VsDevCmd
  exit /b 1
)

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set OUTROOT=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\smoke

cd /d C:\code\cuda_newton_fractal_clone
if not exist "%OUTROOT%" mkdir "%OUTROOT%"

nvcc -allow-unsupported-compiler .\newton_fractal.cu -O2 -std=c++17 -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 -o "%OUTROOT%\newton_fractal.exe"
exit /b %errorlevel%



