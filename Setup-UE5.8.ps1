param(
    [switch]$SkipFirewall
)

$ErrorActionPreference = "Continue"
$Host.UI.RawUI.WindowTitle = "UE 5.8 Setup"

$ueRoot = "C:\Program Files\Epic Games\UE_5.8"
$ueEngine = "$ueRoot\Engine"
$ueBinaries = "$ueEngine\Binaries\Win64"
$ueEditor = "$ueBinaries\UnrealEditor.exe"
$env:UE_ROOT = $ueRoot

Write-Host "=== UE 5.8 Setup Script ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "[1/6] Verifying UE 5.8 installation..." -ForegroundColor Yellow
if (-not (Test-Path $ueEditor)) {
    Write-Host "ERROR: UE 5.8 not found at $ueEditor" -ForegroundColor Red
    Write-Host "Install UE 5.8 via Epic Games Launcher first." -ForegroundColor Red
    exit 1
}
Write-Host "  UE 5.8 found at: $ueRoot" -ForegroundColor Green

Write-Host "[2/6] Checking engine integrity..." -ForegroundColor Yellow
$requiredDirs = @(
    "$ueEngine\Binaries",
    "$ueEngine\Build",
    "$ueEngine\Config",
    "$ueEngine\Content",
    "$ueEngine\Plugins",
    "$ueEngine\Source"
)
$missing = $requiredDirs | Where-Object { -not (Test-Path $_) }
if ($missing) {
    Write-Host "  WARNING: Missing directories:" -ForegroundColor Yellow
    $missing | ForEach-Object { Write-Host "    $_" -ForegroundColor Yellow }
} else {
    Write-Host "  Engine directories OK" -ForegroundColor Green
}

Write-Host "[3/6] Setting environment variables..." -ForegroundColor Yellow
$currentPath = [Environment]::GetEnvironmentVariable("UE_ROOT", "User")
if ($currentPath -ne $ueRoot) {
    [Environment]::SetEnvironmentVariable("UE_ROOT", $ueRoot, "User")
    Write-Host "  UE_ROOT set to $ueRoot" -ForegroundColor Green
} else {
    Write-Host "  UE_ROOT already configured" -ForegroundColor Green
}

$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($userPath -notlike "*$ueBinaries*") {
    $newPath = "$ueBinaries;$userPath"
    [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
    Write-Host "  Added UE binaries to user PATH" -ForegroundColor Green
} else {
    Write-Host "  UE binaries already in PATH" -ForegroundColor Green
}

Write-Host "[4/6] Checking engine association..." -ForegroundColor Yellow
$regPath = "HKCU:\Software\Epic Games\Unreal Engine\5.8"
if (-not (Test-Path $regPath)) {
    Write-Host "  Registry key not found (will be created on first editor launch)" -ForegroundColor Yellow
} else {
    $installedDir = (Get-ItemProperty -Path $regPath -Name "InstalledDirectory" -ErrorAction SilentlyContinue).InstalledDirectory
    if ($installedDir -eq $ueRoot) {
        Write-Host "  Engine association OK" -ForegroundColor Green
    } else {
        Write-Host "  WARNING: Registry points to $installedDir" -ForegroundColor Yellow
    }
}

if (-not $SkipFirewall) {
    Write-Host "[5/6] Configuring Windows Firewall..." -ForegroundColor Yellow
    $rules = @(
        @{ Name = "UE5.8 Remote Control HTTP"; Port = 30010; Proto = "TCP" }
        @{ Name = "UE5.8 Remote Control WS";   Port = 30020; Proto = "TCP" }
        @{ Name = "UE5.8 Python Remote Exec";  Port = 30030; Proto = "TCP" }
        @{ Name = "UE5.8 Multi-User UDP";      Port = "9030-9040"; Proto = "UDP" }
    )
    $admin = [System.Security.Principal.WindowsIdentity]::GetCurrent().Groups -contains 'S-1-5-32-544'
    foreach ($rule in $rules) {
        $existing = Get-NetFirewallRule -DisplayName $rule.Name -ErrorAction SilentlyContinue
        if (-not $existing) {
            if ($admin) {
                New-NetFirewallRule -DisplayName $rule.Name `
                    -Direction Inbound -Protocol $rule.Proto `
                    -LocalPort $rule.Port -Action Allow | Out-Null
                Write-Host "  Added: $($rule.Name)" -ForegroundColor Green
            } else {
                Write-Host "  SKIP ($($rule.Name)) - run as Admin to add firewall rules" -ForegroundColor Yellow
            }
        } else {
            Write-Host "  OK: $($rule.Name)" -ForegroundColor Green
        }
    }
} else {
    Write-Host "[5/6] Firewall config skipped" -ForegroundColor Yellow
}

Write-Host "[6/6] Quick validation..." -ForegroundColor Yellow
$versionInfo = (Get-Item $ueEditor -ErrorAction SilentlyContinue).VersionInfo
if ($versionInfo) {
    Write-Host "  UnrealEditor.exe: $($versionInfo.FileVersion)" -ForegroundColor Green
    Write-Host "  Product: $($versionInfo.ProductVersion)" -ForegroundColor Green
} else {
    Write-Host "  WARNING: Could not read editor version" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== UE 5.8 Setup Complete ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next steps:" -ForegroundColor White
Write-Host "  1. Launch:  $ueEditor" -ForegroundColor Gray
Write-Host "  2. Open your project" -ForegroundColor Gray
Write-Host "  3. Enable Remote Control plugin (Edit > Plugins)" -ForegroundColor Gray
Write-Host "  4. See UE5.8_Connection_Setup.md for remote connection details" -ForegroundColor Gray
