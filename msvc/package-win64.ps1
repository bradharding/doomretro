param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectDir,

    [Parameter(Mandatory = $true)]
    [string]$OutDir,

    [Parameter(Mandatory = $true)]
    [string]$ProjectName
)

$ErrorActionPreference = 'Stop'

$projectRoot = (Resolve-Path (Join-Path $ProjectDir '..')).Path
$outputDir = (Resolve-Path $OutDir).Path
$versionHeader = Join-Path $projectRoot 'src\version.h'
$versionMatch = Select-String -Path $versionHeader -Pattern '#define DOOMRETRO_VERSIONSTRING\s+"([^"]+)"'

if (!$versionMatch)
{
    throw "Unable to determine the DOOM Retro version from '$versionHeader'."
}

$version = $versionMatch.Matches[0].Groups[1].Value
$wadSource = Join-Path $projectRoot "res\$ProjectName.wad"
$wadOutput = Join-Path $outputDir "$ProjectName.wad"
$exeSource = Join-Path $outputDir "$ProjectName.exe"
$packageDir = Join-Path $outputDir "$ProjectName-$version-win64"
$zipPath = Join-Path $outputDir "$ProjectName-$version-win64.zip"
$dllPatterns = @(
    (Join-Path $projectRoot 'SDL2-2.32.10\lib\x64\SDL2.dll'),
    (Join-Path $projectRoot 'SDL2_mixer-2.8.2\lib\x64\SDL2_mixer.dll'),
    (Join-Path $projectRoot 'SDL2_mixer-2.8.2\lib\x64\optional\libogg-0.dll'),
    (Join-Path $projectRoot 'SDL2_mixer-2.8.2\lib\x64\optional\libopus-0.dll'),
    (Join-Path $projectRoot 'SDL2_mixer-2.8.2\lib\x64\optional\libopusfile-0.dll'),
    (Join-Path $projectRoot 'SDL2_mixer-2.8.2\lib\x64\optional\libxmp-0.dll')
)

if (!(Test-Path $exeSource))
{
    throw "Expected executable '$exeSource' was not found."
}

if (!(Test-Path $wadSource))
{
    throw "Expected WAD '$wadSource' was not found."
}

Copy-Item $wadSource -Destination $wadOutput -Force

Remove-Item $packageDir -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $packageDir | Out-Null

Copy-Item $exeSource -Destination $packageDir -Force
Copy-Item $wadOutput -Destination $packageDir -Force

$packagedDlls = @()

foreach ($dllPattern in $dllPatterns)
{
    $dllFiles = Get-ChildItem -Path $dllPattern -File -ErrorAction SilentlyContinue

    foreach ($dllFile in $dllFiles)
    {
        Copy-Item $dllFile.FullName -Destination $outputDir -Force
        Copy-Item $dllFile.FullName -Destination $packageDir -Force
        $packagedDlls += $dllFile.FullName
    }
}

if (!$packagedDlls.Count)
{
    throw 'No DLL files were found to package.'
}

Remove-Item $zipPath -Force -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $packageDir '*') -DestinationPath $zipPath -CompressionLevel Optimal
Remove-Item $packageDir -Recurse -Force
