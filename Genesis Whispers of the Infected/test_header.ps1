$bytes = [System.IO.File]::ReadAllBytes('Assets\Props\Level 2\Bagpack.png')
$header = [System.Text.Encoding]::ASCII.GetString($bytes, 0, 16)
Write-Host "Header: $header"
$hex = [System.BitConverter]::ToString($bytes, 0, 16)
Write-Host "Hex: $hex"
