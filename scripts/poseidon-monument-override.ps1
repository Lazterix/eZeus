param(
    [Parameter(Position=0)]
    [ValidateSet("Install","Enable","Disable","Remove","Status")]
    [string]$Action = "Status",

    [string]$GameDir = "D:\Steam\steamapps\common\Zeus + Poseidon\.ezeus-dev-runtime\eZeus"
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$SourceRoot = Join-Path $ProjectRoot "assets\overrides"
$TexturesRoot = Join-Path $GameDir "Textures"
$DisabledRoot = Join-Path $TexturesRoot "_disabled_overrides\poseidon-monument"
$LegacyRoot = Join-Path $TexturesRoot "_disabled_overrides\poseidon-wrong-target-cleanup"

$Files = @{
    "15" = @("poseidonStatue_0.png")
    "30" = @("poseidonStatue_0.png")
    "45" = @("poseidonStatue_0.png")
    "60" = @("poseidonStatue_0.png","poseidonStatue_1.png","poseidonStatue_2.png","poseidonStatue_3.png")
}

function EnsureDir([string]$p) {
    New-Item -ItemType Directory -Force -Path $p | Out-Null
}

function HashFile([string]$p) {
    if (!(Test-Path -LiteralPath $p)) { return $null }
    (Get-FileHash -Algorithm SHA256 -LiteralPath $p).Hash
}

function Remove-LegacyWrongTarget {
    foreach ($r in @("15","30","45","60")) {
        $wrong = Join-Path (Join-Path $TexturesRoot $r) "poseidonStatues2_0.png"
        if (Test-Path -LiteralPath $wrong) {
            $dstDir = Join-Path $LegacyRoot $r
            EnsureDir $dstDir
            Move-Item -LiteralPath $wrong -Destination (Join-Path $dstDir "poseidonStatues2_0.png") -Force
            Write-Host "Moved old WRONG-TARGET override out of runtime: $r/poseidonStatues2_0.png" -ForegroundColor Yellow
        }
    }
}

switch ($Action) {
    "Install" {
        Remove-LegacyWrongTarget

        foreach ($r in $Files.Keys) {
            $dstDir = Join-Path $TexturesRoot $r
            EnsureDir $dstDir
            foreach ($name in $Files[$r]) {
                $src = Join-Path (Join-Path $SourceRoot $r) $name
                if (!(Test-Path -LiteralPath $src)) { throw "Missing source override: $src" }
                $dst = Join-Path $dstDir $name
                Copy-Item -LiteralPath $src -Destination $dst -Force
                Write-Host "Installed $r/$name" -ForegroundColor Green
            }
        }
        Write-Host "`nCorrect Poseidon GOD MONUMENT override enabled." -ForegroundColor Cyan
        Write-Host "Packed originals remain untouched." -ForegroundColor Cyan
    }

    "Disable" {
        foreach ($r in $Files.Keys) {
            foreach ($name in $Files[$r]) {
                $src = Join-Path (Join-Path $TexturesRoot $r) $name
                if (Test-Path -LiteralPath $src) {
                    $dstDir = Join-Path $DisabledRoot $r
                    EnsureDir $dstDir
                    Move-Item -LiteralPath $src -Destination (Join-Path $dstDir $name) -Force
                    Write-Host "Disabled $r/$name" -ForegroundColor Yellow
                }
            }
        }
        Write-Host "`nOverride disabled; packed Poseidon monument is active." -ForegroundColor Cyan
    }

    "Enable" {
        Remove-LegacyWrongTarget
        foreach ($r in $Files.Keys) {
            $runtimeDir = Join-Path $TexturesRoot $r
            EnsureDir $runtimeDir
            foreach ($name in $Files[$r]) {
                $dst = Join-Path $runtimeDir $name
                $disabled = Join-Path (Join-Path $DisabledRoot $r) $name
                $project = Join-Path (Join-Path $SourceRoot $r) $name

                if (Test-Path -LiteralPath $disabled) {
                    Move-Item -LiteralPath $disabled -Destination $dst -Force
                } elseif (!(Test-Path -LiteralPath $dst)) {
                    Copy-Item -LiteralPath $project -Destination $dst -Force
                }
                Write-Host "Enabled $r/$name" -ForegroundColor Green
            }
        }
    }

    "Remove" {
        foreach ($r in $Files.Keys) {
            foreach ($name in $Files[$r]) {
                $runtime = Join-Path (Join-Path $TexturesRoot $r) $name
                $disabled = Join-Path (Join-Path $DisabledRoot $r) $name
                if (Test-Path -LiteralPath $runtime) { Remove-Item -LiteralPath $runtime -Force }
                if (Test-Path -LiteralPath $disabled) { Remove-Item -LiteralPath $disabled -Force }
            }
        }
        Remove-LegacyWrongTarget
        Write-Host "`nLoose Poseidon monument override removed. Packed originals remain untouched." -ForegroundColor Cyan
    }

    "Status" {
        Write-Host "Poseidon GOD MONUMENT optional override (poseidonStatue)" -ForegroundColor Cyan
        Write-Host "GameDir: $GameDir"
        foreach ($r in @("15","30","45","60")) {
            foreach ($name in $Files[$r]) {
                $src = Join-Path (Join-Path $SourceRoot $r) $name
                $dst = Join-Path (Join-Path $TexturesRoot $r) $name
                $disabled = Join-Path (Join-Path $DisabledRoot $r) $name

                $state =
                    if (Test-Path -LiteralPath $dst) { "ENABLED" }
                    elseif (Test-Path -LiteralPath $disabled) { "DISABLED" }
                    else { "NOT INSTALLED" }

                $match = ""
                if ((Test-Path -LiteralPath $src) -and (Test-Path -LiteralPath $dst)) {
                    $match = if ((HashFile $src) -eq (HashFile $dst)) { " source=OK" } else { " source=DIFF" }
                }
                Write-Host ("{0}/{1}: {2}{3}" -f $r,$name,$state,$match)
            }
        }

        $legacyFound = $false
        foreach ($r in @("15","30","45","60")) {
            $legacy = Join-Path (Join-Path $TexturesRoot $r) "poseidonStatues2_0.png"
            if (Test-Path -LiteralPath $legacy) {
                $legacyFound = $true
                Write-Host "WARNING: wrong-target legacy override still present: $legacy" -ForegroundColor Red
            }
        }
        if (!$legacyFound) {
            Write-Host "Wrong-target poseidonStatues2 runtime files: none" -ForegroundColor DarkGray
        }

        Write-Host "`nEZEUS_DISABLE_TEXTURE_OVERRIDES=1 still bypasses all loose overrides."
    }
}
