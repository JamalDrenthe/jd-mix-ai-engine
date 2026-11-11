<#
.SYNOPSIS
    JD Mix AI Engine — Automated Windows Update & Repair Utility
.DESCRIPTION
    Checks GitHub Releases for the latest version of JD Mix AI Engine,
    downloads the installer executable, and performs an in-place upgrade
    while preserving all custom presets and user settings.
#>

[CmdletBinding()]
param (
    [switch]$Force,
    [switch]$Silent
)

$RepoOwner = "JamalDrenthe"
$RepoName  = "jd-mix-ai-engine"
$ApiUrl    = "https://api.github.com/repos/$RepoOwner/$RepoName/releases/latest"

Write-Host "=========================================================" -ForegroundColor Cyan
Write-Host "   JD MIX AI ENGINE — CONTINUOUS UPDATE UTILITY          " -ForegroundColor Cyan
Write-Host "   FL Studio First | Automated Deployment & Maintenance " -ForegroundColor DarkCyan
Write-Host "=========================================================`n" -ForegroundColor Cyan

# 1. Check local installed version via Registry
$InstalledVersion = "0.0.0"
$RegKey = "HKLM:\Software\JD Audio Labs\JD Mix AI Engine"
if (Test-Path $RegKey) {
    $InstalledVersion = (Get-ItemProperty -Path $RegKey -Name "Version" -ErrorAction SilentlyContinue).Version
    Write-Host "-> Current installed version: v$InstalledVersion" -ForegroundColor Gray
} else {
    Write-Host "-> No existing installation found in Windows registry." -ForegroundColor Yellow
}

# 2. Fetch latest release from GitHub API
Write-Host "-> Checking GitHub Releases for latest version..." -ForegroundColor Gray
try {
    [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12 -bor [Net.SecurityProtocolType]::Tls13
    $Release = Invoke-RestMethod -Uri $ApiUrl -Headers @{ "User-Agent" = "JDMixUpdater" } -TimeoutSec 10
    $LatestTag = $Release.tag_name.TrimStart('v')
    Write-Host "-> Latest available release: v$LatestTag" -ForegroundColor Green
} catch {
    Write-Error "Failed to reach GitHub Releases API: $_"
    exit 1
}

# 3. Compare Versions
$IsUpdateNeeded = $false
if ($Force -or ($InstalledVersion -eq "0.0.0")) {
    $IsUpdateNeeded = $true
} else {
    $vCurrent = [version]$InstalledVersion
    $vLatest  = [version]$LatestTag
    if ($vLatest -gt $vCurrent) {
        $IsUpdateNeeded = $true
    }
}

if (-not $IsUpdateNeeded) {
    Write-Host "`n✓ You are already on the latest version (v$InstalledVersion). No update needed." -ForegroundColor Green
    exit 0
}

Write-Host "`n🚀 New version v$LatestTag is available! Preparing upgrade..." -ForegroundColor Cyan

# 4. Find Setup Executable Asset
$Asset = $Release.assets | Where-Object { $_.name -like "*Setup*.exe" } | Select-Object -First 1
if (-not $Asset) {
    Write-Host "-> Standalone installer not attached to release assets. Checking release ZIP..." -ForegroundColor Yellow
    $Asset = $Release.assets | Where-Object { $_.name -like "*.zip" } | Select-Object -First 1
}

if (-not $Asset) {
    Write-Error "No downloadable asset found for release v$LatestTag. Please visit: $($Release.html_url)"
    exit 1
}

# 5. Download Asset
$TempDir = Join-Path $env:TEMP "JDMixAIEngine_Update"
New-Item -ItemType Directory -Force -Path $TempDir | Out-Null
$InstallerPath = Join-Path $TempDir $Asset.name

Write-Host "-> Downloading $($Asset.name)..." -ForegroundColor Gray
Invoke-WebRequest -Uri $Asset.browser_download_url -OutFile $InstallerPath

# 6. Check for running FL Studio or JD Mix AI Engine processes
$RunningDaw = Get-Process -Name "FL64", "FL", "JD Mix AI Engine" -ErrorAction SilentlyContinue
if ($RunningDaw) {
    Write-Host "-> Running audio software detected ($($RunningDaw.ProcessName -join ', '))." -ForegroundColor Yellow
    $Prompt = Read-Host "Close audio processes now to complete update? (Y/N)"
    if ($Prompt -match "^[yY]") {
        Stop-Process -Name $RunningDaw.ProcessName -Force -ErrorAction SilentlyContinue
        Start-Sleep -Seconds 2
    } else {
        Write-Host "Aborted by user. Please save your project and run the updater again." -ForegroundColor Red
        exit 1
    }
}

# 7. Execute Installer
Write-Host "-> Executing in-place upgrade..." -ForegroundColor Cyan
if ($Silent) {
    Start-Process -FilePath $InstallerPath -ArgumentList "/VERYSILENT /SUPPRESSMSGBOXES /NORESTART" -Wait
} else {
    Start-Process -FilePath $InstallerPath -Wait
}

Write-Host "`n✓ JD Mix AI Engine successfully upgraded to v$LatestTag!" -ForegroundColor Green
Write-Host "Your custom presets and mix memories are fully preserved.`n" -ForegroundColor DarkGray
