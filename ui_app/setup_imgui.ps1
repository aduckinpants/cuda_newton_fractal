param(
  [string]$OutDir = "$PSScriptRoot\\third_party\\imgui"
)

$ErrorActionPreference = 'Stop'

$zipUrl = "https://github.com/ocornut/imgui/archive/refs/heads/master.zip"
$zipPath = Join-Path $env:TEMP "imgui_master.zip"
$tmpExtract = Join-Path $env:TEMP "imgui_master_extract"

Write-Host "Downloading Dear ImGui from: $zipUrl"
Invoke-WebRequest -Uri $zipUrl -OutFile $zipPath

if (Test-Path $tmpExtract) {
  Remove-Item -Recurse -Force $tmpExtract
}
New-Item -ItemType Directory -Path $tmpExtract | Out-Null

Write-Host "Extracting..."
Expand-Archive -LiteralPath $zipPath -DestinationPath $tmpExtract -Force

$srcRoot = Get-ChildItem -LiteralPath $tmpExtract | Where-Object { $_.PSIsContainer } | Select-Object -First 1
if (-not $srcRoot) {
  throw "Could not locate extracted ImGui folder"
}

if (Test-Path $OutDir) {
  Remove-Item -Recurse -Force $OutDir
}
New-Item -ItemType Directory -Path $OutDir | Out-Null

Write-Host "Copying sources to: $OutDir"
Copy-Item -Recurse -Force (Join-Path $srcRoot.FullName '*') $OutDir

$required = @(
  "imgui.h",
  "imgui.cpp",
  "imgui_draw.cpp",
  "imgui_tables.cpp",
  "imgui_widgets.cpp",
  "backends\\imgui_impl_win32.cpp",
  "backends\\imgui_impl_dx11.cpp"
)
foreach ($r in $required) {
  $p = Join-Path $OutDir $r
  if (-not (Test-Path $p)) {
    throw "Missing required ImGui file: $r"
  }
}

Write-Host "OK: Dear ImGui staged."
