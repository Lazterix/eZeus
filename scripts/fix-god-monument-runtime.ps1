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

function Replace-Exact {
    param(
        [string]$Path,
        [string]$Old,
        [string]$New,
        [string]$Label
    )

    $text = Get-Content -LiteralPath $Path -Raw
    $count = ([regex]::Matches($text, [regex]::Escape($Old))).Count
    if ($count -ne 1) {
        throw "${Label}: expected exactly 1 match in ${Path}, found $count. No write performed."
    }

    if (!(Test-Path -LiteralPath "$Path.poseidon-backup")) {
        Copy-Item -LiteralPath $Path -Destination "$Path.poseidon-backup"
    }

    $text = $text.Replace($Old, $New)
    Set-Content -LiteralPath $Path -Value $text -NoNewline -Encoding utf8
    Write-Host "Patched: $Label" -ForegroundColor Green
}

# Explicit include for eWorldDirection / board direction use.
$aText = Get-Content -LiteralPath $Aesthetics -Raw
if ($aText -notmatch '#include "engine/egameboard.h"') {
    $needle = '#include "textures/egametextures.h"'
    if ($aText -notmatch [regex]::Escape($needle)) {
        throw "Could not find include insertion point in $Aesthetics"
    }
    if (!(Test-Path -LiteralPath "$Aesthetics.poseidon-backup")) {
        Copy-Item -LiteralPath $Aesthetics -Destination "$Aesthetics.poseidon-backup"
    }
    $aText = $aText.Replace($needle, "$needle`r`n#include `"engine/egameboard.h`"")
    Set-Content -LiteralPath $Aesthetics -Value $aText -NoNewline -Encoding utf8
    Write-Host "Patched: explicit egameboard include" -ForegroundColor Green
}

$oldErase = @'
void eGodMonument::erase() {
    for(const auto& t : mTiles) {
        t->eBuilding::erase();
    }
    eBuilding::erase();
}
'@

$newErase = @'
void eGodMonument::erase() {
    // Detach the auxiliary 4x4 surround tiles before scheduling deletion.
    // They store a raw pointer back to this monument, so leaving that pointer
    // alive after the monument is deleted creates a dangling-pointer crash.
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

Replace-Exact -Path $Aesthetics -Old $oldErase -New $newErase -Label "safe god-monument erase"

$oldTexture = @'
std::shared_ptr<eTexture> eGodMonument::getTexture(const eTileSize size) const {
    const auto coll = eTempleMonumentBuilding::sGodMonumentTextureCollection(size, mGod);
    return coll->getTexture(1);
}
'@

$newTexture = @'
std::shared_ptr<eTexture> eGodMonument::getTexture(const eTileSize size) const {
    const auto coll =
        eTempleMonumentBuilding::sGodMonumentTextureCollection(size, mGod);
    if(!coll) return nullptr;

    // God monument atlases contain four directional sprites.
    // eWorldDirection is ordered N, W, S, E in eZeus.
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

Replace-Exact -Path $Aesthetics -Old $oldTexture -New $newTexture -Label "directional god-monument sprites"

$oldTileErase = @'
void eGodMonumentTile::erase() {
    mMonument->erase();
}
'@

$newTileErase = @'
void eGodMonumentTile::erase() {
    // A surround tile may outlive its monument for a deferred-delete frame.
    // Never dereference a stale/null monument pointer.
    const auto monument = mMonument;
    mMonument = nullptr;

    if(monument) {
        monument->erase();
    } else {
        eBuilding::erase();
    }
}
'@

Replace-Exact -Path $Aesthetics -Old $oldTileErase -New $newTileErase -Label "safe god-monument surround-tile erase"

$oldBuild = @'
            const bool cb = mBoard->canBuild(tx, ty, 4, 4, mEditorMode, cid, pid);
            if(!cb) return true;

            const auto am = eBuildingMode::aphroditeMonument;
            const int id = static_cast<int>(mode) -
                           static_cast<int>(am);
            const auto gt = static_cast<eGodType>(id);
            const auto s = e::make_shared<eGodMonument>(
                               gt, eGodQuestId::godQuest1, *mBoard, mViewedCityId);
            const bool b = mBoard->build(tminX + 1, tminY + 2, 2, 2, cid, pid, mEditorMode, [&]() {
                return s;
            });
            for(int x = tminX; x < tmaxX; x++) {
                for(int y = tminY; y < tmaxY; y++) {
                    const bool cb = mBoard->canBuild(x, y, 1, 1, mEditorMode, cid, pid);
                    if(!cb) continue;
                    mBoard->build(x, y, 1, 1, cid, pid, mEditorMode, [&]() {
                        const auto t = e::make_shared<eGodMonumentTile>(
                                           *mBoard, mViewedCityId);
                        t->setMonument(s.get());
                        s->addTile(t.get());
                        return t;
                    });
                }
            }
            if(b) {
                mBoard->built(mViewedCityId, eBuildingType::godMonument, id);
                const bool ss = mBoard->supportsBuilding(mViewedCityId, mode);
                if(!ss) mGm->clearMode();
            }
'@

$newBuild = @'
            // tminX/tminY are the actual top-left corner of the complete
            // 4x4 monument footprint. Validate the same area we will build.
            const bool cb = mBoard->canBuild(
                                tminX, tminY, 4, 4,
                                mEditorMode, cid, pid);
            if(!cb) return true;

            const auto am = eBuildingMode::aphroditeMonument;
            const int id = static_cast<int>(mode) -
                           static_cast<int>(am);
            const auto gt = static_cast<eGodType>(id);
            const auto s = e::make_shared<eGodMonument>(
                               gt, eGodQuestId::godQuest1,
                               *mBoard, mViewedCityId);

            const bool b = mBoard->build(
                               tminX + 1, tminY + 2, 2, 2,
                               cid, pid, mEditorMode, [&]() {
                return s;
            });

            // Never create surround tiles unless the central monument was
            // successfully registered on the board. Otherwise they retain
            // a raw pointer to a temporary eGodMonument and later crash.
            if(!b) return true;

            for(int x = tminX; x < tmaxX; x++) {
                for(int y = tminY; y < tmaxY; y++) {
                    const bool tileBuildable =
                        mBoard->canBuild(x, y, 1, 1,
                                         mEditorMode, cid, pid);
                    if(!tileBuildable) continue;

                    mBoard->build(
                        x, y, 1, 1, cid, pid, mEditorMode, [&]() {
                            const auto t =
                                e::make_shared<eGodMonumentTile>(
                                    *mBoard, mViewedCityId);
                            t->setMonument(s.get());
                            s->addTile(t.get());
                            return t;
                        });
                }
            }

            mBoard->built(mViewedCityId,
                          eBuildingType::godMonument, id);
            const bool ss = mBoard->supportsBuilding(
                                mViewedCityId, mode);
            if(!ss) mGm->clearMode();
'@

Replace-Exact -Path $BuildFile -Old $oldBuild -New $newBuild -Label "safe god-monument 4x4 construction"

Write-Host ""
Write-Host "Poseidon/god monument runtime fixes applied." -ForegroundColor Cyan
Write-Host "Backups:"
Write-Host "  $Aesthetics.poseidon-backup"
Write-Host "  $BuildFile.poseidon-backup"
Write-Host ""
Write-Host "Next: cmake --build .\build --config Release --parallel"

