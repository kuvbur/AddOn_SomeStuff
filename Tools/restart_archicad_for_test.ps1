# ==============================================================================
# НАСТРОЙКИ ПАУЗ И ТАЙМ-АУТОВ (в секундах)
# ==============================================================================
$processCloseTimeoutSec      = 15  # Время ожидания мягкого закрытия Archicad (сек)
$postCloseCleanupPauseSec    = 15  # Пауза после закрытия перед очисткой и сборкой (сек)
$initialLaunchWaitSec        = 10   # Пауза сразу после запуска Archicad перед проверкой процесса (сек)
$testResultTimeoutSec        = 20  # Дополнительный таймаут ожидания создания test_results.txt (сек)
$testResultCheckIntervalSec = 2   # Интервал проверки появления файла test_results.txt (сек)

# ==============================================================================
# ПУТИ И КОНФИГУРАЦИЯ
# ==============================================================================
$scriptDir       = $PSScriptRoot
$projectRoot     = Split-Path -Parent $scriptDir
$configPath      = Join-Path -Path $projectRoot -ChildPath "config.json"
$buildScriptPath = Join-Path -Path $scriptDir -ChildPath "BuildAddOn.py"
$testResultsPath = Join-Path -Path $projectRoot -ChildPath "test_results.txt"

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
# 1. ПОИСК И ЗАКРЫТИЕ ПРЕДЫДУЩЕГО ARCHICAD
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
        
        # Переводим секунды в миллисекунды для WaitForExit
        if (-not $proc.WaitForExit($processCloseTimeoutSec * 1000)) {
            Write-Host "Process did not respond in time. Force stopping..." -ForegroundColor Red
            Stop-Process -Id $proc.Id -Force
        }
    }
    
    Write-Host "Waiting $postCloseCleanupPauseSec seconds before cleaning up..." -ForegroundColor Cyan
    Start-Sleep -Seconds $postCloseCleanupPauseSec
} else {
    Write-Host "Archicad with '$fileName' is not running." -ForegroundColor Cyan
}

# ==============================================================================
# 2. УДАЛЕНИЕ ВРЕМЕННЫХ ФАЙЛОВ (.LCK и test_results.txt)
# ==============================================================================
if (Test-Path -Path $lckFilePath) {
    Write-Host "Removing lock file: $lckFilePath" -ForegroundColor Yellow
    Remove-Item -Path $lckFilePath -Force -ErrorAction SilentlyContinue
}

if (Test-Path -Path $testResultsPath) {
    Write-Host "Removing old test results: $testResultsPath" -ForegroundColor Yellow
    Remove-Item -Path $testResultsPath -Force -ErrorAction SilentlyContinue
}

# ==============================================================================
# 3. ЗАПУСК СБОРКИ (Tools\BuildAddOn.py)
# ==============================================================================
Write-Host "Starting build via Tools\BuildAddOn.py..." -ForegroundColor Cyan

# Настройка UTF-8 для корректного отображения вывода Python/MSBuild
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding           = [System.Text.Encoding]::UTF8
$env:PYTHONIOENCODING     = "utf-8"
$env:VSLANG               = "1033"

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

# ==============================================================================
# 5. ПРОВЕРКА ЗАПУСКА, ОЖИДАНИЕ ТЕСТОВ И ЗАКРЫТИЕ ARCHICAD
# ==============================================================================
Write-Host "Waiting $initialLaunchWaitSec seconds for Archicad process to start..." -ForegroundColor Cyan
Start-Sleep -Seconds $initialLaunchWaitSec

# Проверяем, появился ли процесс Archicad
$checkLaunched = Get-Process -Name "ARCHICAD*" -ErrorAction SilentlyContinue | Where-Object {
    $procId = $_.Id
    $cmdLine = (Get-CimInstance Win32_Process -Filter "ProcessId = $procId").CommandLine
    return ($cmdLine -like "*$fileName*") -or ($_.MainWindowTitle -like "*$fileName*")
}

if (-not $checkLaunched) {
    Write-Host "`n[ERROR] Failed to verify Archicad startup. Process with '$fileName' was not found!" -ForegroundColor Red
    exit 1
}

$pidList = ($checkLaunched | Select-Object -ExpandProperty Id) -join ", "
Write-Host "[SUCCESS] Archicad process verified (PID: $pidList)." -ForegroundColor Green

# Ожидание создания файла test_results.txt
Write-Host "Waiting for test_results.txt (Timeout: ${testResultTimeoutSec}s)..." -ForegroundColor Cyan
$elapsed = 0

while (-not (Test-Path -Path $testResultsPath) -and ($elapsed -lt $testResultTimeoutSec)) {
    Start-Sleep -Seconds $testResultCheckIntervalSec
    $elapsed += $testResultCheckIntervalSec
    Write-Host "Still waiting for test_results.txt (${elapsed}/${testResultTimeoutSec}s)..." -ForegroundColor Gray
}

# Вывод результатов
if (Test-Path -Path $testResultsPath) {
    Write-Host "`n------------------- TEST RESULTS -------------------" -ForegroundColor Cyan
    Get-Content -Path $testResultsPath -Encoding UTF8 | ForEach-Object { Write-Host $_ }
    Write-Host "----------------------------------------------------`n" -ForegroundColor Cyan
} else {
    Write-Host "`n[WARNING] Timeout reached (${testResultTimeoutSec}s)! test_results.txt was not created." -ForegroundColor Yellow
}

# Закрытие запущенного процесса Archicad
Write-Host "Closing launched Archicad process..." -ForegroundColor Yellow
foreach ($proc in $checkLaunched) {
    $null = $proc.CloseMainWindow()
    
    if (-not $proc.WaitForExit($processCloseTimeoutSec * 1000)) {
        Write-Host "Process did not close in time. Force stopping..." -ForegroundColor Red
        Stop-Process -Id $proc.Id -Force
    }
}
Write-Host "[SUCCESS] Archicad successfully closed." -ForegroundColor Green