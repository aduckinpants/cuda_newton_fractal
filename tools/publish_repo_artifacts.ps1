param(
    [string]$PublishRoot = "D:\salt-fractal",
    [string]$Label = "",
    [switch]$WhatIf
)

$ErrorActionPreference = "Stop"

# Disk-pressure-safe publisher shim. Keep this wrapper tiny; the tested manifest and cleanup rules live in tools/viewer_host_deploy_disk_pressure.py.
$repoRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($Label)) {
    $Label = "artifact_publish"
}
$helper = Join-Path $repoRoot "tools\viewer_host_deploy_disk_pressure.py"
$argsList = @(
    "-3.14",
    $helper,
    "publish",
    "--repo-root",
    $repoRoot,
    "--publish-root",
    $PublishRoot,
    "--label",
    $Label
)
if (-not $WhatIf) {
    $argsList += "--execute"
}
& py @argsList
exit $LASTEXITCODE
