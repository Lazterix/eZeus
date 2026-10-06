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
if(ub->type() != eBuildingType::godMonument &&
               !isPatrolSelected(ub) &&
               mViewMode != eViewMode::appeal) {
'@

if ($text.Contains($already)) {
    Write-Host "Already patched: god monument basement suppressed." -ForegroundColor DarkGray
    exit 0
}

$old = @'
        if(ub && !v) {
            if(!isPatrolSelected(ub) && mViewMode != eViewMode::appeal) {
                const auto tex = getBasementTexture(rtx, rty, ub, trrTexs,
                                                    dir, boardw, boardh);
                tp.drawTexture(rx, ry, tex, eAlignment::top);
            }
'@

$new = @'
        if(ub && !v) {
            // God monuments already contain their complete pedestal/base in the
            // monument sprite. Drawing the generic building basement underneath
            // exposes a dark diamond/outline around the custom remastered asset.
            if(ub->type() != eBuildingType::godMonument &&
               !isPatrolSelected(ub) &&
               mViewMode != eViewMode::appeal) {
                const auto tex = getBasementTexture(rtx, rty, ub, trrTexs,
                                                    dir, boardw, boardh);
                tp.drawTexture(rx, ry, tex, eAlignment::top);
            }
'@

$count = ([regex]::Matches($text, [regex]::Escape($old))).Count

if ($count -ne 1) {
    throw "Expected exactly one basement-render block, found $count. File was not modified."
}

$backup = "$Path.poseidon-basement-backup"
if (!(Test-Path -LiteralPath $backup)) {
    Copy-Item -LiteralPath $Path -Destination $backup
    Write-Host "Backup created: $backup" -ForegroundColor DarkGray
}

$text = $text.Replace($old, $new)
Set-Content -LiteralPath $Path -Value $text -NoNewline -Encoding utf8

Write-Host ""
Write-Host "SUCCESS: generic basement disabled for eBuildingType::godMonument." -ForegroundColor Green
Write-Host "The monument sprite itself is unchanged." -ForegroundColor Green
Write-Host ""
Write-Host "Next:"
Write-Host "  cmake --build .\build --config Release --parallel"
