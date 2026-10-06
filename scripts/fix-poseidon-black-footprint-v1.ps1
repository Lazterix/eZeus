param(
    [string]$ProjectRoot = "X:\Projects\eZeus-Extended"
)

$ErrorActionPreference = "Stop"

$Path = Join-Path $ProjectRoot "widgets\egamewidgetpaint.cpp"

if (!(Test-Path -LiteralPath $Path)) {
    throw "Missing file: $Path"
}

$text = Get-Content -LiteralPath $Path -Raw

$alreadyPattern = 'eBuilding::sFlatBuilding\s*\(\s*terrBt\s*\)\s*\|\|\s*terrBt\s*==\s*eBuildingType::godMonument'

if ([regex]::IsMatch($text, $alreadyPattern)) {
    Write-Host "Already patched: terrain is drawn under god monuments." -ForegroundColor DarkGray
    exit 0
}

$pattern = @'
if\s*\(\s*!terrUb\s*\|\|\s*flatSanct\s*\|\|\s*
eBuilding::sFlatBuilding\s*\(\s*terrBt\s*\)\s*\)\s*\{
'@

$matches = [regex]::Matches(
    $text,
    $pattern,
    [System.Text.RegularExpressions.RegexOptions]::IgnorePatternWhitespace
)

if ($matches.Count -ne 1) {
    Write-Host ""
    Write-Host "Could not safely locate the terrain-under-building condition." -ForegroundColor Red
    Write-Host "Relevant lines:" -ForegroundColor Yellow
    Select-String -Path $Path -Pattern "flatSanct|sFlatBuilding\(terrBt\)|drawTerrain\(tile\)" -Context 2,2
    throw "Expected exactly one patch target, found $($matches.Count). File was not modified."
}

$backup = "$Path.before-poseidon-terrain-underlay-fix"

if (!(Test-Path -LiteralPath $backup)) {
    Copy-Item -LiteralPath $Path -Destination $backup
    Write-Host "Backup created: $backup" -ForegroundColor DarkGray
}

$replacement = @'
if(!terrUb || flatSanct ||
               eBuilding::sFlatBuilding(terrBt) ||
               terrBt == eBuildingType::godMonument) {
'@

$text = [regex]::Replace(
    $text,
    $pattern,
    $replacement,
    [System.Text.RegularExpressions.RegexOptions]::IgnorePatternWhitespace
)

Set-Content -LiteralPath $Path -Value $text -NoNewline -Encoding utf8

Write-Host ""
Write-Host "SUCCESS: terrain will now render underneath god monuments." -ForegroundColor Green
Write-Host "This fills the previously undrawn 2x2 footprint that appeared as a black diamond/rim." -ForegroundColor Green
Write-Host "The Poseidon PNG itself is not modified." -ForegroundColor Green
Write-Host ""
Write-Host "Next:"
Write-Host "  cmake --build .\build --config Release --parallel"
