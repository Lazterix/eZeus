param(
    [string]$ProjectRoot = "X:\Projects\eZeus-Extended"
)

$ErrorActionPreference = "Stop"

$Path = Join-Path $ProjectRoot "widgets\egamewidgetpaint.cpp"

if (!(Test-Path -LiteralPath $Path)) {
    throw "Missing file: $Path"
}

$text = Get-Content -LiteralPath $Path -Raw

$alreadyPattern = @'
if\s*\(\s*
ub->type\(\)\s*!=\s*eBuildingType::godMonument\s*&&\s*
!isPatrolSelected\s*\(\s*ub\s*\)\s*&&\s*
mViewMode\s*!=\s*eViewMode::appeal\s*
\)\s*\{
'@

if ([regex]::IsMatch(
        $text,
        $alreadyPattern,
        [System.Text.RegularExpressions.RegexOptions]::IgnorePatternWhitespace
    )) {
    Write-Host "Already patched: god monument basement is disabled." -ForegroundColor DarkGray
    exit 0
}

$pattern = @'
if\s*\(\s*
!isPatrolSelected\s*\(\s*ub\s*\)\s*&&\s*
mViewMode\s*!=\s*eViewMode::appeal\s*
\)\s*\{
'@

$regexOptions =
    [System.Text.RegularExpressions.RegexOptions]::IgnorePatternWhitespace

$matches = [regex]::Matches($text, $pattern, $regexOptions)

if ($matches.Count -lt 1) {
    Write-Host ""
    Write-Host "Could not find the expected basement condition." -ForegroundColor Red
    Write-Host "Showing every getBasementTexture occurrence for diagnosis:" -ForegroundColor Yellow
    Select-String -Path $Path -Pattern "getBasementTexture|isPatrolSelected|eViewMode::appeal" -Context 4,4
    throw "No safe patch target found. File was not modified."
}

# Prefer the match immediately before the basement texture call.
$target = $null

foreach ($m in $matches) {
    $afterStart = $m.Index
    $afterLen = [Math]::Min(900, $text.Length - $afterStart)
    $after = $text.Substring($afterStart, $afterLen)

    if ($after -match 'getBasementTexture\s*\(') {
        $target = $m
        break
    }
}

if ($null -eq $target) {
    throw "Found condition candidates, but none controls getBasementTexture(). File was not modified."
}

$backup = "$Path.poseidon-basement-backup"

if (!(Test-Path -LiteralPath $backup)) {
    Copy-Item -LiteralPath $Path -Destination $backup
    Write-Host "Backup created: $backup" -ForegroundColor DarkGray
}

$replacement = @'
if(ub->type() != eBuildingType::godMonument &&
               !isPatrolSelected(ub) &&
               mViewMode != eViewMode::appeal) {
'@

$newText =
    $text.Substring(0, $target.Index) +
    $replacement +
    $text.Substring($target.Index + $target.Length)

Set-Content -LiteralPath $Path -Value $newText -NoNewline -Encoding utf8

Write-Host ""
Write-Host "SUCCESS: disabled generic basement rendering for god monuments." -ForegroundColor Green
Write-Host ""

Select-String `
    -Path $Path `
    -Pattern "godMonument|isPatrolSelected\(ub\)|getBasementTexture" `
    -Context 1,3 |
    Select-Object -First 12

Write-Host ""
Write-Host "Next:"
Write-Host "  cmake --build .\build --config Release --parallel"
