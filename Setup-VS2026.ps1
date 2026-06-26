param(
    [switch]$Quiet,
    [switch]$SkipInstall
)

$ErrorActionPreference = "Continue"
$Host.UI.RawUI.WindowTitle = "VS 2026 Setup for UE 5.8"

$vsInstallPath = "C:\Program Files\Microsoft Visual Studio\18\Community"
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsSetup = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\setup.exe"
$workloads = @(
    "Microsoft.VisualStudio.Workload.NativeGame",
    "Microsoft.VisualStudio.Workload.NativeDesktop"
)
$components = @(
    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
    "Microsoft.VisualStudio.Component.VC.CMake.Project"
)

Write-Host "=== Visual Studio 2026 Setup for UE 5.8 ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "[1/5] Checking Visual Studio installation..." -ForegroundColor Yellow
if (-not (Test-Path $vswhere)) {
    Write-Host "ERROR: Visual Studio Installer not found" -ForegroundColor Red
    Write-Host "Download from: https://visualstudio.microsoft.com/" -ForegroundColor Red
    exit 1
}

$vsInfo = & $vswhere -products Microsoft.VisualStudio.Product.Community -format json 2>$null | ConvertFrom-Json
if (-not $vsInfo) {
    Write-Host "  Visual Studio Community not installed" -ForegroundColor Yellow
    $installed = $false
} else {
    Write-Host "  Found: $($vsInfo.displayName) v$($vsInfo.installationVersion)" -ForegroundColor Green
    Write-Host "  Path: $($vsInfo.installationPath)" -ForegroundColor Green
    $installed = $true
    $vsInstallPath = $vsInfo.installationPath
}

Write-Host "[2/5] Checking C++ toolchain..." -ForegroundColor Yellow
$msvcDir = "$vsInstallPath\VC\Tools\MSVC"
$hasMsvc = (Test-Path $msvcDir) -and ((Get-ChildItem $msvcDir -ErrorAction SilentlyContinue).Count -gt 0)

if ($hasMsvc) {
    $msvcVer = (Get-ChildItem $msvcDir -ErrorAction SilentlyContinue | Select-Object -First 1).Name
    Write-Host "  MSVC $msvcVer - OK" -ForegroundColor Green
} else {
    Write-Host "  MSVC toolchain not installed" -ForegroundColor Yellow
}

if (-not $SkipInstall) {
    Write-Host "[3/5] Checking required workloads..." -ForegroundColor Yellow
    $needsWorkloads = -not $hasMsvc

    if ($needsWorkloads) {
        Write-Host "  Missing C++/Game dev workloads - starting installation..." -ForegroundColor Yellow
        Write-Host "  Workloads: $($workloads -join ', ')" -ForegroundColor Gray

        $admin = [System.Security.Principal.WindowsIdentity]::GetCurrent().Groups -contains 'S-1-5-32-544'
        if (-not $admin) {
            Write-Host ""
            Write-Host "  NOTE: Installation requires Administrator privileges." -ForegroundColor Yellow
            Write-Host "  A UAC prompt will appear if you proceed." -ForegroundColor Yellow
            Write-Host ""
        }

        $addArgs = @()
        foreach ($wl in $workloads) { $addArgs += "--add"; $addArgs += $wl }
        foreach ($comp in $components) { $addArgs += "--add"; $addArgs += $comp }

        if ($Quiet) {
            $addArgs += "--quiet"
            $addArgs += "--norestart"
        } else {
            $addArgs += "--passive"
            $addArgs += "--norestart"
        }
        $addArgs += "--installPath"
        $addArgs += $vsInstallPath
        $addArgs += "--includeRecommended"

        Write-Host "  Running: setup.exe modify ..." -ForegroundColor Gray
        Write-Host "  (This will download several GB - may take a while)" -ForegroundColor Yellow
        Write-Host ""

        $proc = Start-Process -FilePath $vsSetup -ArgumentList $addArgs -Wait -PassThru -NoNewWindow:$false
        if ($proc.ExitCode -eq 0 -or $proc.ExitCode -eq 3010) {
            Write-Host "  Installation completed successfully" -ForegroundColor Green
            if ($proc.ExitCode -eq 3010) {
                Write-Host "  A system reboot is required" -ForegroundColor Yellow
            }
        } else {
            Write-Host "  Installation returned exit code $($proc.ExitCode)" -ForegroundColor Red
            Write-Host "  Try running manually:" -ForegroundColor Yellow
            $manualCmd = "& '" + $vsSetup + "' modify --installPath '" + $vsInstallPath + "'"
            foreach ($wl in $workloads) { $manualCmd += " --add $wl" }
            $manualCmd += " --includeRecommended --passive"
            Write-Host "  $manualCmd" -ForegroundColor Gray
        }
    } else {
        Write-Host "  All workloads already installed" -ForegroundColor Green
    }
} else {
    Write-Host "[3/5] Installation skipped (--SkipInstall)" -ForegroundColor Yellow
}

Write-Host "[4/5] Verifying installation..." -ForegroundColor Yellow
$msvcDir = "$vsInstallPath\VC\Tools\MSVC"
$hasMsvcAfter = (Test-Path $msvcDir) -and ((Get-ChildItem $msvcDir -ErrorAction SilentlyContinue).Count -gt 0)

if ($hasMsvcAfter) {
    $msvcVer = (Get-ChildItem $msvcDir -ErrorAction SilentlyContinue | Select-Object -First 1).Name
    Write-Host "  MSVC $msvcVer - OK" -ForegroundColor Green
} else {
    Write-Host "  MSVC toolchain still missing" -ForegroundColor Red
}

Write-Host "[5/5] Configuring environment..." -ForegroundColor Yellow
$msbuildPath = "$vsInstallPath\MSBuild\Current\Bin"
if (Test-Path $msbuildPath) {
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
    if ($currentPath -notlike "*$msbuildPath*") {
        [Environment]::SetEnvironmentVariable("Path", "$msbuildPath;$currentPath", "User")
        Write-Host "  Added MSBuild to PATH" -ForegroundColor Green
    } else {
        Write-Host "  MSBuild already in PATH" -ForegroundColor Green
    }
} else {
    Write-Host "  MSBuild not found at expected path" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== VS 2026 Setup Complete ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Summary:" -ForegroundColor White
Write-Host "  Engine: UE 5.8 at C:\Program Files\Epic Games\UE_5.8" -ForegroundColor Gray
Write-Host "  Editor: $vsInstallPath\Common7\IDE\devenv.exe" -ForegroundColor Gray

if ($hasMsvcAfter) {
    Write-Host ""
    Write-Host "You can now generate project files and build for UE 5.8." -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "NOTE: C++ development tools need to be installed first." -ForegroundColor Yellow
    Write-Host "Run this script again with admin privileges, or install manually:" -ForegroundColor Yellow
    Write-Host "  Visual Studio Installer > Modify > Workloads > Game development with C++" -ForegroundColor Gray
}
