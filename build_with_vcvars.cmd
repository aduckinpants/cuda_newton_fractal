@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
  echo vcvars64.bat failed
  exit /b 1
)
where cl
if errorlevel 1 (
  echo cl.exe not found after vcvars
  exit /b 1
)
"%CUDA_PATH%\bin\nvcc.exe" -O2 -std=c++17 -arch=sm_70 -lineinfo -o newton_fractal.exe newton_fractal.cu
exit /b %errorlevel%
