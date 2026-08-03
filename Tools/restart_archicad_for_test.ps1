# ==============================================================================
# ПУТИ И НАСТРОЙКИ
# Скрипт и BuildAddOn.py находятся в папке \Tools
# ==============================================================================
$scriptDir       = $PSScriptRoot
$projectRoot     = Split-Path -Parent $scriptDir
$configPath      = Join-Path -Path $projectRoot -ChildPath "config.json"
$buildScriptPath = Join-Path -Path $scriptDir -ChildPath "BuildAddOn.py"

# Считываем путь к тестовому файлу из config.json
if (Test-Path -Path $configPath) {
    $config = Get-Content -Raw -Path $configPath -Encoding UTF8 | ConvertFrom-Json
    $filePath = $config.filePath
}

# Резервный путь, если в config.json не задан filePath
if (-not $filePath) {
    $filePath = Join-Path -Path $projectRoot -ChildPath "Test_file\test_25.pln"
}

$lckFilePath = "$filePath.lck"
$fileName    = [System.IO.Path]::GetFileNameWithoutExtension($filePath)

# ==============================================================================
# 1. ПОИСК И ЗАКРЫТИЕ ЗАПУЩЕННОГО ARCHICAD
# ==============================================================================
$runningProcesses = Get-Process -Name "ARCHICAD*" -ErrorAction SilentlyContinue | Where-Object {
    $procId = $_.Id
    $cmdLine = (Get-CimInstance Win32_Process -Filter "ProcessId = $procId").CommandLine
    return ($cmdLine -like "*$fileName*") -or ($_.MainWindowTitle -like "*$fileName*")
}

if ($runningProcesses) {
    Write-Host "Found running Archicad with '$fileName'. Closing..." -ForegroundColor Yellow
    
    foreach ($proc in $runningProcesses) {
        $null = $proc.CloseMainWindow()
        
        if (-not $proc.WaitForExit(10000)) {
            Write-Host "Process did not respond in time. Force stopping..." -ForegroundColor Red
            Stop-Process -Id $proc.Id -Force
        }
    }
    
    Write-Host "Waiting 10 seconds before cleaning up..." -ForegroundColor Cyan
    Start-Sleep -Seconds 10
} else {
    Write-Host "Archicad with '$fileName' is not running." -ForegroundColor Cyan
}

# ==============================================================================
# 2. УДАЛЕНИЕ .LCK ФАЙЛА
# ==============================================================================
if (Test-Path -Path $lckFilePath) {
    Write-Host "Removing lock file: $lckFilePath" -ForegroundColor Yellow
    Remove-Item -Path $lckFilePath -Force -ErrorAction SilentlyContinue
}

# ==============================================================================
# 3. ЗАПУСК СБОРКИ (Tools\BuildAddOn.py)
# ==============================================================================
Write-Host "Starting build via Tools\BuildAddOn.py..." -ForegroundColor Cyan

# Настройка UTF-8 для корректного отображения вывода Python/MSBuild
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding           = [System.Text.Encoding]::UTF8
$env:PYTHONIOENCODING     = "utf-8"

$env:VSLANG = "1033"

$previousLocation = Get-Location
Set-Location -Path $projectRoot

# Запуск BuildAddOn.py из папки Tools с выполнением в контексте корня проекта
$buildOutput = & python $buildScriptPath --configFile config.json --acVersion 25 2>&1
$buildExitCode = $LASTEXITCODE

Set-Location -Path $previousLocation

# Вывод лога сборки в консоль
$buildOutput | ForEach-Object { Write-Host $_ }

# Проверка на ошибки
$hasBuildError = ($buildExitCode -ne 0) -or ($buildOutput -match "Failed to build project")

if ($hasBuildError) {
    Write-Host "`n[ERROR] Build failed! Archicad will NOT be started." -ForegroundColor Red
    exit 1
}

Write-Host "`n[SUCCESS] Build completed successfully." -ForegroundColor Green

# ==============================================================================
# 4. ЗАПУСК ARCHICAD
# ==============================================================================
Write-Host "Starting $filePath..." -ForegroundColor Green
Start-Process -FilePath $filePath