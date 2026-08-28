param(
    [switch]$AllArchitectures,
    [uint64]$ReplayTicks = 10000
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$solutionPath = Join-Path $repositoryRoot 'Graphics.sln'
$vswherePath = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'

if (-not (Test-Path -LiteralPath $vswherePath)) {
    throw "Visual Studio Installer's vswhere.exe was not found."
}

$msbuildPath = & $vswherePath `
    -latest `
    -products '*' `
    -requires Microsoft.Component.MSBuild `
    -find 'MSBuild\**\Bin\MSBuild.exe' |
    Select-Object -First 1

if (-not $msbuildPath -or -not (Test-Path -LiteralPath $msbuildPath)) {
    throw 'MSBuild.exe was not found in the latest Visual Studio installation.'
}

$buildMatrix = @(
    @{ Configuration = 'Debug'; Platform = 'x64'; ArtifactPlatform = 'x64' },
    @{ Configuration = 'Release'; Platform = 'x64'; ArtifactPlatform = 'x64' }
)

if ($AllArchitectures) {
    $buildMatrix += @(
        @{ Configuration = 'Debug'; Platform = 'x86'; ArtifactPlatform = 'Win32' },
        @{ Configuration = 'Release'; Platform = 'x86'; ArtifactPlatform = 'Win32' }
    )
}

Push-Location $repositoryRoot
try {
    foreach ($entry in $buildMatrix) {
        $configuration = $entry.Configuration
        $platform = $entry.Platform
        $artifactPlatform = $entry.ArtifactPlatform
        Write-Host "Building $configuration|$platform..."

        & $msbuildPath $solutionPath `
            /t:Build `
            "/p:Configuration=$configuration" `
            "/p:Platform=$platform" `
            /m `
            /v:minimal

        if ($LASTEXITCODE -ne 0) {
            throw "Build failed for $configuration|$platform."
        }

        $testPath = Join-Path $repositoryRoot `
            "artifacts\$artifactPlatform\$configuration\PortfolioRuntimeTests.exe"
        if (-not (Test-Path -LiteralPath $testPath)) {
            throw "Runtime verification executable was not produced: $testPath"
        }

        Write-Host "Running deterministic replay verification ($ReplayTicks ticks)..."
        & $testPath --ticks $ReplayTicks
        if ($LASTEXITCODE -ne 0) {
            throw "Runtime verification failed for $configuration|$platform."
        }
    }
}
finally {
    Pop-Location
}

Write-Host 'All requested builds and runtime verification checks passed.'
