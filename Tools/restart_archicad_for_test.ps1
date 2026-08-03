#requires -Version 5.1

# ==============================================================================
# ARCHICAD ADD-ON AUTOMATED TEST RUNNER
#
# Назначение:
#   1. Проверить окружение и конфигурацию.
#   2. Закрыть Archicad, оставшийся от предыдущего запуска.
#   3. Очистить временные файлы.
#   4. Собрать Add-On.
#   5. Запустить ARCHICAD.exe с файлом PLN и сервисными флагами.
#   6. Дождаться test_results.txt.
#   7. Проанализировать результаты тестов.
#   8. В ЛЮБОМ случае попытаться корректно завершить запущенный Archicad.
#   9. При необходимости принудительно завершить процесс (tree kill).
#  10. Проверить, что Archicad действительно завершён.
# ==============================================================================

$ErrorActionPreference = "Stop"


# ==============================================================================
# 1. EXIT CODES
# ==============================================================================

$EXIT_SUCCESS                 = 0
$EXIT_CONFIG_ERROR            = 10
$EXIT_PREVIOUS_AC_FAILED      = 20
$EXIT_BUILD_FAILED            = 30
$EXIT_AC_START_FAILED         = 40
$EXIT_RUNTIME_ERROR            = 50
$EXIT_TEST_TIMEOUT             = 60
$EXIT_TESTS_FAILED             = 70
$EXIT_AC_SHUTDOWN_FAILED       = 80
$EXIT_CLEANUP_FAILED           = 90


# ==============================================================================
# 2. НАСТРОЙКИ (DEFAULTS)
# ==============================================================================

# Для выделенной тестовой машины / CI:
$killExistingArchicad = $true

# Время graceful shutdown.
$gracefulCloseTimeoutSec = 30

# Время ожидания после force kill.
$forceCloseTimeoutSec = 15

# Время ожидания запуска Archicad.
$archicadLaunchTimeoutSec = 60

# Время ожидания test_results.txt.
$testResultTimeoutSec = 120

# Интервал polling.
$pollIntervalSec = 2

# Пауза после подтверждённого завершения Archicad.
$postCloseCleanupPauseSec = 5

# Retry удаления файлов.
$fileDeleteRetries = 10

# Задержка между retry удаления.
$fileDeleteRetryDelayMs = 500


# ==============================================================================
# 3. ПУТИ
# ==============================================================================

$scriptDir       = $PSScriptRoot
$projectRoot     = Split-Path -Parent $scriptDir
$configPath      = Join-Path $projectRoot "config.json"
$buildScriptPath = Join-Path $scriptDir "BuildAddOn.py"
$testResultsPath = Join-Path $projectRoot "test_results.txt"


# ==============================================================================
# 4. LOGGING
# ==============================================================================

function Write-Log {
    param (
        [Parameter(Mandatory = $true)]
        [string]$Message,

        [ConsoleColor]$Color = [ConsoleColor]::Gray
    )

    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Write-Host "[$timestamp] $Message" -ForegroundColor $Color
}


# ==============================================================================
# 5. ARCHICAD PROCESS HELPERS
# ==============================================================================

function Get-ArchicadProcesses {
    @(
        Get-Process -Name "ARCHICAD*" -ErrorAction SilentlyContinue |
        Where-Object { $_.ProcessName -notlike "*server*" }
    )
}


function Get-ArchicadProcessIds {
    @(
        Get-ArchicadProcesses | Select-Object -ExpandProperty Id
    )
}


function Test-ProcessExists {
    param (
        [Parameter(Mandatory = $true)]
        [int]$ProcessId
    )

    return $null -ne (Get-Process -Id $ProcessId -ErrorAction SilentlyContinue)
}


function Wait-ArchicadProcessesExit {
    param (
        [Parameter(Mandatory = $true)]
        [int[]]$ProcessIds,

        [Parameter(Mandatory = $true)]
        [int]$TimeoutSec
    )

    if ($ProcessIds.Count -eq 0) {
        return $true
    }

    $deadline = (Get-Date).AddSeconds($TimeoutSec)

    while ((Get-Date) -lt $deadline) {
        $alive = @($ProcessIds | Where-Object { Test-ProcessExists -ProcessId $_ })
        if ($alive.Count -eq 0) {
            return $true
        }
        Start-Sleep -Milliseconds 500
    }

    $alive = @($ProcessIds | Where-Object { Test-ProcessExists -ProcessId $_ })
    return ($alive.Count -eq 0)
}


# ==============================================================================
# 6. FILE HELPERS
# ==============================================================================

function Remove-FileWithRetry {
    param (
        [Parameter(Mandatory = $true)]
        [string]$Path,

        [int]$Retries = 10
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        return $true
    }

    for ($attempt = 1; $attempt -le $Retries; $attempt++) {
        try {
            Remove-Item -LiteralPath $Path -Force -ErrorAction Stop
            if (-not (Test-Path -LiteralPath $Path)) {
                return $true
            }
        }
        catch {
            Write-Log "Unable to remove '$Path' (attempt $attempt/$Retries)." DarkYellow
        }
        Start-Sleep -Milliseconds $fileDeleteRetryDelayMs
    }

    Write-Log "FAILED to remove: $Path" Red
    return $false
}


# ==============================================================================
# 7. FORCE KILL
# ==============================================================================

function Stop-ArchicadProcessTree {
    param (
        [Parameter(Mandatory = $true)]
        [int]$ProcessId
    )

    if (-not (Test-ProcessExists -ProcessId $ProcessId)) {
        return
    }
    try {
        $output = & taskkill.exe /F /T /PID $ProcessId 2>&1
    }
    catch {
        Write-Log "taskkill failed for PID ${ProcessId}: $($_.Exception.Message)" DarkYellow
    }

    try {
        Stop-Process -Id $ProcessId -Force -ErrorAction SilentlyContinue
    }
    catch {
        # Процесс мог уже завершиться через taskkill
    }
}


# ==============================================================================
# 8. GRACEFUL + FORCE SHUTDOWN КОНКРЕТНЫХ ПРОЦЕССОВ
# ==============================================================================

function Stop-TrackedArchicad {
    param (
        [Parameter(Mandatory = $true)]
        [int[]]$ProcessIds,

        [string]$Reason = "cleanup"
    )

    if ($ProcessIds.Count -eq 0) {
        return $true
    }

    Write-Log "Stopping tracked Archicad process(es). Reason: $Reason" Yellow

    # 8.1. Graceful shutdown
    foreach ($processId in $ProcessIds) {
        $process = Get-Process -Id $processId -ErrorAction SilentlyContinue
        if (-not $process) { continue }
        try {
            if ($process.MainWindowHandle -ne 0) {
                $null = $process.CloseMainWindow()
            } else {
                Write-Log "PID ${processId} has no main window." DarkYellow
            }
        }
        catch {
            Write-Log "Graceful close failed for PID ${processId}: $($_.Exception.Message)" DarkYellow
        }
    }

    # 8.2. Ожидание завершения
    if (Wait-ArchicadProcessesExit -ProcessIds $ProcessIds -TimeoutSec $gracefulCloseTimeoutSec) {
        return $true
    }


    # 8.3. Force kill
    foreach ($processId in $ProcessIds) {
        if (Test-ProcessExists -ProcessId $processId) {
            Stop-ArchicadProcessTree -ProcessId $processId
        }
    }

    # 8.4. Проверка force kill
    if (Wait-ArchicadProcessesExit -ProcessIds $ProcessIds -TimeoutSec $forceCloseTimeoutSec) {
        return $true
    }

    # 8.5. Диагностика ошибок
    $alive = @($ProcessIds | Where-Object { Test-ProcessExists -ProcessId $_ })
    if ($alive.Count -gt 0) {
        Write-Log "CRITICAL: tracked Archicad process(es) are still running." Red
        foreach ($processId in $alive) {
            $process = Get-Process -Id $processId -ErrorAction SilentlyContinue
            if ($process) {
                Write-Log "PID=$($process.Id), Name=$($process.ProcessName), Window='$($process.MainWindowTitle)'" Red
            }
        }
        return $false
    }

    return $true
}


# ==============================================================================
# 9. ЗАКРЫТИЕ ПРЕДЫДУЩИХ ARCHICAD
# ==============================================================================

function Stop-ExistingArchicad {
    $processes = @(Get-ArchicadProcesses)

    if ($processes.Count -eq 0) {
        return $true
    }

    if (-not $killExistingArchicad) {
        Write-Log "Existing Archicad detected, but automatic termination is disabled." Red
        return $false
    }

    $ids = @($processes | Select-Object -ExpandProperty Id)
    Write-Log "Found $($ids.Count) existing Archicad process(es)." Yellow

    if (-not (Stop-TrackedArchicad -ProcessIds $ids -Reason "pre-test cleanup")) {
        return $false
    }

    Start-Sleep -Seconds 1
    $remaining = @(Get-ArchicadProcesses)

    if ($remaining.Count -gt 0) {
        Write-Log "Additional Archicad process(es) detected after shutdown." Yellow
        $remainingIds = @($remaining | Select-Object -ExpandProperty Id)

        if (-not (Stop-TrackedArchicad -ProcessIds $remainingIds -Reason "pre-test final cleanup")) {
            return $false
        }
    }

    return (@(Get-ArchicadProcesses).Count -eq 0)
}


# ==============================================================================
# 10. ОЖИДАНИЕ ЗАПУСКА КОНКРЕТНОГО ARCHICAD
# ==============================================================================

function Wait-ForNewArchicad {
    param (
        [AllowEmptyCollection()]
        [int[]]$PidsBefore = @(),

        [Parameter(Mandatory = $true)]
        [int]$TimeoutSec
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSec)

    while ((Get-Date) -lt $deadline) {
        $processes = @(Get-ArchicadProcesses | Where-Object { $_.Id -notin $PidsBefore })
        if ($processes.Count -gt 0) {
            return $processes
        }
        Start-Sleep -Seconds $pollIntervalSec
    }

    return @()
}


# ==============================================================================
# 11. РАЗБОР test_results.txt
# ==============================================================================

function Get-TestResultStatus {
    param (
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        return "MISSING"
    }

    $lines = $null
    for ($attempt = 1; $attempt -le 5; $attempt++) {
        try {
            $lines = @(Get-Content -LiteralPath $Path -Encoding UTF8 -ErrorAction Stop)
            break
        }
        catch {
            Start-Sleep -Milliseconds 200
        }
    }

    if ($null -eq $lines) {
        Write-Log "Unable to read test results after multiple retries." Red
        return "ERROR"
    }

    $text = $lines -join "`n"

    if ($text -match "(?im)===\s*ERROR IN TEST\s*===") {
        return "FAILED"
    }

    if ($text -match "(?im)^\s*(FAIL|FAILED|ERROR)\s*$") {
        return "FAILED"
    }
    if ($text -match "(?im)TESTS\s+FAILED\s*:\s*[1-9][0-9]*") {
        return "FAILED"
    }

    if ($text -match "(?im)^\s*(PASS|PASSED|SUCCESS|SUCCESSFUL|ok)\s*$") {
        return "PASSED"
    }
    if ($text -match "(?im)TESTS\s+FAILED\s*:\s*0") {
        return "PASSED"
    }
    if ($text -match "(?im)TESTS\s+PASSED\s*:\s*[1-9][0-9]*") {
        return "PASSED"
    }

    return "UNKNOWN"
}


# ==============================================================================
# 12. ПЕРЕМЕННЫЕ RUNNER
# ==============================================================================

$runnerExitCode      = $EXIT_SUCCESS
$archicadStarted     = $false
$trackedArchicadPids = @()
$testResultStatus    = "NOT_RUN"
$buildSucceeded      = $false


# ==============================================================================
# 13. MAIN
# ==============================================================================

try {
    # CONFIG


    if (-not (Test-Path -LiteralPath $configPath)) {
        throw "config.json not found: $configPath"
    }

    if (-not (Test-Path -LiteralPath $buildScriptPath)) {
        throw "BuildAddOn.py not found: $buildScriptPath"
    }

    $config = Get-Content -Raw -LiteralPath $configPath -Encoding UTF8 | ConvertFrom-Json
    
    # Версия Archicad
    $acVersion = if ($config.acVersion) { $config.acVersion } else { "25" }

    # Поиск ARCHICAD.exe
    $archicadExePath = $config.archicadExePath
    if (-not $archicadExePath -or -not (Test-Path -LiteralPath $archicadExePath)) {
        $archicadExePath = "C:\Program Files\GRAPHISOFT\ARCHICAD $acVersion\ARCHICAD.exe"
    }

    if (-not (Test-Path -LiteralPath $archicadExePath)) {
        $runnerExitCode = $EXIT_CONFIG_ERROR
        throw "ARCHICAD.exe not found at path: '$archicadExePath'. Check 'archicadExePath' or 'acVersion' in config.json."
    }

    # Файл тестового проекта PLN
    $filePath = $config.filePath
    if (-not $filePath) {
        $filePath = Join-Path $projectRoot "Test_file\test_$acVersion.pln"
    }

    $filePath = [System.IO.Path]::GetFullPath($filePath)
    $lckFilePath = "$filePath.lck"

    if (-not (Test-Path -LiteralPath $filePath)) {
        throw "Test PLN not found: $filePath"
    }
    Write-Log "Results:       $testResultsPath" Gray

    # PREVIOUS ARCHICAD
    if (-not (Stop-ExistingArchicad)) {
        $runnerExitCode = $EXIT_PREVIOUS_AC_FAILED
        throw "Unable to guarantee that previous Archicad processes are terminated."
    }

    Start-Sleep -Seconds $postCloseCleanupPauseSec

    # CLEANUP


    if (-not (Remove-FileWithRetry -Path $lckFilePath -Retries $fileDeleteRetries)) {
        $runnerExitCode = $EXIT_CLEANUP_FAILED
        throw "Unable to remove Archicad lock file."
    }

    if (-not (Remove-FileWithRetry -Path $testResultsPath -Retries $fileDeleteRetries)) {
        $runnerExitCode = $EXIT_CLEANUP_FAILED
        throw "Unable to remove old test_results.txt."
    }

    # ENVIRONMENT
    [Console]::OutputEncoding = [System.Text.Encoding]::UTF8
    $OutputEncoding           = [System.Text.Encoding]::UTF8
    $env:PYTHONIOENCODING     = "utf-8"
    $env:VSLANG               = "1033"

    # BUILD
    $previousLocation = Get-Location

    try {
        Set-Location -LiteralPath $projectRoot
        $buildOutput = @(& python $buildScriptPath --configFile config.json --acVersion $acVersion 2>&1)
        $buildExitCode = $LASTEXITCODE
    }
    catch {
        Set-Location -LiteralPath $previousLocation
        $runnerExitCode = $EXIT_BUILD_FAILED
        throw "Build process failed to start: $($_.Exception.Message)"
    }

    Set-Location -LiteralPath $previousLocation

    foreach ($line in $buildOutput) { Write-Host $line }

    if ($buildExitCode -ne 0) {
        $runnerExitCode = $EXIT_BUILD_FAILED
        throw "Build failed. Python exit code: $buildExitCode"
    }

    $buildOutputText = $buildOutput -join "`n"
    if ($buildOutputText -match "Failed to build project") {
        $runnerExitCode = $EXIT_BUILD_FAILED
        throw "Build script reported 'Failed to build project'."
    }

    $buildSucceeded = $true
    Write-Log "Build completed successfully." Green

    # ARCHICAD START
    $pidsBefore = @(Get-ArchicadProcessIds)



    # Передаём файл PLN первым аргументом, затем сервисные флаги
    $acArgs = @(
        "`"$filePath`"",
        "-forceaccessdialog",
        "-bringToFront",
        "-DISABLERECOVERYDIALOG"
    )

    $startedProcess = Start-Process -FilePath $archicadExePath -ArgumentList $acArgs -PassThru
    Write-Log "Waiting for Archicad startup (timeout ${archicadLaunchTimeoutSec}s)..." Cyan

    $newProcesses = Wait-ForNewArchicad -PidsBefore $pidsBefore -TimeoutSec $archicadLaunchTimeoutSec

    if ($newProcesses.Count -eq 0) {
        $runnerExitCode = $EXIT_AC_START_FAILED
        throw "Archicad process was not detected after launch."
    }

    $trackedArchicadPids = @($newProcesses | Select-Object -ExpandProperty Id)
    $archicadStarted     = $true



    # TEST WAITING
    Write-Log "Waiting for test_results.txt (timeout ${testResultTimeoutSec}s)..." Cyan

    $deadline = (Get-Date).AddSeconds($testResultTimeoutSec)
    $lastElapsed = -1

    while ((Get-Date) -lt $deadline) {
        if (Test-Path -LiteralPath $testResultsPath) {
            Write-Log "test_results.txt detected." Green
            break
        }

        $aliveTracked = @($trackedArchicadPids | Where-Object { Test-ProcessExists -ProcessId $_ })
        if ($aliveTracked.Count -eq 0) {
            $runnerExitCode = $EXIT_RUNTIME_ERROR
            throw "Tracked Archicad process terminated unexpectedly before test_results.txt was created."
        }

        $elapsed = [int]((Get-Date).Subtract($deadline.AddSeconds(-$testResultTimeoutSec)).TotalSeconds)
        if ($elapsed -ne $lastElapsed) {
            $lastElapsed = $elapsed
        }

        Start-Sleep -Seconds $pollIntervalSec
    }

    # TIMEOUT CHECK
    if (-not (Test-Path -LiteralPath $testResultsPath)) {
        $testResultStatus = "TIMEOUT"
        $runnerExitCode   = $EXIT_TEST_TIMEOUT
        throw "test_results.txt was not created within $testResultTimeoutSec seconds."
    }

    # READ TEST RESULTS
    Write-Host ""
    Write-Host "================ TEST RESULTS ================" -ForegroundColor Cyan
    Get-Content -LiteralPath $testResultsPath -Encoding UTF8 | ForEach-Object { Write-Host $_ }
    Write-Host "==============================================" -ForegroundColor Cyan
    Write-Host ""

    # PARSE RESULT
    $testResultStatus = Get-TestResultStatus -Path $testResultsPath

    switch ($testResultStatus) {
        "PASSED" {
            Write-Log "Automated tests: PASSED." Green
        }
        "FAILED" {
            Write-Log "Automated tests: FAILED." Red
            $runnerExitCode = $EXIT_TESTS_FAILED
        }
        "UNKNOWN" {
            Write-Log "Test result file exists, but its status is not recognized." Yellow
        }
        "ERROR" {
            $runnerExitCode = $EXIT_RUNTIME_ERROR
            throw "Unable to parse test_results.txt."
        }
    }
}
catch {
    Write-Log "RUNNER ERROR: $($_.Exception.Message)" Red
    if ($runnerExitCode -eq $EXIT_SUCCESS) {
        $runnerExitCode = $EXIT_RUNTIME_ERROR
    }
}


# ==============================================================================
# 14. FINALLY / MANDATORY ARCHICAD SHUTDOWN
# ==============================================================================

if ($archicadStarted -and $trackedArchicadPids.Count -gt 0) {
    $shutdownOk = Stop-TrackedArchicad -ProcessIds $trackedArchicadPids -Reason "post-test cleanup"

    if (-not $shutdownOk) {
        Write-Log "CRITICAL: tracked Archicad process could not be terminated." Red
        $runnerExitCode = $EXIT_AC_SHUTDOWN_FAILED
    }
}


# ==============================================================================
# 15. GLOBAL ARCHICAD VERIFICATION
# ==============================================================================

Start-Sleep -Seconds $postCloseCleanupPauseSec
$remainingArchicad = @(Get-ArchicadProcesses)

if ($remainingArchicad.Count -gt 0) {
    Write-Log "CRITICAL: Archicad process(es) remain after shutdown." Red

    foreach ($process in $remainingArchicad) {
        try {
            Write-Log "Remaining PID=$($process.Id), Name=$($process.ProcessName), Window='$($process.MainWindowTitle)'" Red
        }
        catch {
            Write-Log "Remaining PID=$($process.Id)" Red
        }
    }

    $remainingIds = @($remainingArchicad | Select-Object -ExpandProperty Id)
    foreach ($processId in $remainingIds) {
        Stop-ArchicadProcessTree -ProcessId $processId
    }

    if (-not (Wait-ArchicadProcessesExit -ProcessIds $remainingIds -TimeoutSec $forceCloseTimeoutSec)) {
        Write-Log "CRITICAL: Archicad is STILL RUNNING after final force kill." Red
        $runnerExitCode = $EXIT_AC_SHUTDOWN_FAILED
    }
}


# ==============================================================================
# 16. FINAL CLEANUP
# ==============================================================================

if (@(Get-ArchicadProcesses).Count -eq 0) {
    if (Test-Path -LiteralPath $lckFilePath) {
        if (-not (Remove-FileWithRetry -Path $lckFilePath -Retries $fileDeleteRetries)) {
            Write-Log "WARNING: Archicad lock file could not be removed." Yellow
            if ($runnerExitCode -eq $EXIT_SUCCESS) {
                $runnerExitCode = $EXIT_CLEANUP_FAILED
            }
        }
    }
} else {
    Write-Log "Skipping lock-file cleanup because Archicad is still running." Red
    if ($runnerExitCode -eq $EXIT_SUCCESS) {
        $runnerExitCode = $EXIT_AC_SHUTDOWN_FAILED
    }
}


# ==============================================================================
# 17. FINAL STATUS & EXIT
# ==============================================================================

Write-Host ""

if ($runnerExitCode -eq $EXIT_SUCCESS) {
    Write-Log "==================================================" Green
    Write-Log "AUTOMATED TEST RUNNER: SUCCESS" Green
    Write-Log "Build:    $buildSucceeded" Green
    Write-Log "Tests:    $testResultStatus" Green
    Write-Log "Archicad: terminated" Green
    Write-Log "Exit:     0" Green
    Write-Log "==================================================" Green
} else {
    Write-Log "==================================================" Red
    Write-Log "AUTOMATED TEST RUNNER: FAILED" Red
    Write-Log "Exit code: $runnerExitCode" Red
    Write-Log "Build:    $buildSucceeded" Red
    Write-Log "Tests:    $testResultStatus" Red

    if (@(Get-ArchicadProcesses).Count -eq 0) {
        Write-Log "Archicad: terminated" Red
    } else {
        Write-Log "Archicad: STILL RUNNING" Red
    }
    Write-Log "==================================================" Red
}

exit $runnerExitCode