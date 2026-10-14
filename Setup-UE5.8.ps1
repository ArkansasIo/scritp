[CmdletBinding()]
param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [switch]$ConfigureFirewall,
    [switch]$SkipEnvironment
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
    $ueRoot = [Environment]::ExpandEnvironmentVariables($EngineRoot)
    $ueEngine = Join-Path $ueRoot "Engine"
    $ueEditor = Join-Path $ueEngine "Binaries\Win64\UnrealEditor.exe"

    Write-Status "=== Unreal Engine 5.8 Setup Check ===" Cyan
    Write-Status "Engine root: $ueRoot" Gray

    if (-not (Test-Path -LiteralPath $ueEditor -PathType Leaf)) {
        throw "UnrealEditor.exe was not found at '$ueEditor'. Install UE 5.8 or pass -EngineRoot with the correct path."
    }

    $versionInfo = (Get-Item -LiteralPath $ueEditor).VersionInfo
    Write-Status "[OK] UnrealEditor.exe found" Green
    if ($versionInfo) {
        Write-Status "File version: $($versionInfo.FileVersion)" Green
        Write-Status "Product version: $($versionInfo.ProductVersion)" Green
    }

    $requiredDirs = @(
        (Join-Path $ueEngine "Binaries"),
        (Join-Path $ueEngine "Build"),
        (Join-Path $ueEngine "Config"),
        (Join-Path $ueEngine "Content"),
        (Join-Path $ueEngine "Plugins")
    )
    $missing = @($requiredDirs | Where-Object { -not (Test-Path -LiteralPath $_ -PathType Container) })
    if ($missing.Count -gt 0) {
        Write-Status "WARNING: The following expected directories were not found:" Yellow
        $missing | ForEach-Object { Write-Status "  $_" Yellow }
    } else {
        Write-Status "[OK] Expected engine directories found" Green
    }

    if (-not $SkipEnvironment) {
        $oldRoot = [Environment]::GetEnvironmentVariable("UE_ROOT", "User")
        if ($oldRoot -ne $ueRoot) {
            [Environment]::SetEnvironmentVariable("UE_ROOT", $ueRoot, "User")
            Write-Status "[OK] Set user UE_ROOT" Green
        } else {
            Write-Status "[OK] User UE_ROOT already matches" Green
        }

        $userPath = [Environment]::GetEnvironmentVariable("Path", "User")
        $pathEntries = @()
        if (-not [string]::IsNullOrWhiteSpace($userPath)) {
            $pathEntries = @($userPath -split ';' | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
        }
        $binaries = Join-Path $ueEngine "Binaries\Win64"
        $alreadyPresent = $pathEntries | Where-Object { $_.TrimEnd('\') -ieq $binaries.TrimEnd('\') }
        if (-not $alreadyPresent) {
            [Environment]::SetEnvironmentVariable("Path", (@($binaries) + $pathEntries | Select-Object -Unique) -join ';', "User")
            Write-Status "[OK] Added UE binaries to the user PATH" Green
        } else {
            Write-Status "[OK] UE binaries are already in the user PATH" Green
        }
    } else {
        Write-Status "Environment variable changes skipped" Yellow
    }

    if ($ConfigureFirewall) {
        $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
        $principal = [Security.Principal.WindowsPrincipal]::new($identity)
        if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
            throw "Firewall configuration requires an elevated PowerShell session. Re-run as Administrator or omit -ConfigureFirewall."
        }

        Write-Status "Firewall configuration is a deliberate opt-in." Yellow
        Write-Status "No broad inbound rules are created by this script." Yellow
        Write-Status "Enable only ports required by the plugins in use and scope each rule to trusted remote addresses." Yellow
        Write-Status "Configure rules for your deployment explicitly after reviewing docs/SETUP_TROUBLESHOOTING.md." Yellow
    } else {
        Write-Status "Firewall changes skipped (safe default)." Green
    }

    Write-Status ""
    Write-Status "Setup checks completed." Cyan
    exit 0
}
catch {
    Write-Status "ERROR: $($_.Exception.Message)" Red
    exit 1
}
