[CmdletBinding(DefaultParameterSetName = 'Prepare')]
param(
    [Parameter(Mandatory, ParameterSetName = 'Prepare')]
    [switch]$Prepare,

    [Parameter(Mandatory, ParameterSetName = 'Run')]
    [switch]$Run,

    [string]$OriginalGameDir,

    [string]$RuntimeAssetsDir,

    [string]$StageRoot
)

$ErrorActionPreference = 'Stop'

function Resolve-InputDirectory {
    param(
        [string]$ParameterValue,
        [string]$EnvironmentValue,
        [string]$Name
    )

    $value = if ([string]::IsNullOrWhiteSpace($ParameterValue)) {
        $EnvironmentValue
    } else {
        $ParameterValue
    }

    if ([string]::IsNullOrWhiteSpace($value)) {
        throw "Set $Name or pass the corresponding command-line parameter."
    }

    return (Resolve-Path -LiteralPath $value -ErrorAction Stop).Path
}

function Copy-Directory {
    param(
        [string]$Source,
        [string]$Destination
    )

    if (Test-Path -LiteralPath $Destination) {
        Remove-Item -LiteralPath $Destination -Recurse -Force
    }
    Copy-Item -LiteralPath $Source -Destination $Destination -Recurse -Force
}

function Get-RelativeDirectoryPath {
    param(
        [string]$From,
        [string]$To
    )

    $separator = [System.IO.Path]::DirectorySeparatorChar
    $fromUri = [uri]([System.IO.Path]::GetFullPath($From).TrimEnd('\', '/') + $separator)
    $toUri = [uri]([System.IO.Path]::GetFullPath($To).TrimEnd('\', '/') + $separator)
    $relativeUri = $fromUri.MakeRelativeUri($toUri)
    if ($relativeUri.IsAbsoluteUri) {
        throw 'The runtime and original game installation must be on the same drive because eZeus zeus_path.txt only supports a path relative to Bin.'
    }

    return [uri]::UnescapeDataString($relativeUri.ToString()).Replace('/', $separator)
}

function Test-SameVolume {
    param(
        [string]$FirstPath,
        [string]$SecondPath
    )

    $firstRoot = [System.IO.Path]::GetPathRoot([System.IO.Path]::GetFullPath($FirstPath))
    $secondRoot = [System.IO.Path]::GetPathRoot([System.IO.Path]::GetFullPath($SecondPath))
    return [string]::Equals($firstRoot, $secondRoot, [System.StringComparison]::OrdinalIgnoreCase)
}

$repositoryRoot = Split-Path -Parent $PSScriptRoot
$executableSource = Join-Path $repositoryRoot 'build/Release/eZeus.exe'

$originalRoot = Resolve-InputDirectory `
    -ParameterValue $OriginalGameDir `
    -EnvironmentValue $env:EZEUS_ORIGINAL_GAME_DIR `
    -Name 'EZEUS_ORIGINAL_GAME_DIR'
$assetsRoot = Resolve-InputDirectory `
    -ParameterValue $RuntimeAssetsDir `
    -EnvironmentValue $env:EZEUS_RUNTIME_ASSETS_DIR `
    -Name 'EZEUS_RUNTIME_ASSETS_DIR'

$stageRootValue = if (-not [string]::IsNullOrWhiteSpace($StageRoot)) {
    $StageRoot
} elseif (-not [string]::IsNullOrWhiteSpace($env:EZEUS_STAGE_ROOT)) {
    $env:EZEUS_STAGE_ROOT
} elseif (Test-SameVolume -FirstPath $repositoryRoot -SecondPath $originalRoot) {
    Join-Path $repositoryRoot '.dev-runtime'
} else {
    Join-Path $originalRoot '.ezeus-dev-runtime'
}
$resolvedStageRoot = [System.IO.Path]::GetFullPath($stageRootValue)

if (-not (Test-SameVolume -FirstPath $resolvedStageRoot -SecondPath $originalRoot)) {
    throw 'StageRoot must be on the same drive as OriginalGameDir because eZeus zeus_path.txt is relative to Bin.'
}

$normalizedStageRoot = $resolvedStageRoot.TrimEnd('\', '/')
$normalizedOriginalRoot = $originalRoot.TrimEnd('\', '/')
if ([string]::Equals($normalizedStageRoot, $normalizedOriginalRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw 'StageRoot must not be the original game directory itself.'
}

$runtimeRoot = Join-Path $resolvedStageRoot 'eZeus'
$runtimeBin = Join-Path $runtimeRoot 'Bin'

if (-not (Test-Path -LiteralPath $executableSource -PathType Leaf)) {
    throw "Release executable not found: $executableSource"
}

foreach ($directory in @('DATA', 'Audio', 'Model')) {
    $path = Join-Path $originalRoot $directory
    if (-not (Test-Path -LiteralPath $path -PathType Container)) {
        throw "Original game directory is missing required $directory directory: $path"
    }
}

foreach ($optionalPath in @('Adventures', 'zeus.ico')) {
    $path = Join-Path $originalRoot $optionalPath
    if (-not (Test-Path -LiteralPath $path)) {
        Write-Warning "Optional original-game resource not found: $path"
    }
}

$requiredAssets = @('interface.e', 'Zeus_Text.xml', 'Zeus_MM.xml')
foreach ($file in $requiredAssets) {
    $path = Join-Path $assetsRoot $file
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "Runtime asset directory is missing required file: $path"
    }
}

$texturePacks = @('i15.e', 'i30.e', 'i45.e', 'i60.e') | Where-Object {
    Test-Path -LiteralPath (Join-Path $assetsRoot $_) -PathType Leaf
}
if ($texturePacks.Count -eq 0) {
    throw "Runtime asset directory must contain at least one texture pack: i15.e, i30.e, i45.e, or i60.e."
}

$relativeOriginalPath = Get-RelativeDirectoryPath -From $runtimeBin -To $originalRoot

New-Item -ItemType Directory -Force -Path $runtimeBin | Out-Null
Copy-Item -LiteralPath $executableSource -Destination (Join-Path $runtimeBin 'eZeus.exe') -Force

foreach ($dllSource in @(
    (Join-Path $assetsRoot 'Bin'),
    (Split-Path -Parent $executableSource)
)) {
    if (Test-Path -LiteralPath $dllSource -PathType Container) {
        Get-ChildItem -LiteralPath $dllSource -Filter '*.dll' -File | ForEach-Object {
            Copy-Item -LiteralPath $_.FullName -Destination $runtimeBin -Force
        }
    }
}

foreach ($file in @('i15.e', 'i30.e', 'i45.e', 'i60.e')) {
    $stagedPath = Join-Path $runtimeRoot $file
    if (Test-Path -LiteralPath $stagedPath -PathType Leaf) {
        Remove-Item -LiteralPath $stagedPath -Force
    }
}
foreach ($file in $requiredAssets + $texturePacks) {
    Copy-Item -LiteralPath (Join-Path $assetsRoot $file) -Destination $runtimeRoot -Force
}

$repositoryResources = @(
    @{ Source = 'Adventures'; Destination = 'Adventures' },
    @{ Source = 'fonts'; Destination = 'Fonts' },
    @{ Source = 'sanctuaries'; Destination = 'Sanctuaries' },
    @{ Source = 'text'; Destination = 'Text' }
)
foreach ($resource in $repositoryResources) {
    $source = Join-Path $repositoryRoot $resource.Source
    if (-not (Test-Path -LiteralPath $source -PathType Container)) {
        throw "Repository runtime resource directory not found: $source"
    }
    Copy-Directory -Source $source -Destination (Join-Path $runtimeRoot $resource.Destination)
}

$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
[System.IO.File]::WriteAllText(
    (Join-Path $runtimeRoot 'zeus_path.txt'),
    $relativeOriginalPath,
    $utf8NoBom
)

Write-Host "Prepared eZeus development runtime: $runtimeRoot"

if ($Run) {
    & (Join-Path $runtimeBin 'eZeus.exe')
    exit $LASTEXITCODE
}
