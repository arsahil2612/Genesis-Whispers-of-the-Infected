Add-Type -AssemblyName PresentationCore

$files = Get-ChildItem -Path "Assets\Props\Level 2\*.png"
foreach ($file in $files) {
    Write-Host "Converting $($file.Name)..."
    $uri = New-Object System.Uri($file.FullName)
    try {
        $decoder = [System.Windows.Media.Imaging.BitmapDecoder]::Create($uri, 'None', 'Default')
        $frame = $decoder.Frames[0]
        
        $encoder = New-Object System.Windows.Media.Imaging.PngBitmapEncoder
        $encoder.Frames.Add($frame)
        
        $tempFile = $file.FullName + ".tmp"
        $fs = [System.IO.File]::OpenWrite($tempFile)
        $encoder.Save($fs)
        $fs.Close()
        
        # Explicit garbage collection and decoder variable clearing to release file locks
        $decoder = $null
        $frame = $null
        [System.GC]::Collect()
        [System.GC]::WaitForPendingFinalizers()

        Move-Item -Path $tempFile -Destination $file.FullName -Force
        Write-Host "Success!"
    } catch {
        Write-Host "Failed to convert $($file.Name): $_"
    }
}
Write-Host "Done!"
