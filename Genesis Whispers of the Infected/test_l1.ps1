Add-Type -AssemblyName System.Drawing
$file = "Assets\Props\Level 1\Furniture\furn_broken_chair_01.png"
Write-Host "Testing $file..."
try {
    $img = [System.Drawing.Image]::FromFile($file)
    Write-Host "Success! Pixel Format: $($img.PixelFormat)"
    $img.Dispose()
} catch {
    Write-Host "Failed: $_"
}
