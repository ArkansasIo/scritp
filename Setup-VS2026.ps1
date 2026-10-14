[CmdletBinding()]
param(
    [string]$InstallPath,
    [switch]$Quiet,
    [switch]$SkipInstall
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Status {
    param(
        [Parameter(Mandatory)][string]$Message,
        [ConsoleColor]$Color = [ConsoleColor]::Gray
    )
    Write-Host $Message -ForegroundColor $Color
}

try {
    $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
    $vswhere = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer\vswhere.exe"
    $vsSetup = Join-Path $programFilesX86 "Microsoft Visual Studio\Installer\setup.exe"

    Write-Status "=== Visual Studio C++ Toolchain Check ===" Cyan

    if (-not (Test-Path -LiteralPath $vswhere -PathType Leaf)) {
        throw "vswhere.exe was not found at '$vswhere'. Install Visual Studio Installer or repair the Visual Studio installation."
    }

    $instanceJson = & $vswhere -all -products '*' -format json
    $instances = @($instanceJson | ConvertFrom-Json)
    if ($InstallPath) {
        $selected = $instances | Where-Object { $_.installationPath -ieq $InstallPath } | Select-Object -First 1
        if (-not $selected) {
            throw "No Visual Studio instance found at '$InstallPath'."
        }
    } else {
        $selected = $instances |
            Where-Object { $_.isComplete -and $_.isLaunchable } |
            Sort-Object installationVersion -Descending |
            Select-Object -First 1
        if (-not $selected) {
            $selected = $instances | Where-Object { $_.isComplete } | Select-Object -First 1
        }
    }

    if (-not $selected) {
        throw "No Visual Studio instance was detected. Install Visual Studio and the C++ game development workload, then rerun this check."
    }

    $vsInstallPath = $selected.installationPath
    Write-Status "Detected: $($selected.displayName) $($selected.installationVersion)" Green
    Write-Status "Path: $vsInstallPath" Gray

    $msvcDir = Join-Path $vsInstallPath "VC\Tools\MSVC"
    $msvcVersions = @()
    if (Test-Path -LiteralPath $msvcDir -PathType Container) {
        $msvcVersions = @(Get-ChildItem -LiteralPath $msvcDir -Directory -ErrorAction SilentlyContinue)
    }
    if ($msvcVersions.Count -gt 0) {
        Write-Status "MSVC toolchain versions: $($msvcVersions.Name -join ', ')" Green
    } else {
        Write-Status "MSVC toolchain not detected." Yellow
    }

    $requiredPaths = @(
        (Join-Path $vsInstallPath "MSBuild\Current\Bin\MSBuild.exe"),
        (Join-Path $vsInstallPath "VC\Auxiliary\Build\vcvars64.bat")
    )
    $missingPaths = @($requiredPaths | Where-Object { -not (Test-Path -LiteralPath $_ -PathType Leaf) })
    if ($missingPaths.Count -gt 0) {
        Write-Status "Some expected C++ build files are missing:" Yellow
        $missingPaths | ForEach-Object { Write-Status "  $_" Yellow }
    } else {
        Write-Status "MSBuild and x64 compiler environment scripts found." Green
    }

    if (-not $SkipInstall -and ($msvcVersions.Count -eq 0 -or $missingPaths.Count -gt 0)) {
        if (-not (Test-Path -LiteralPath $vsSetup -PathType Leaf)) {
            throw "Visual Studio setup executable not found at '$vsSetup'. Use Visual Studio Installer manually."
        }

        $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
        $principal = [Security.Principal.WindowsPrincipal]::new($identity)
        if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
            throw "Installing Visual Studio workloads requires an elevated PowerShell session. Re-run as Administrator or install the workload manually."
        }

        $addArgs = @(
            "modify",
            "--installPath", $vsInstallPath,
            "--add", "Microsoft.VisualStudio.Workload.NativeGame",
            "--add", "Microsoft.VisualStudio.Workload.NativeDesktop",
            "--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "--add", "Microsoft.VisualStudio.Component.VC.CMake.Project",
            "--includeRecommended",
            "--norestart"
        )
        if ($Quiet) {
            $addArgs += "--quiet"
        } else {
            $addArgs += "--passive"
        }

        Write-Status "Launching Visual Studio Installer to add C++ workloads..." Yellow
        $proc = Start-Process -FilePath $vsSetup -ArgumentList $addArgs -Wait -PassThru
        if ($proc.ExitCode -notin @(0, 3010)) {
            throw "Visual Studio Installer exited with code $($proc.ExitCode). Review installer logs and repair the workload installation."
        }
        if ($proc.ExitCode -eq 3010) {
            Write-Status "Installer completed; a reboot is required." Yellow
        }

        $msvcVersions = @()
        if (Test-Path -LiteralPath $msvcDir -PathType Container) {
            $msvcVersions = @(Get-ChildItem -LiteralPath $msvcDir -Directory -ErrorAction SilentlyContinue)
        }
        if ($msvcVersions.Count -eq 0) {
            throw "Installation returned successfully but MSVC could not be verified. Check workloads in Visual Studio Installer."
        }
    } elseif ($SkipInstall -and ($msvcVersions.Count -eq 0 -or $missingPaths.Count -gt 0)) {
        throw "Required C++ build tools appear to be missing and installation was skipped."
    }

    Write-Status ""
    Write-Status "=== Visual Studio toolchain check completed ===" Cyan
    Write-Status "Verify compatibility against the exact Unreal Engine build requirements." Gray
    exit 0
}
catch {
    Write-Status "ERROR: $($_.Exception.Message)" Red
    exit 1
}
