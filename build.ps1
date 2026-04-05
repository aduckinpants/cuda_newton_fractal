$ErrorActionPreference = 'Stop'

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $here

Write-Host "Building in $here"

if ([string]::IsNullOrWhiteSpace($env:SALT_FRACTAL_ROOT)) {
  $env:SALT_FRACTAL_ROOT = 'D:\salt-fractal'
}

$outRoot = Join-Path $env:SALT_FRACTAL_ROOT 'cuda_newton_fractal_clone\smoke'
New-Item -ItemType Directory -Force -Path $outRoot | Out-Null

if (-not (Get-Command nvcc -ErrorAction SilentlyContinue)) {
  throw "nvcc not found on PATH. Install the CUDA Toolkit and re-open your terminal."
}

if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
  Write-Host "cl.exe not found on PATH."
  Write-Host "Run this build from a 'Developer PowerShell/Command Prompt for VS' (x64), or install the VS C++ workload + Windows SDK."
  Write-Host "You can run .\\doctor.py for a detailed checklist."
  throw "Missing MSVC build environment (cl.exe)."
}

nvcc -allow-unsupported-compiler .\newton_fractal.cu -O2 -std=c++17 -gencode=arch=compute_86,code=sm_86 -gencode=arch=compute_120,code=sm_120 -gencode=arch=compute_121,code=sm_121 -o (Join-Path $outRoot 'newton_fractal.exe')

Write-Host "Built $(Join-Path $outRoot 'newton_fractal.exe')"




