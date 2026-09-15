Add-Type -AssemblyName System.Drawing
$files = Get-ChildItem -Path "Assets\Props\Level 2\*.png"
foreach ($file in $files) {
    Write-Host "Resaving $($file.Name)..."
    $img = [System.Drawing.Image]::FromFile($file.FullName)
    $bmp = new-object System.Drawing.Bitmap($img)
    $img.Dispose()
    $bmp.Save($file.FullName, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
}
Write-Host "Done!"
