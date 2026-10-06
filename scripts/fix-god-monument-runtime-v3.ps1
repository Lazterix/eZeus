param(
    [string]$ProjectRoot = "X:\Projects\eZeus-Extended"
)

$ErrorActionPreference = "Stop"

$Aesthetics = Join-Path $ProjectRoot "buildings\eaestheticsbuilding.cpp"
$BuildFile  = Join-Path $ProjectRoot "widgets\egamewidgetbuild.cpp"

foreach ($p in @($Aesthetics, $BuildFile)) {
    if (!(Test-Path -LiteralPath $p)) {
        throw "Missing file: $p"
    }
}

function Ensure-Backup {
    param([string]$Path)
    $backup = "$Path.poseidon-backup"
    if (!(Test-Path -LiteralPath $backup)) {
        Copy-Item -LiteralPath $Path -Destination $backup
        Write-Host "Backup created: $backup" -ForegroundColor DarkGray
    }
}

function Replace-RegexOnce {
    param(
        [string]$Path,
        [string]$Pattern,
        [string]$Replacement,
        [string]$Label,
        [string]$AlreadyPattern = ""
    )

    $text = Get-Content -LiteralPath $Path -Raw

    if ($AlreadyPattern -and [regex]::IsMatch(
            $text,
            $AlreadyPattern,
            [System.Text.RegularExpressions.RegexOptions]::Singleline)) {
        Write-Host "Already patched: $Label" -ForegroundColor DarkGray
        return
    }

    $matches = [regex]::Matches(
        $text,
        $Pattern,
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )

    if ($matches.Count -ne 1) {
        throw ("{0}: expected exactly 1 match in {1}, found {2}. No write performed." -f `
            $Label, $Path, $matches.Count)
    }

    Ensure-Backup -Path $Path

    $newText = [regex]::Replace(
        $text,
        $Pattern,
        $Replacement,
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )

    Set-Content -LiteralPath $Path -Value $newText -NoNewline -Encoding utf8
    Write-Host "Patched: $Label" -ForegroundColor Green
}

Write-Host "`n=== GOD MONUMENT RUNTIME FIX v3 ===" -ForegroundColor Cyan

# ---------------------------------------------------------------------------
# 1) Explicit board include for direction() / eWorldDirection.
# ---------------------------------------------------------------------------
$aText = Get-Content -LiteralPath $Aesthetics -Raw
if ($aText -notmatch '#include\s+"engine/egameboard\.h"') {
    $includePattern = '#include\s+"textures/egametextures\.h"'
    if (-not [regex]::IsMatch($aText, $includePattern)) {
        throw "Could not find egametextures include insertion point in $Aesthetics"
    }

    Ensure-Backup -Path $Aesthetics
    $aText = [regex]::Replace(
        $aText,
        $includePattern,
        "#include `"textures/egametextures.h`"`r`n#include `"engine/egameboard.h`"",
        [System.Text.RegularExpressions.RegexOptions]::None
    )
    Set-Content -LiteralPath $Aesthetics -Value $aText -NoNewline -Encoding utf8
    Write-Host "Patched: explicit egameboard include" -ForegroundColor Green
} else {
    Write-Host "Already patched: explicit egameboard include" -ForegroundColor DarkGray
}

# ---------------------------------------------------------------------------
# 2) Safe monument erase.
# ---------------------------------------------------------------------------
$erasePattern = @'
void\s+eGodMonument::erase\s*\(\s*\)\s*\{\s*
for\s*\(\s*const\s+auto&?\s+t\s*:\s*mTiles\s*\)\s*\{\s*
t->eBuilding::erase\s*\(\s*\)\s*;\s*
\}\s*
eBuilding::erase\s*\(\s*\)\s*;\s*
\}
'@

$eraseReplacement = @'
void eGodMonument::erase() {
    // Detach the auxiliary 4x4 surround tiles before scheduling deletion.
    // They store a raw pointer back to this monument.
    const auto tiles = mTiles;
    mTiles.clear();

    for(const auto t : tiles) {
        if(!t) continue;
        t->setMonument(nullptr);
        t->eBuilding::erase();
    }

    eBuilding::erase();
}
'@

Replace-RegexOnce `
    -Path $Aesthetics `
    -Pattern $erasePattern `
    -Replacement $eraseReplacement `
    -Label "safe god-monument erase" `
    -AlreadyPattern 'const\s+auto\s+tiles\s*=\s*mTiles\s*;\s*mTiles\.clear\s*\(\s*\)'

# ---------------------------------------------------------------------------
# 3) Use the four directional monument sprites.
#    eWorldDirection enum in eZeus is N, W, S, E.
# ---------------------------------------------------------------------------
$texturePattern = @'
std::shared_ptr<eTexture>\s+eGodMonument::getTexture\s*\(\s*const\s+eTileSize\s+size\s*\)\s+const\s*\{\s*
const\s+auto\s+coll\s*=\s*eTempleMonumentBuilding::sGodMonumentTextureCollection\s*\(\s*size\s*,\s*mGod\s*\)\s*;\s*
return\s+coll->getTexture\s*\(\s*1\s*\)\s*;\s*
\}
'@

$textureReplacement = @'
std::shared_ptr<eTexture> eGodMonument::getTexture(const eTileSize size) const {
    const auto coll =
        eTempleMonumentBuilding::sGodMonumentTextureCollection(size, mGod);
    if(!coll) return nullptr;

    int dirId = 0;
    switch(getBoard().direction()) {
    case eWorldDirection::N:
        dirId = 0;
        break;
    case eWorldDirection::W:
        dirId = 1;
        break;
    case eWorldDirection::S:
        dirId = 2;
        break;
    case eWorldDirection::E:
        dirId = 3;
        break;
    }

    return coll->getTexture(dirId);
}
'@

Replace-RegexOnce `
    -Path $Aesthetics `
    -Pattern $texturePattern `
    -Replacement $textureReplacement `
    -Label "directional god-monument sprites" `
    -AlreadyPattern 'switch\s*\(\s*getBoard\(\)\.direction\(\)\s*\)'

# ---------------------------------------------------------------------------
# 4) Safe erase when clicking a surround tile.
# ---------------------------------------------------------------------------
$tileErasePattern = @'
void\s+eGodMonumentTile::erase\s*\(\s*\)\s*\{\s*
mMonument->erase\s*\(\s*\)\s*;\s*
\}
'@

$tileEraseReplacement = @'
void eGodMonumentTile::erase() {
    const auto monument = mMonument;
    mMonument = nullptr;

    if(monument) {
        monument->erase();
    } else {
        eBuilding::erase();
    }
}
'@

Replace-RegexOnce `
    -Path $Aesthetics `
    -Pattern $tileErasePattern `
    -Replacement $tileEraseReplacement `
    -Label "safe god-monument surround-tile erase" `
    -AlreadyPattern 'const\s+auto\s+monument\s*=\s*mMonument\s*;\s*mMonument\s*=\s*nullptr'

# ---------------------------------------------------------------------------
# 5) Validate the ACTUAL 4x4 footprint, not tx/ty shifted footprint.
# ---------------------------------------------------------------------------
$buildText = Get-Content -LiteralPath $BuildFile -Raw

if ($buildText -match 'mBoard->canBuild\s*\(\s*tminX\s*,\s*tminY\s*,\s*4\s*,\s*4') {
    Write-Host "Already patched: god-monument 4x4 footprint validation" -ForegroundColor DarkGray
} else {
    $canBuildPattern = 'const\s+bool\s+cb\s*=\s*mBoard->canBuild\s*\(\s*tx\s*,\s*ty\s*,\s*4\s*,\s*4\s*,\s*mEditorMode\s*,\s*cid\s*,\s*pid\s*\)\s*;'
    $matches = [regex]::Matches($buildText, $canBuildPattern)

    if ($matches.Count -ne 1) {
        throw ("god-monument 4x4 footprint validation: expected exactly 1 match in {0}, found {1}." -f `
            $BuildFile, $matches.Count)
    }

    Ensure-Backup -Path $BuildFile
    $buildText = [regex]::Replace(
        $buildText,
        $canBuildPattern,
        'const bool cb = mBoard->canBuild(tminX, tminY, 4, 4, mEditorMode, cid, pid);'
    )
    Set-Content -LiteralPath $BuildFile -Value $buildText -NoNewline -Encoding utf8
    Write-Host "Patched: god-monument 4x4 footprint validation" -ForegroundColor Green
}

# ---------------------------------------------------------------------------
# 6) Do not create surround tiles unless the central monument build succeeded.
# ---------------------------------------------------------------------------
$buildText = Get-Content -LiteralPath $BuildFile -Raw

if ($buildText -match 'if\s*\(\s*!b\s*\)\s*return\s+true\s*;\s*for\s*\(\s*int\s+x\s*=\s*tminX') {
    Write-Host "Already patched: prevent orphan monument surround tiles" -ForegroundColor DarkGray
} else {
    $centralBuildPattern = @'
(const\s+bool\s+b\s*=\s*mBoard->build\s*\(\s*tminX\s*\+\s*1\s*,\s*tminY\s*\+\s*2\s*,\s*2\s*,\s*2\s*,\s*cid\s*,\s*pid\s*,\s*mEditorMode\s*,\s*\[&\]\(\)\s*\{\s*
return\s+s\s*;\s*
\}\s*\)\s*;\s*)
(for\s*\(\s*int\s+x\s*=\s*tminX)
'@

    $matches = [regex]::Matches(
        $buildText,
        $centralBuildPattern,
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )

    if ($matches.Count -ne 1) {
        throw ("prevent orphan monument surround tiles: expected exactly 1 match in {0}, found {1}." -f `
            $BuildFile, $matches.Count)
    }

    Ensure-Backup -Path $BuildFile

    $replacement = @'
$1            if(!b) return true;

            $2
'@

    $buildText = [regex]::Replace(
        $buildText,
        $centralBuildPattern,
        $replacement,
        [System.Text.RegularExpressions.RegexOptions]::Singleline
    )

    Set-Content -LiteralPath $BuildFile -Value $buildText -NoNewline -Encoding utf8
    Write-Host "Patched: prevent orphan monument surround tiles" -ForegroundColor Green
}

Write-Host "`n=== VERIFY ===" -ForegroundColor Cyan

Select-String `
    -Path $Aesthetics `
    -Pattern "const auto tiles = mTiles|switch\(getBoard\(\)\.direction\(\)\)|const auto monument = mMonument" |
    ForEach-Object { Write-Host $_.Line.Trim() }

Select-String `
    -Path $BuildFile `
    -Pattern "canBuild\(tminX, tminY, 4, 4|if\(!b\) return true" |
    ForEach-Object { Write-Host $_.Line.Trim() }

Write-Host "`nGod monument runtime fixes applied successfully." -ForegroundColor Cyan
Write-Host "Next:"
Write-Host "  cmake --build .\build --config Release --parallel"
