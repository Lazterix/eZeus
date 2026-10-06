param(
    [string]$ProjectRoot = "X:\Projects\eZeus-Extended"
)

$ErrorActionPreference = "Stop"

$Path = Join-Path $ProjectRoot "widgets\egamewidgetpaint.cpp"

if (!(Test-Path -LiteralPath $Path)) {
    throw "Missing file: $Path"
}

$text = Get-Content -LiteralPath $Path -Raw

$already = @'
            const bool drawTerrainUnderBuilding =
                    eBuilding::sFlatBuilding(terrBt) ||
                    terrBt == eBuildingType::godMonument ||
                    terrBt == eBuildingType::laurelGarden ||
                    terrBt == eBuildingType::saltWorks;
'@

if ($text.Contains($already)) {
    Write-Host "Already patched: terrain is drawn under god monuments." -ForegroundColor DarkGray
    exit 0
}

$old = @'
            const bool drawTerrainUnderBuilding =
                    eBuilding::sFlatBuilding(terrBt) ||
                    terrBt == eBuildingType::laurelGarden ||
                    terrBt == eBuildingType::saltWorks;
'@

$new = @'
            const bool drawTerrainUnderBuilding =
                    eBuilding::sFlatBuilding(terrBt) ||
                    terrBt == eBuildingType::godMonument ||
                    terrBt == eBuildingType::laurelGarden ||
                    terrBt == eBuildingType::saltWorks;
'@

$count = ([regex]::Matches($text, [regex]::Escape($old))).Count

if ($count -ne 1) {
    Write-Host ""
    Write-Host "Could not locate the exact current drawTerrainUnderBuilding block." -ForegroundColor Red
    Write-Host "Relevant lines:" -ForegroundColor Yellow
    Select-String -Path $Path -Pattern "drawTerrainUnderBuilding|laurelGarden|saltWorks|godMonument" -Context 2,2
    throw "Expected exactly one current block, found $count. File was not modified."
}

$backup = "$Path.before-godmonument-terrain-underlay"
if (!(Test-Path -LiteralPath $backup)) {
    Copy-Item -LiteralPath $Path -Destination $backup
    Write-Host "Backup created: $backup" -ForegroundColor DarkGray
}

$text = $text.Replace($old, $new)
Set-Content -LiteralPath $Path -Value $text -NoNewline -Encoding utf8

Write-Host ""
Write-Host "SUCCESS: eBuildingType::godMonument added to drawTerrainUnderBuilding." -ForegroundColor Green
Write-Host ""
Write-Host "Verification:" -ForegroundColor Cyan
Select-String `
    -Path $Path `
    -Pattern "drawTerrainUnderBuilding|eBuildingType::godMonument|laurelGarden|saltWorks" `
    -Context 0,3 |
    Select-Object -First 12
