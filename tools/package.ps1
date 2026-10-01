<#
.SYNOPSIS
    Packages the built wxl-modern-water extension into the release zip.

.DESCRIPTION
    Builds wxl-modern-water.zip at the repo root from the current wxl-modern-water.dll,
    wxl-modern-water.ini and data\waterdata.bin. The three files sit at the zip root because the
    WarcraftXL Hub installer expects <id>.dll at the root and the core reads the ini and water data
    from the extension's own folder. Sources, shaders, wxl.json, store/ and *.bak files are never
    packaged.

    Build the DLL into the repo folder first (AGENTS.md, "Building this module"); this script only
    packages what is already there, so a missing DLL or data file is a hard error.

.EXAMPLE
    .\tools\package.ps1
    Packages wxl-modern-water.zip next to this repository's root. An existing zip is backed up to
    wxl-modern-water.zip.bak first.
#>
[CmdletBinding()]
param(
    [string]$RepoRoot,
    [string]$DllPath,
    [string]$IniPath,
    [string]$DataPath,
    [string]$OutputPath
)

$ErrorActionPreference = 'Stop'

if (-not $RepoRoot) { $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path }

$name = 'wxl-modern-water'
if (-not $DllPath)    { $DllPath    = Join-Path $RepoRoot "$name.dll" }
if (-not $IniPath)    { $IniPath    = Join-Path $RepoRoot "$name.ini" }
if (-not $DataPath)   { $DataPath   = Join-Path $RepoRoot 'data\waterdata.bin' }
if (-not $OutputPath) { $OutputPath = Join-Path $RepoRoot "$name.zip" }

foreach ($file in @($DllPath, $IniPath, $DataPath)) {
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
        throw "Missing '$file'. Build the module first (see AGENTS.md, 'Building this module')."
    }
}

# Stage into a clean temp folder so the zip contains exactly the three shipped files, no matter what
# else lives in the repo root.
$staging = Join-Path ([System.IO.Path]::GetTempPath()) ("$name-package-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force -Path $staging | Out-Null
try {
    Copy-Item -LiteralPath $DllPath  -Destination (Join-Path $staging "$name.dll")
    Copy-Item -LiteralPath $IniPath  -Destination (Join-Path $staging "$name.ini")
    Copy-Item -LiteralPath $DataPath -Destination (Join-Path $staging 'waterdata.bin')

    if (Test-Path -LiteralPath $OutputPath) {
        Copy-Item -LiteralPath $OutputPath -Destination "$OutputPath.bak" -Force
    }
    Compress-Archive -Path (Join-Path $staging '*') -DestinationPath $OutputPath -Force
}
finally {
    Remove-Item -Recurse -Force -LiteralPath $staging -ErrorAction SilentlyContinue
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = Get-Item -LiteralPath $OutputPath
Write-Host ("Packaged {0} ({1:N0} bytes)" -f $zip.FullName, $zip.Length)
[System.IO.Compression.ZipFile]::OpenRead($OutputPath).Entries | ForEach-Object {
    Write-Host ("  {0}  ({1:N0} bytes)" -f $_.FullName, $_.Length)
}
