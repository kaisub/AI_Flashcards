$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
& "$scriptDir\build.ps1"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "`n========================================"
Write-Host "Running Tests..."
Write-Host "========================================"
ctest --test-dir build -C Release --output-on-failure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }