$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
& "$scriptDir\build_and_test.ps1"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "`n========================================"
Write-Host "Running AI Flashcards..."
Write-Host "========================================"
if (Test-Path ".\build\Release\flashcards_app.exe") {
    & ".\build\Release\flashcards_app.exe"
} elseif (Test-Path ".\build\flashcards_app.exe") {
    & ".\build\flashcards_app.exe"
} else {
    Write-Error "Executable not found!"
}