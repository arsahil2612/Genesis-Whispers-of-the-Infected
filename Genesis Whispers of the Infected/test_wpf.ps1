Add-Type -AssemblyName PresentationCore
$file = "Assets\Props\Level 2\Bagpack.png"
$uri = New-Object System.Uri((Resolve-Path $file).Path)
try {
    $decoder = [System.Windows.Media.Imaging.BitmapDecoder]::Create($uri, 'None', 'Default')
    $frame = $decoder.Frames[0]
    Write-Host "Success! WPF loaded the WebP file. Dimensions: $($frame.PixelWidth)x$($frame.PixelHeight)"
} catch {
    Write-Host "WPF Failed: $_"
}
