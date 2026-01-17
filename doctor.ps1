$ErrorActionPreference = 'Continue'

function Write-Check {
  param(
    [string]$Name,
    [bool]$Ok,
    [string]$Details = '',
    [string]$Hint = ''
  )

  if ($Ok) {
    Write-Host ("[OK]   {0} {1}" -f $Name, $Details).TrimEnd()
  } else {
    Write-Host ("[FAIL] {0} {1}" -f $Name, $Details).TrimEnd()
    if ($Hint) { Write-Host ("       Hint: {0}" -f $Hint) }
  }
}

Write-Host "CUDA Newton Fractal doctor (PowerShell)"
Write-Host ("PowerShell: {0}" -f $PSVersionTable.PSVersion)
Write-Host ("Folder: {0}" -f $PSScriptRoot)
Write-Host ""

$nvccCmd = Get-Command nvcc -ErrorAction SilentlyContinue
$nvccPath = ''
if ($nvccCmd) { $nvccPath = $nvccCmd.Source }
Write-Check -Name 'nvcc on PATH' -Ok ([bool]$nvccCmd) -Details $nvccPath -Hint 'Install CUDA Toolkit and ensure nvcc.exe is on PATH.'
if ($nvccCmd) {
  try {
    $ver = & nvcc --version 2>$null
    $ver | Select-Object -Last 4 | ForEach-Object { Write-Host ("       {0}" -f $_) }
  } catch { }
}

Write-Host ""
$clCmd = Get-Command cl -ErrorAction SilentlyContinue
$clPath = ''
if ($clCmd) { $clPath = $clCmd.Source }
Write-Check -Name 'cl.exe on PATH' -Ok ([bool]$clCmd) -Details $clPath -Hint 'Install MSVC (Desktop development with C++) and run from a VS Developer Command Prompt/PowerShell.'

$vsCandidates = @(
  'C:\Program Files\Microsoft Visual Studio\18\Community',
  'C:\Program Files\Microsoft Visual Studio\2022\Community',
  'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools'
)

$vsFound = $null
foreach ($c in $vsCandidates) {
  if (Test-Path $c) { $vsFound = $c; break }
}

Write-Host ""
Write-Check -Name 'VS/BuildTools folder' -Ok ([bool]$vsFound) -Details ($vsFound) -Hint 'Install Visual Studio Community or Visual Studio Build Tools.'

if ($vsFound) {
  $vcvarsall = Join-Path $vsFound 'VC\Auxiliary\Build\vcvarsall.bat'
  Write-Check -Name 'vcvarsall.bat present' -Ok (Test-Path $vcvarsall) -Details $vcvarsall -Hint 'Your VS install looks incomplete; add the C++ workload (MSVC) via Visual Studio Installer.'

  $msvcTools = Join-Path $vsFound 'VC\Tools\MSVC'
  $msvcVersion = Get-ChildItem $msvcTools -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending | Select-Object -First 1
  $msvcVersionPath = ''
  if ($msvcVersion) { $msvcVersionPath = $msvcVersion.FullName }
  Write-Check -Name 'MSVC toolset folder' -Ok ([bool]$msvcVersion) -Details $msvcVersionPath -Hint 'Install an MSVC toolset (v14x) via Visual Studio Installer.'

  if ($msvcVersion) {
    $msvcInclude = Join-Path $msvcVersion.FullName 'include'
    $msvcLibX64 = Join-Path $msvcVersion.FullName 'lib\x64'
    Write-Check -Name 'MSVC include dir' -Ok (Test-Path $msvcInclude) -Details $msvcInclude -Hint 'MSVC headers missing; install the MSVC toolset.'
    Write-Check -Name 'MSVC x64 libs' -Ok (Test-Path $msvcLibX64) -Details $msvcLibX64 -Hint 'MSVC libs missing; install the MSVC toolset.'
  }
}

Write-Host ""
$kits10 = 'C:\Program Files (x86)\Windows Kits\10\Include'
Write-Check -Name 'Windows SDK (Kits\10\Include)' -Ok (Test-Path $kits10) -Details $kits10 -Hint 'Install a Windows 10/11 SDK via Visual Studio Installer (Individual components).'

Write-Host ""
Write-Host "If any checks failed:"
Write-Host "  1) Open 'Visual Studio Installer' -> Modify -> select 'Desktop development with C++'"
Write-Host "  2) Ensure a Windows 10/11 SDK component is selected"
Write-Host "  3) Re-open a 'x64 Native Tools Command Prompt for VS' and run: .\\build.ps1"
