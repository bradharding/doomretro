#
#============================================================================
#
#                                 DOOM Retro
#           The classic, refined DOOM source port. For Windows PC.
#
#============================================================================
#
#    Copyright © 1993-2026 by id Software LLC, a ZeniMax Media company.
#    Copyright © 2013-2026 by Brad Harding <mailto:brad@doomretro.com>.
#
#    This file is a part of DOOM Retro.
#
#    DOOM Retro is free software: you can redistribute it and/or modify it
#    under the terms of the GNU General Public License as published by the
#    Free Software Foundation, either version 3 of the license, or (at your
#    option) any later version.
#
#    DOOM Retro is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#    General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.
#
#    DOOM is a registered trademark of id Software LLC, a ZeniMax Media
#    company, in the US and/or other countries, and is used without
#    permission. All other trademarks are the property of their respective
#    holders. DOOM Retro is in no way affiliated with nor endorsed by
#    id Software.
#
#============================================================================
#

param(
    [string]$ProjectDir = $PSScriptRoot,

    [string]$OutDir = (Join-Path $PSScriptRoot 'Release'),

    [string]$ProjectName = 'doomretro',

    [string]$MsysRoot = $(if ($env:MSYS2_ROOT) { $env:MSYS2_ROOT } else { 'C:\msys64' }),

    [string]$MsysEnv = 'ucrt64',

    [string]$March = 'x86-64-v2'
)

$ErrorActionPreference = 'Stop'

$projectRoot = (Resolve-Path (Join-Path $ProjectDir '..')).Path

New-Item -ItemType Directory -Path $OutDir -Force | Out-Null

$outputDir = (Resolve-Path $OutDir).Path
$versionHeader = Join-Path $projectRoot 'src\version.h'
$versionMatch = Select-String -Path $versionHeader -Pattern '#define DOOMRETRO_VERSIONSTRING\s+"([^"]+)"'

if (!$versionMatch)
{
    throw "Unable to determine the DOOM Retro version from '$versionHeader'."
}

$version = $versionMatch.Matches[0].Groups[1].Value
$wadSource = Join-Path $projectRoot "res\$ProjectName.wad"
$msysBin = Join-Path $MsysRoot "$MsysEnv\bin"
$msysUsrBin = Join-Path $MsysRoot 'usr\bin'
$buildDir = Join-Path $outputDir "$ProjectName-mingw-build"
$packageDir = Join-Path $outputDir "$ProjectName-$version-win64-static"
$zipPath = Join-Path $outputDir "$ProjectName-$version-win64-static.zip"

$requirements = @(
    (Join-Path $msysBin 'gcc.exe'),
    (Join-Path $msysBin 'cmake.exe'),
    (Join-Path $msysBin 'ninja.exe'),
    (Join-Path $msysBin 'pkg-config.exe'),
    (Join-Path $MsysRoot "$MsysEnv\lib\libSDL2_mixer.a")
)
$missing = $requirements | Where-Object { !(Test-Path $_) }

if ($missing)
{
    throw "MSYS2 '$MsysEnv' toolchain not found in '$MsysRoot' (missing: $($missing -join ', '))."
}

if (!(Test-Path $wadSource))
{
    throw "Expected WAD '$wadSource' was not found."
}

$originalPath = $env:PATH
$originalMsystem = $env:MSYSTEM

try
{
    $env:MSYSTEM = $MsysEnv.ToUpper()
    $env:PATH = "$msysBin;$msysUsrBin;$originalPath"

    & cmake -G Ninja -S $projectRoot -B $buildDir -DCMAKE_BUILD_TYPE=Release -DDOOMRETRO_STATIC=ON -DDOOMRETRO_OPTIMIZE=ON "-DDOOMRETRO_MARCH=$March"

    if ($LASTEXITCODE -ne 0)
    {
        throw "CMake configure failed with exit code $LASTEXITCODE."
    }

    & cmake --build $buildDir

    if ($LASTEXITCODE -ne 0)
    {
        throw "CMake build failed with exit code $LASTEXITCODE."
    }

    $exeSource = Join-Path $buildDir "$ProjectName.exe"

    if (!(Test-Path $exeSource))
    {
        throw "Expected executable '$exeSource' was not found."
    }

    $badDlls = & objdump -p $exeSource | Select-String 'DLL Name' | Where-Object { $_ -match 'SDL2|libgcc|libstdc|libwinpthread|mingw|libogg|libopus|libxmp|libvorbis|libFLAC|libmpg123' }

    if ($badDlls)
    {
        throw "Executable still depends on: $($badDlls -join ', ')"
    }
}
finally
{
    $env:PATH = $originalPath
    $env:MSYSTEM = $originalMsystem
}

Remove-Item $packageDir -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path $packageDir | Out-Null

Copy-Item $exeSource -Destination $packageDir -Force
Copy-Item $wadSource -Destination $packageDir -Force

Remove-Item $zipPath -Force -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $packageDir '*') -DestinationPath $zipPath -CompressionLevel Optimal
Remove-Item $packageDir -Recurse -Force

Write-Host "Created $zipPath"
