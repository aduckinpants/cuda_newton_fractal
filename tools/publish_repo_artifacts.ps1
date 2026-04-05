param(
    [string]$PublishRoot = "D:\salt-fractal",
    [string]$Label = "",
    [switch]$WhatIf
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$repoName = Split-Path -Leaf $repoRoot
$stamp = Get-Date -Format "yyyy-MM-dd_HHmmss"
if ([string]::IsNullOrWhiteSpace($Label)) {
    $Label = "artifact_publish"
}

$destinationRoot = Join-Path $PublishRoot $repoName
$publishDir = Join-Path $destinationRoot ("published\" + $stamp + "_" + $Label)

New-Item -ItemType Directory -Force -Path $publishDir | Out-Null

$moveSpecs = @(
    @{ Source = Join-Path $repoRoot "artifacts"; Destination = Join-Path $publishDir "artifacts" },
    @{ Source = Join-Path $repoRoot "ui\diagnostics"; Destination = Join-Path $publishDir "ui_diagnostics" },
    @{ Source = Join-Path $repoRoot "ui_app\build"; Destination = Join-Path $publishDir "ui_app_build" },
    @{ Source = Join-Path $repoRoot "ui_app\build_tests"; Destination = Join-Path $publishDir "ui_app_build_tests" },
    @{ Source = Join-Path $repoRoot "ui_app\ui"; Destination = Join-Path $publishDir "ui_app_runtime_ui" }
)

$moveFiles = @(
    @{ Source = Join-Path $repoRoot "newton.ppm"; Destination = Join-Path $publishDir "root_outputs\newton.ppm" },
    @{ Source = Join-Path $repoRoot "newton_driver_updated.ppm"; Destination = Join-Path $publishDir "root_outputs\newton_driver_updated.ppm" },
    @{ Source = Join-Path $repoRoot "newton_fractal.exe"; Destination = Join-Path $publishDir "root_outputs\newton_fractal.exe" },
    @{ Source = Join-Path $repoRoot "newton_fractal_fresh.exe"; Destination = Join-Path $publishDir "root_outputs\newton_fractal_fresh.exe" },
    @{ Source = Join-Path $repoRoot "imgui.ini"; Destination = Join-Path $publishDir "root_outputs\imgui.ini" },
    @{ Source = Join-Path $repoRoot "_build_tmp.cmd"; Destination = Join-Path $publishDir "root_outputs\_build_tmp.cmd" },
    @{ Source = Join-Path $repoRoot "ui_app\fractal_ui.exe"; Destination = Join-Path $publishDir "ui_app_runtime\fractal_ui.exe" },
    @{ Source = Join-Path $repoRoot "ui_app\imgui.ini"; Destination = Join-Path $publishDir "ui_app_runtime\imgui.ini" }
)

$uiAppLooseObjects = Get-ChildItem -Path (Join-Path $repoRoot "ui_app") -Filter *.obj -ErrorAction SilentlyContinue
foreach ($file in $uiAppLooseObjects) {
    $moveFiles += @{ Source = $file.FullName; Destination = Join-Path $publishDir ("ui_app_runtime\" + $file.Name) }
}

foreach ($spec in $moveSpecs) {
    if (-not (Test-Path $spec.Source)) {
        continue
    }

    $destParent = Split-Path -Parent $spec.Destination
    New-Item -ItemType Directory -Force -Path $destParent | Out-Null
    if ($WhatIf) {
        Write-Host "WHATIF move dir $($spec.Source) -> $($spec.Destination)"
    }
    else {
        Move-Item -Force $spec.Source $spec.Destination
    }
}

foreach ($spec in $moveFiles) {
    if (-not (Test-Path $spec.Source)) {
        continue
    }

    $destParent = Split-Path -Parent $spec.Destination
    New-Item -ItemType Directory -Force -Path $destParent | Out-Null
    if ($WhatIf) {
        Write-Host "WHATIF move file $($spec.Source) -> $($spec.Destination)"
    }
    else {
        Move-Item -Force $spec.Source $spec.Destination
    }
}

$manifest = [ordered]@{
    repo_root = $repoRoot
    publish_root = $PublishRoot
    publish_dir = $publishDir
    timestamp = $stamp
    label = $Label
}

$manifestPath = Join-Path $publishDir "publish_manifest.json"
$manifest | ConvertTo-Json -Depth 4 | Set-Content -Encoding UTF8 $manifestPath

Write-Host $publishDir