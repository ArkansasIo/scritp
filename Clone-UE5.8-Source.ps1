#requires -Version 5.1
[CmdletBinding()]
param([string]$Destination=(Join-Path $PSScriptRoot "LocalEngine/UnrealEngine-5.8"),[string]$Branch="5.8",[switch]$SkipBranchCheck)
$ErrorActionPreference="Stop"
$RepositoryUrl="https://github.com/EpicGames/UnrealEngine.git"
Write-Host "Epic-licensed engine source is cloned locally, not copied into this repository." -ForegroundColor Cyan
if(-not(Get-Command git -ErrorAction SilentlyContinue)){throw "Install Git for Windows first."}
if(Test-Path $Destination){if((Get-ChildItem -Force -LiteralPath $Destination|Measure-Object).Count -gt 0){throw "Destination exists and is not empty: $Destination"}}
if(-not $SkipBranchCheck){
  Write-Host "Checking remote branch '$Branch'; this requires Epic-approved GitHub access..."
  $refs=git ls-remote --heads $RepositoryUrl $Branch 2>&1
  if($LASTEXITCODE -ne 0 -or -not $refs){throw "Branch '$Branch' could not be verified. Link Epic and GitHub, accept the EpicGames organization invitation, verify license access and branch names, then rerun with -Branch <branch>. No source was downloaded."}
}
$parent=Split-Path -Parent $Destination
New-Item -ItemType Directory -Force -Path $parent|Out-Null
git clone --branch $Branch --single-branch $RepositoryUrl $Destination
if($LASTEXITCODE -ne 0){throw "Clone failed. Check GitHub authentication, access and branch name."}
Write-Host "Clone complete. Follow ENGINE_SOURCE_ACCESS.md for Setup.bat and project-file generation." -ForegroundColor Green
