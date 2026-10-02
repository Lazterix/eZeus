$ErrorActionPreference = 'Stop'

function Assert-True {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) { throw $Message }
}

function New-Fixture {
    param(
        [string]$RepositoryRoot,
        [string]$OriginalRoot,
        [string]$AssetsRoot,
        [string]$SourceScript
    )

    foreach ($path in @(
        (Join-Path $RepositoryRoot 'scripts'),
        (Join-Path $RepositoryRoot 'build/Release'),
        (Join-Path $RepositoryRoot 'Adventures'),
        (Join-Path $RepositoryRoot 'assets/textures/buildings/laurel-garden'),
        (Join-Path $RepositoryRoot 'assets/textures/buildings/salt-works'),
        (Join-Path $RepositoryRoot 'fonts'),
        (Join-Path $RepositoryRoot 'sanctuaries'),
        (Join-Path $RepositoryRoot 'text'),
        (Join-Path $OriginalRoot 'DATA'),
        (Join-Path $OriginalRoot 'Audio'),
        (Join-Path $OriginalRoot 'Model'),
        $AssetsRoot
    )) {
        New-Item -ItemType Directory -Force -Path $path | Out-Null
    }

    Copy-Item -LiteralPath $SourceScript -Destination (Join-Path $RepositoryRoot 'scripts/dev-runtime.ps1')
    Set-Content -LiteralPath (Join-Path $RepositoryRoot 'build/Release/eZeus.exe') -Value 'exe'
    Set-Content -LiteralPath (Join-Path $RepositoryRoot 'Adventures/adventure.epak') -Value 'adventure'
    Set-Content -LiteralPath (Join-Path $RepositoryRoot 'assets/textures/buildings/laurel-garden/laurel-garden.png') -Value 'owned texture'
    Set-Content -LiteralPath (Join-Path $RepositoryRoot 'assets/textures/buildings/salt-works/salt-works.png') -Value 'salt works texture'
    Set-Content -LiteralPath (Join-Path $RepositoryRoot 'fonts/Zeus.ttf') -Value 'font'
    Set-Content -LiteralPath (Join-Path $RepositoryRoot 'sanctuaries/zeus.txt') -Value 'sanctuary'
    Set-Content -LiteralPath (Join-Path $RepositoryRoot 'text/language.txt') -Value 'language'
    Set-Content -LiteralPath (Join-Path $AssetsRoot 'interface.e') -Value 'interface'
    Set-Content -LiteralPath (Join-Path $AssetsRoot 'i30.e') -Value 'textures'
    Set-Content -LiteralPath (Join-Path $AssetsRoot 'Zeus_Text.xml') -Value '<text />'
    Set-Content -LiteralPath (Join-Path $AssetsRoot 'Zeus_MM.xml') -Value '<text />'
}

function Assert-StagedRuntime {
    param([string]$RuntimeRoot, [string]$ExpectedZeusPath)

    Assert-True (Test-Path -LiteralPath (Join-Path $RuntimeRoot 'Bin/eZeus.exe') -PathType Leaf) 'Executable was not staged under eZeus/Bin.'
    Assert-True (Test-Path -LiteralPath (Join-Path $RuntimeRoot 'interface.e') -PathType Leaf) 'interface.e was not staged.'
    Assert-True (Test-Path -LiteralPath (Join-Path $RuntimeRoot 'i30.e') -PathType Leaf) 'Available texture pack was not staged.'
    $stagedTexture = Join-Path $RuntimeRoot 'Textures/buildings/laurel-garden/laurel-garden.png'
    Assert-True (Test-Path -LiteralPath $stagedTexture -PathType Leaf) 'Repository-owned Laurel Garden texture was not staged.'
    Assert-True ((Get-Content -Raw -LiteralPath $stagedTexture) -eq "owned texture`r`n") 'Staged Laurel Garden texture did not match the repository-owned source.'
    $stagedSaltWorksTexture = Join-Path $RuntimeRoot 'Textures/buildings/salt-works/salt-works.png'
    Assert-True (Test-Path -LiteralPath $stagedSaltWorksTexture -PathType Leaf) 'Repository-owned Salt Works texture was not staged.'
    Assert-True (Test-Path -LiteralPath (Join-Path $RuntimeRoot 'Fonts/Zeus.ttf') -PathType Leaf) 'Repository font was not staged with runtime casing.'
    Assert-True (Test-Path -LiteralPath (Join-Path $RuntimeRoot 'Sanctuaries/zeus.txt') -PathType Leaf) 'Repository sanctuary data was not staged with runtime casing.'
    Assert-True (-not (Test-Path -LiteralPath (Join-Path $RuntimeRoot 'DATA'))) 'Original DATA directory was copied into the runtime.'
    $actualPath = Get-Content -Raw -LiteralPath (Join-Path $RuntimeRoot 'zeus_path.txt')
    Assert-True ($actualPath -eq $ExpectedZeusPath) 'zeus_path.txt does not contain the expected path relative to eZeus/Bin.'
}

$sourceScript = Join-Path $PSScriptRoot 'dev-runtime.ps1'
$systemTemp = Join-Path ([System.IO.Path]::GetTempPath()) ("ezeus-runtime-test-" + [guid]::NewGuid())
$workspaceTemp = Join-Path (Split-Path -Parent $PSScriptRoot) (".stage-root-test-" + [guid]::NewGuid())
$oldOriginal = $env:EZEUS_ORIGINAL_GAME_DIR
$oldAssets = $env:EZEUS_RUNTIME_ASSETS_DIR
$oldStageRoot = $env:EZEUS_STAGE_ROOT

try {
    $env:EZEUS_ORIGINAL_GAME_DIR = Join-Path $systemTemp 'wrong-original'
    $env:EZEUS_RUNTIME_ASSETS_DIR = Join-Path $systemTemp 'wrong-assets'
    $env:EZEUS_STAGE_ROOT = $null

    $sameDriveRepo = Join-Path $systemTemp 'same-drive-repo'
    $sameDriveOriginal = Join-Path $systemTemp 'Original Game'
    $sameDriveAssets = Join-Path $systemTemp 'Runtime Assets'
    New-Fixture $sameDriveRepo $sameDriveOriginal $sameDriveAssets $sourceScript

    & (Join-Path $sameDriveRepo 'scripts/dev-runtime.ps1') -Prepare -OriginalGameDir $sameDriveOriginal -RuntimeAssetsDir $sameDriveAssets
    $sameDriveRuntime = Join-Path $sameDriveRepo '.dev-runtime/eZeus'
    Assert-StagedRuntime $sameDriveRuntime '..\..\..\..\Original Game\'

    Set-Content -LiteralPath (Join-Path $sameDriveRuntime 'i15.e') -Value 'stale textures'
    & (Join-Path $sameDriveRepo 'scripts/dev-runtime.ps1') -Prepare -OriginalGameDir $sameDriveOriginal -RuntimeAssetsDir $sameDriveAssets
    Assert-True (-not (Test-Path -LiteralPath (Join-Path $sameDriveRuntime 'i15.e'))) 'An obsolete texture pack remained after preparing again.'

    $crossDriveRepo = Join-Path $workspaceTemp 'cross-drive-repo'
    $crossDriveOriginal = Join-Path $systemTemp 'Cross Drive Original'
    $crossDriveAssets = Join-Path $systemTemp 'Cross Drive Assets'
    New-Fixture $crossDriveRepo $crossDriveOriginal $crossDriveAssets $sourceScript

    $env:EZEUS_STAGE_ROOT = $null
    & (Join-Path $crossDriveRepo 'scripts/dev-runtime.ps1') -Prepare -OriginalGameDir $crossDriveOriginal -RuntimeAssetsDir $crossDriveAssets
    $automaticRuntime = Join-Path $crossDriveOriginal '.ezeus-dev-runtime/eZeus'
    Assert-StagedRuntime $automaticRuntime '..\..\..\'

    $invalidStageRoot = Join-Path $crossDriveRepo 'invalid-stage'
    $environmentStageRoot = Join-Path $crossDriveOriginal 'environment-stage'
    $env:EZEUS_STAGE_ROOT = $environmentStageRoot
    & (Join-Path $crossDriveRepo 'scripts/dev-runtime.ps1') -Prepare -OriginalGameDir $crossDriveOriginal -RuntimeAssetsDir $crossDriveAssets
    Assert-StagedRuntime (Join-Path $environmentStageRoot 'eZeus') '..\..\..\'

    $rejected = $false
    try {
        & (Join-Path $crossDriveRepo 'scripts/dev-runtime.ps1') -Prepare -OriginalGameDir $crossDriveOriginal -RuntimeAssetsDir $crossDriveAssets -StageRoot $invalidStageRoot
    }
    catch {
        $rejected = $_.Exception.Message -like '*same drive*'
    }
    Assert-True $rejected 'An explicit StageRoot on another drive was not rejected.'
    Assert-True (-not (Test-Path -LiteralPath $invalidStageRoot)) 'Invalid StageRoot was written before rejection.'

    Write-Host 'dev-runtime fixture tests passed.'
}
finally {
    $env:EZEUS_ORIGINAL_GAME_DIR = $oldOriginal
    $env:EZEUS_RUNTIME_ASSETS_DIR = $oldAssets
    $env:EZEUS_STAGE_ROOT = $oldStageRoot
    foreach ($path in @($systemTemp, $workspaceTemp)) {
        if (Test-Path -LiteralPath $path) {
            Remove-Item -LiteralPath $path -Recurse -Force
        }
    }
}
