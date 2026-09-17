param()
$dir = "Assets\Props\Level 2"

# Ensure we're in the right directory
if (-not (Test-Path $dir)) { Write-Error "Dir not found"; exit 1 }

# Create subdirectories
$folders = @('Buildings','Decorations','Furniture','Military','Nature','Vehicles','Items')
foreach ($f in $folders) {
    if (-not (Test-Path "$dir\$f")) { New-Item -ItemType Directory -Path "$dir\$f" | Out-Null }
}

# Define mapping
$map = @{
    "Bagpack.png.png" = "Items\Bagpack.png"
    "Genesis_Specimen_Container.png.png" = "Decorations\Genesis_Specimen_Container.png"
    "NovaGen_Containment_Chamber_1024_Transparent.png.png" = "Decorations\NovaGen_Containment_Chamber_1024_Transparent.png"
    "NovaGen_Laboratory_Computer_Terminal.png.png" = "Furniture\NovaGen_Laboratory_Computer_Terminal.png"
    "Weathered_Olive_Military_Folding_Table.png.png" = "Furniture\Weathered_Olive_Military_Folding_Table.png"
    "Abandoned_Medical_Examination_Machine.png.png" = "Furniture\Abandoned_Medical_Examination_Machine.png"
    "Military_Field_Tent.png.png" = "Military\Military_Field_Tent.png"
    "Military_Portable_Generator.png.png" = "Military\Military_Portable_Generator.png"
    "Military_Supply_Crate.png.png" = "Military\Military_Supply_Crate.png"
    "Military_Survival_Water_Jerrycan .png.png" = "Items\Military_Survival_Water_Jerrycan .png"
    "Burning_Wrecked_Military_SUV.png.png" = "Vehicles\Burning_Wrecked_Military_SUV.png"
    "Military_Supply_Vehicle.png.png" = "Vehicles\Military_Supply_Vehicle.png"
    "Broken_Bridge_Plank_1024_Transparent.png.png" = "Nature\Broken_Bridge_Plank_1024_Transparent.png"
    "Mossy_Fallen_Log_Asset.png.png" = "Nature\Mossy_Fallen_Log_Asset.png"
    "Evacuation_Route_Sign.png.png" = "Decorations\Evacuation_Route_Sign.png"
    "Facility_Direction_Sign_Transparent.png.png" = "Decorations\Facility_Direction_Sign_Transparent.png"
    "Weathered_Military_Road_Barricade.png.png" = "Military\Weathered_Military_Road_Barricade.png"
}

# Delete bad png files
Get-ChildItem -Path $dir -Filter "*.png" | Where-Object { $_.Name -notmatch '\.png\.png$' } | Remove-Item -Force -ErrorAction SilentlyContinue

# Move and rename
foreach ($key in $map.Keys) {
    $src = "$dir\$key"
    $dst = "$dir\$($map[$key])"
    if (Test-Path $src) {
        Move-Item -Path $src -Destination $dst -Force
    }
}
Write-Host "Done"
