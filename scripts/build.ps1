Write-Host "========================================"
Write-Host "Configuring AI Flashcards..."
Write-Host "========================================"
cmake -S . -B build -G "Visual Studio 17 2022"
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake configuration failed."
    exit $LASTEXITCODE
}

Write-Host "`n========================================"
Write-Host "Building AI Flashcards..."
Write-Host "========================================"
cmake --build build --config Release --parallel
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed."
    exit $LASTEXITCODE
}

Write-Host "`nBuild completed successfully!"