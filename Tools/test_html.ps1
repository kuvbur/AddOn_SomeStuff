<# 
.SYNOPSIS
    Standalone HTML validation script for SomeStuff AddOn BrowserPalette.
    Runs HTMLHint + custom verify.js (ТЗ checks) for BOTH Interface_ru.html and
    Interface_en.html, plus an i18n catalog freshness check.
    Does not build or launch Archicad.

.DESCRIPTION
    Use this when you only edit HTML/JS and want quick feedback.
    Exits with code 0 on success, non-zero on failure.

.EXAMPLE
    powershell -File Tools\test_html.ps1
    powershell -File Tools\test_html.ps1 -ProjectRoot "D:\SomeStuff_addon"
    powershell -File Tools\test_html.ps1 -HtmlFile "Sources\AddOnResources\RFIX\HTML\Interface_ru.html"
    powershell -File Tools\test_html.ps1 -SkipEn

.NOTES
    Requires: node.js, npm packages (htmlhint, eslint) - run 'npm install' once.
#>

param (
    [string]$ProjectRoot = (Split-Path -Parent $PSScriptRoot),
    [string]$HtmlFile = "Sources\AddOnResources\RFIX\HTML\Interface_ru.html",
    [string]$VerifyScript = "Tools\verify.js",
    [string]$PackageJson = "package.json",
    [string]$HtmlFileEn = "Sources\AddOnResources\RFIX\HTML\Interface_en.html",
    [string]$I18nScript = "Tools\localize_html.js",
    [switch]$SkipEn
)

$ErrorActionPreference = "Stop"

function Write-Log {
    param (
        [Parameter(Mandatory = $true)][string]$Message,
        [ConsoleColor]$Color = [ConsoleColor]::Gray
    )
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$timestamp] $Message" -ForegroundColor $Color
}

function Test-FileExists {
    param ([string]$Path, [string]$Description)
    $fullPath = Join-Path $ProjectRoot $Path
    if (-not (Test-Path -LiteralPath $fullPath)) {
        Write-Log "$Description not found: $fullPath" Red
        return $false
    }
    return $true
}

Write-Log "=== STANDALONE HTML VALIDATION ===" Cyan
Write-Log "Project root: $ProjectRoot" Gray

# Check required files
$allOk = $true
$allOk = (Test-FileExists $HtmlFile "HTML file") -and $allOk
$allOk = (Test-FileExists $VerifyScript "verify.js") -and $allOk
$allOk = (Test-FileExists $PackageJson "package.json") -and $allOk
if (-not $SkipEn) {
    $allOk = (Test-FileExists $HtmlFileEn "EN HTML file") -and $allOk
    $allOk = (Test-FileExists $I18nScript "localize_html.js") -and $allOk
}

if (-not $allOk) {
    Write-Log "Missing required files. Aborting." Red
    exit 1
}

$htmlPath = Join-Path $ProjectRoot $HtmlFile
$verifyPath = Join-Path $ProjectRoot $VerifyScript
$htmlPathEn = Join-Path $ProjectRoot $HtmlFileEn
$i18nPath = Join-Path $ProjectRoot $I18nScript

# Список файлов для проверки: EN подключаем, только если он нужен этой проверке
$targets = @($htmlPath)
if (-not $SkipEn) { $targets += $htmlPathEn }

# Ensure node_modules exists
$nodeModules = Join-Path $ProjectRoot "node_modules"
if (-not (Test-Path -LiteralPath $nodeModules)) {
    Write-Log "node_modules not found, running npm install..." Yellow
    try {
        Set-Location -LiteralPath $ProjectRoot
        $npmOutput = @(& npm install 2>&1)
        $npmCode = $LASTEXITCODE
        foreach ($line in $npmOutput) { Write-Host $line }
        if ($npmCode -ne 0) {
            Write-Log "npm install failed (exit $npmCode)" Red
            exit 2
        }
    }
    catch {
        Write-Log "npm install failed to start: $($_.Exception.Message)" Red
        exit 2
    }
}

# 1. HTMLHint
Write-Log "Running HTMLHint..." Cyan
try {
    Set-Location -LiteralPath $ProjectRoot
    $hintOutput = @(& npx htmlhint @targets 2>&1)
    $hintCode = $LASTEXITCODE
    foreach ($line in $hintOutput) { Write-Host $line }
    if ($hintCode -ne 0) {
        Write-Log "HTMLHint FAILED" Red
        exit 3
    }
    Write-Log "HTMLHint: PASSED" Green
}
catch {
    Write-Log "HTMLHint error: $($_.Exception.Message)" Red
    exit 3
}

# 2. Custom verify.js (ТЗ checks)
Write-Log "Running custom verify.js (ТЗ checks)..." Cyan
try {
    Set-Location -LiteralPath $ProjectRoot
    $verifyOutput = @(& node $verifyPath @targets 2>&1)
    $verifyCode = $LASTEXITCODE
    foreach ($line in $verifyOutput) { Write-Host $line }
    if ($verifyCode -ne 0) {
        Write-Log "Custom verify.js FAILED" Red
        exit 4
    }
    Write-Log "Custom verify.js: PASSED" Green
}
catch {
    Write-Log "verify.js error: $($_.Exception.Message)" Red
    exit 4
}

# 3. Каталог переводов не отстал от RU-исходника
if (-not $SkipEn) {
    Write-Log "Running i18n check..." Cyan
    try {
        Set-Location -LiteralPath $ProjectRoot
        $i18nOutput = @(& node $i18nPath check 2>&1)
        $i18nCode = $LASTEXITCODE
        foreach ($line in $i18nOutput) { Write-Host $line }
        if ($i18nCode -ne 0) {
            Write-Log "i18n check FAILED (EN catalog is stale)" Red
            exit 5
        }
        Write-Log "i18n check: PASSED" Green
    }
    catch {
        Write-Log "i18n check error: $($_.Exception.Message)" Red
        exit 5
    }
}

Write-Log "=== ALL HTML CHECKS PASSED ===" Green
exit 0