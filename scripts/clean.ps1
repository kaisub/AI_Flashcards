Write-Host "Cleaning build artifacts..."
if (Test-Path "build") {
    cmake --build build --target clean
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Clean failed. If your CMake cache is broken, you may need to manually delete the build folder."
    } else {
        Write-Host "Clean completed successfully. Downloaded dependencies were kept intact!"
    }
} else {
    Write-Host "Build directory does not exist."
}