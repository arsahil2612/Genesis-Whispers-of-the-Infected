Add-Type -AssemblyName System.Drawing
$img1 = [System.Drawing.Image]::FromFile('Assets/UI/HUD/survival_data_panel.png')
Write-Host "Survival Panel: $($img1.Width)x$($img1.Height)"
$img2 = [System.Drawing.Image]::FromFile('Assets/UI/Mission/mission_panel.png')
Write-Host "Mission Panel: $($img2.Width)x$($img2.Height)"
