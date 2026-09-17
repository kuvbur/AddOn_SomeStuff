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
#   8. Оставить Archicad открытым для работы/отладки.
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
$EXIT_HTML_VALIDATION_FAILED   = 75


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


function Write-AIStatus {
    param (
        [Parameter(Mandatory = $true)]
        [string]$State,

        [Parameter(Mandatory = $true)]
        [string]$Message,

        [ConsoleColor]$Color = [ConsoleColor]::Gray
    )

    Write-Log "AI_STATUS [$State] $Message" $Color
}


function Set-RunnerFailureReason {
    param (
        [Parameter(Mandatory = $true)]
        [string]$Reason
    )

    $script:runnerFailureReason = $Reason
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


function Test-ArchicadProcessIsTestProject {
    param (
        [Parameter(Mandatory = $true)]
        [System.Diagnostics.Process]$Process
    )

    return ($Process.MainWindowTitle -match "(?i)test")
}


function Test-ArchicadProcessesAreTestProjects {
    param (
        [Parameter(Mandatory = $true)]
        [System.Diagnostics.Process[]]$Processes
    )

    foreach ($process in $Processes) {
        Write-Log "Candidate Archicad process: PID=$($process.Id), Name=$($process.ProcessName), Window='$($process.MainWindowTitle)'" Yellow

        if (-not (Test-ArchicadProcessIsTestProject -Process $process)) {
            Set-RunnerFailureReason "non_test_archicad_process"
            Write-AIStatus "SAFETY_BLOCK" "reason=non_test_archicad_process action=do_not_close_process pid=$($process.Id) name=$($process.ProcessName) window='$($process.MainWindowTitle)'" Red
            Write-Log "Attempt to stop a non-test Archicad process. Do not keep trying; report that Archicad cannot be closed because a working project, not a test project, is open." Red
            Write-Log "Refusing to stop PID=$($process.Id), Name=$($process.ProcessName), Window='$($process.MainWindowTitle)'" Red
            return $false
        }
    }

    return $true
}


function Test-SingleArchicadProcess {
    param (
        [Parameter(Mandatory = $true)]
        [System.Diagnostics.Process[]]$Processes
    )

    if ($Processes.Count -le 1) {
        return $true
    }

    Write-Log "Multiple Archicad processes detected. Refusing to stop any process automatically." Red
    Set-RunnerFailureReason "multiple_archicad_processes"
    Write-AIStatus "SAFETY_BLOCK" "reason=multiple_archicad_processes action=do_not_close_any_process count=$($Processes.Count)" Red
    foreach ($process in $Processes) {
        Write-Log "Detected Archicad process: PID=$($process.Id), Name=$($process.ProcessName), Window='$($process.MainWindowTitle)'" Red
    }

    return $false
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
    Write-AIStatus "CLOSING_ARCHICAD" "action=close_tracked_processes reason='$Reason' count=$($ProcessIds.Count)" Yellow

    # 8.1. Graceful shutdown
    foreach ($processId in $ProcessIds) {
        $process = Get-Process -Id $processId -ErrorAction SilentlyContinue
        if (-not $process) { continue }
        Write-Log "Stopping PID=$($process.Id), Name=$($process.ProcessName), Window='$($process.MainWindowTitle)'" Yellow
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
        Set-RunnerFailureReason "archicad_shutdown_failed"
        Write-AIStatus "FAILED" "reason=archicad_shutdown_failed action=manual_close_required alive_count=$($alive.Count)" Red
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
        Set-RunnerFailureReason "automatic_archicad_termination_disabled"
        Write-AIStatus "SAFETY_BLOCK" "reason=automatic_archicad_termination_disabled action=manual_close_required" Red
        Write-Log "Existing Archicad detected, but automatic termination is disabled." Red
        return $false
    }

    $ids = @($processes | Select-Object -ExpandProperty Id)
    Write-Log "Found $($ids.Count) existing Archicad process(es)." Yellow
    Write-AIStatus "CHECK_EXISTING_ARCHICAD" "found=$($ids.Count) action=validate_before_closing" Yellow

    if (-not (Test-SingleArchicadProcess -Processes $processes)) {
        return $false
    }

    if (-not (Test-ArchicadProcessesAreTestProjects -Processes $processes)) {
        return $false
    }

    if (-not (Stop-TrackedArchicad -ProcessIds $ids -Reason "pre-test cleanup")) {
        return $false
    }

    Start-Sleep -Seconds 1
    $remaining = @(Get-ArchicadProcesses)

    if ($remaining.Count -gt 0) {
        Write-Log "Additional Archicad process(es) detected after shutdown." Yellow
        $remainingIds = @($remaining | Select-Object -ExpandProperty Id)

        if (-not (Test-SingleArchicadProcess -Processes $remaining)) {
            return $false
        }

        if (-not (Test-ArchicadProcessesAreTestProjects -Processes $remaining)) {
            return $false
        }

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
# 12. HTML ВАЛИДАЦИЯ (запускается ДО билда)
# ==============================================================================

function Test-HtmlValidation {
    param (
        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot
    )

    $htmlPath = Join-Path $ProjectRoot "Sources\AddOnResources\RFIX\HTML\Interface_ru.html"
    $verifyScript = Join-Path $ProjectRoot "Tools\verify.js"
    $packageJson = Join-Path $ProjectRoot "package.json"

    Write-Log "=== HTML VALIDATION START ===" Cyan

    # Проверяем наличие файлов
    if (-not (Test-Path -LiteralPath $htmlPath)) {
        Write-Log "HTML file not found: $htmlPath" Red
        return $false
    }

    if (-not (Test-Path -LiteralPath $verifyScript)) {
        Write-Log "verify.js not found: $verifyScript" Red
        return $false
    }

    if (-not (Test-Path -LiteralPath $packageJson)) {
        Write-Log "package.json not found: $packageJson" Red
        return $false
    }

    # Проверяем наличие node_modules (npm install)
    $nodeModules = Join-Path $ProjectRoot "node_modules"
    if (-not (Test-Path -LiteralPath $nodeModules)) {
        Write-Log "node_modules not found, running npm install..." Yellow
        try {
            Set-Location -LiteralPath $ProjectRoot
            $npmInstallOutput = @(& npm install 2>&1)
            $npmExitCode = $LASTEXITCODE
            foreach ($line in $npmInstallOutput) { Write-Host $line }
            if ($npmExitCode -ne 0) {
                Write-Log "npm install failed with exit code: $npmExitCode" Red
                return $false
            }
        }
        catch {
            Write-Log "npm install failed to start: $($_.Exception.Message)" Red
            return $false
        }
    }

    # 1. HTMLHint validation
    Write-Log "Running HTMLHint..." Cyan
    try {
        Set-Location -LiteralPath $ProjectRoot
        $htmlHintOutput = @(& npx htmlhint $htmlPath 2>&1)
        $htmlHintExitCode = $LASTEXITCODE
        foreach ($line in $htmlHintOutput) { Write-Host $line }
        if ($htmlHintExitCode -ne 0) {
            Write-Log "HTMLHint validation FAILED" Red
            return $false
        }
        Write-Log "HTMLHint: PASSED" Green
    }
    catch {
        Write-Log "HTMLHint failed to run: $($_.Exception.Message)" Red
        return $false
    }

    # 2. Custom verify.js validation (ТЗ-проверки)
    Write-Log "Running custom verify.js (ТЗ checks)..." Cyan
    try {
        Set-Location -LiteralPath $ProjectRoot
        $verifyOutput = @(& node $verifyScript $htmlPath 2>&1)
        $verifyExitCode = $LASTEXITCODE
        foreach ($line in $verifyOutput) { Write-Host $line }
        if ($verifyExitCode -ne 0) {
            Write-Log "Custom HTML validation FAILED" Red
            return $false
        }
        Write-Log "Custom verify.js: PASSED" Green
    }
    catch {
        Write-Log "verify.js failed to run: $($_.Exception.Message)" Red
        return $false
    }

    Write-Log "=== HTML VALIDATION PASSED ===" Green
    return $true
}


# ==============================================================================
# 13. ПЕРЕМЕННЫЕ RUNNER
# ==============================================================================

$runnerExitCode      = $EXIT_SUCCESS
$archicadStarted     = $false
$trackedArchicadPids = @()
$testResultStatus    = "NOT_RUN"
$buildSucceeded      = $false
$runnerFailureReason = "none"


# ==============================================================================
# 14. MAIN
# ==============================================================================

try {
    Write-AIStatus "START" "task=restart_archicad_for_test action=validate_environment_then_close_test_archicad_build_launch_and_wait_for_results" Cyan

    # CONFIG
    if (-not (Test-Path -LiteralPath $configPath)) {
        Set-RunnerFailureReason "config_json_not_found"
        throw "config.json not found: $configPath"
    }

    if (-not (Test-Path -LiteralPath $buildScriptPath)) {
        Set-RunnerFailureReason "build_script_not_found"
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
        Set-RunnerFailureReason "archicad_exe_not_found"
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
        Set-RunnerFailureReason "test_pln_not_found"
        throw "Test PLN not found: $filePath"
    }
    Write-Log "Results:       $testResultsPath" Gray
    Write-AIStatus "CONFIG_OK" "ac_version=$acVersion archicad_exe='$archicadExePath' test_file='$filePath' results_file='$testResultsPath'" Green

    # PREVIOUS ARCHICAD
    if (-not (Stop-ExistingArchicad)) {
        $runnerExitCode = $EXIT_PREVIOUS_AC_FAILED
        if ($runnerFailureReason -eq "none") {
            Set-RunnerFailureReason "previous_archicad_not_terminated"
        }
        throw "Unable to guarantee that previous Archicad processes are terminated."
    }

    Start-Sleep -Seconds $postCloseCleanupPauseSec

    # CLEANUP
        Write-AIStatus "CLEANUP" "action=remove_lock_and_old_results lock_file='$lckFilePath' results_file='$testResultsPath'" Cyan
        if (-not (Remove-FileWithRetry -Path $lckFilePath -Retries $fileDeleteRetries)) {
            $runnerExitCode = $EXIT_CLEANUP_FAILED
            Set-RunnerFailureReason "unable_to_remove_archicad_lock_file"
            throw "Unable to remove Archicad lock file."
        }

        if (-not (Remove-FileWithRetry -Path $testResultsPath -Retries $fileDeleteRetries)) {
            $runnerExitCode = $EXIT_CLEANUP_FAILED
            Set-RunnerFailureReason "unable_to_remove_old_test_results"
            throw "Unable to remove old test_results.txt."
        }

        # HTML VALIDATION (ДО БИЛДА)
        Write-AIStatus "HTML_VALIDATION" "action=run_htmlhint_and_custom_verify before_build=true" Cyan
        if (-not (Test-HtmlValidation -ProjectRoot $projectRoot)) {
            $runnerExitCode = $EXIT_HTML_VALIDATION_FAILED
            Set-RunnerFailureReason "html_validation_failed"
            throw "HTML validation failed. Build aborted."
        }

        # ENVIRONMENT
            [Console]::OutputEncoding = [System.Text.Encoding]::UTF8
            $OutputEncoding           = [System.Text.Encoding]::UTF8
            $env:PYTHONIOENCODING     = "utf-8"
            $env:VSLANG               = "1033"

            # BUILD
            $previousLocation = Get-Location
            Write-AIStatus "BUILD" "action=run_build_script ac_version=$acVersion" Cyan

            try {
        Set-Location -LiteralPath $projectRoot
        $buildOutput = @(& python $buildScriptPath --configFile config.json --acVersion $acVersion 2>&1)
        $buildExitCode = $LASTEXITCODE
    }
    catch {
        Set-Location -LiteralPath $previousLocation
        $runnerExitCode = $EXIT_BUILD_FAILED
        Set-RunnerFailureReason "build_process_failed_to_start"
        throw "Build process failed to start: $($_.Exception.Message)"
    }

    Set-Location -LiteralPath $previousLocation

    foreach ($line in $buildOutput) { Write-Host $line }

    if ($buildExitCode -ne 0) {
        $runnerExitCode = $EXIT_BUILD_FAILED
        Set-RunnerFailureReason "build_failed_exit_code_$buildExitCode"
        throw "Build failed. Python exit code: $buildExitCode"
    }

    $buildOutputText = $buildOutput -join "`n"
    if ($buildOutputText -match "Failed to build project") {
        $runnerExitCode = $EXIT_BUILD_FAILED
        Set-RunnerFailureReason "build_script_reported_failure"
        throw "Build script reported 'Failed to build project'."
    }

    $buildSucceeded = $true
    Write-Log "Build completed successfully." Green
    Write-AIStatus "BUILD_OK" "build_succeeded=true output_addon_expected=true" Green

    # ARCHICAD START
    $pidsBefore = @(Get-ArchicadProcessIds)

    # Передаём файл PLN первым аргументом, затем сервисные флаги
    $acArgs = @(
        "`"$filePath`"",
        "-forceaccessdialog",
        "-bringToFront",
        "-DISABLERECOVERYDIALOG"
    )

    Write-AIStatus "LAUNCH_ARCHICAD" "action=start_archicad exe='$archicadExePath' file='$filePath'" Cyan
    $startedProcess = Start-Process -FilePath $archicadExePath -ArgumentList $acArgs -PassThru
    Write-Log "Waiting for Archicad startup (timeout ${archicadLaunchTimeoutSec}s)..." Cyan

    $newProcesses = Wait-ForNewArchicad -PidsBefore $pidsBefore -TimeoutSec $archicadLaunchTimeoutSec

    if ($newProcesses.Count -eq 0) {
        $runnerExitCode = $EXIT_AC_START_FAILED
        Set-RunnerFailureReason "archicad_process_not_detected_after_launch"
        throw "Archicad process was not detected after launch."
    }

    $trackedArchicadPids = @($newProcesses | Select-Object -ExpandProperty Id)
    $archicadStarted     = $true
    Write-AIStatus "ARCHICAD_STARTED" "tracked_pids=$($trackedArchicadPids -join ',') action=wait_for_test_results" Green

    # TEST WAITING
    Write-Log "Waiting for test_results.txt (timeout ${testResultTimeoutSec}s)..." Cyan

    $deadline = (Get-Date).AddSeconds($testResultTimeoutSec)

    while ((Get-Date) -lt $deadline) {
        if (Test-Path -LiteralPath $testResultsPath) {
            Write-Log "test_results.txt detected." Green
            break
        }

        $aliveTracked = @($trackedArchicadPids | Where-Object { Test-ProcessExists -ProcessId $_ })
        if ($aliveTracked.Count -eq 0) {
            $runnerExitCode = $EXIT_RUNTIME_ERROR
            Set-RunnerFailureReason "archicad_terminated_before_test_results"
            throw "Tracked Archicad process terminated unexpectedly before test_results.txt was created."
        }

        Start-Sleep -Seconds $pollIntervalSec
    }

    # TIMEOUT CHECK
    if (-not (Test-Path -LiteralPath $testResultsPath)) {
        $testResultStatus = "TIMEOUT"
        $runnerExitCode   = $EXIT_TEST_TIMEOUT
        Set-RunnerFailureReason "test_results_timeout"
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
            Write-AIStatus "TESTS_OK" "status=PASSED source='$testResultsPath'" Green
        }
        "FAILED" {
            Write-Log "Automated tests: FAILED." Red
            $runnerExitCode = $EXIT_TESTS_FAILED
            Set-RunnerFailureReason "automated_tests_failed"
            Write-AIStatus "TESTS_FAILED" "status=FAILED source='$testResultsPath'" Red
        }
        "UNKNOWN" {
            Write-Log "Test result file exists, but its status is not recognized." Yellow
            Write-AIStatus "TESTS_UNKNOWN" "status=UNKNOWN source='$testResultsPath' action=manual_log_review_recommended" Yellow
        }
        "ERROR" {
            $runnerExitCode = $EXIT_RUNTIME_ERROR
            Set-RunnerFailureReason "unable_to_parse_test_results"
            throw "Unable to parse test_results.txt."
        }
    }
    
    # =================================================================
    # JSON COMMANDS TESTING (после C++ тестов, если они не провалены)
    # =================================================================
    if ($testResultStatus -eq "PASSED" -or $testResultStatus -eq "UNKNOWN") {
        Write-Log "Starting JSON commands testing..." Cyan
        Write-AIStatus "JSON_TESTS" "action=run_optional_json_command_tests if_script_exists=true" Cyan
        
        # Ждём инициализации PropertyCache
        Write-Log "Waiting 15 seconds for PropertyCache initialization..." Yellow
        Start-Sleep -Seconds 15
        
        $jsonTestScript = Join-Path $projectRoot "Tools\test_json_commands.py"
        if (Test-Path -LiteralPath $jsonTestScript) {
            $jsonTestOutput = @(& python $jsonTestScript 2>&1)
            $jsonTestExitCode = $LASTEXITCODE
            
            Write-Host ""
            Write-Host "================ JSON COMMANDS TESTS ================" -ForegroundColor Cyan
            foreach ($line in $jsonTestOutput) { Write-Host $line }
            Write-Host "====================================================" -ForegroundColor Cyan
            Write-Host ""
            
            if ($jsonTestExitCode -eq 0) {
                Write-Log "JSON commands tests: PASSED." Green
                Write-AIStatus "JSON_TESTS_OK" "status=PASSED" Green
            } else {
                Write-Log "JSON commands tests: FAILED (exit code: $jsonTestExitCode)." Red
                $runnerExitCode = $EXIT_TESTS_FAILED
                Set-RunnerFailureReason "json_commands_tests_failed_exit_code_$jsonTestExitCode"
                Write-AIStatus "JSON_TESTS_FAILED" "status=FAILED exit_code=$jsonTestExitCode" Red
            }
        } else {
            Write-Log "JSON test script not found: $jsonTestScript" Yellow
            Write-AIStatus "JSON_TESTS_SKIPPED" "reason=script_not_found path='$jsonTestScript'" Yellow
        }
    }
}
catch {
    Write-Log "RUNNER ERROR: $($_.Exception.Message)" Red
    if ($runnerFailureReason -eq "none") {
        Set-RunnerFailureReason "exception_$($_.Exception.GetType().Name)"
    }
    Write-AIStatus "ERROR" "reason=$runnerFailureReason message='$($_.Exception.Message)'" Red
    if ($runnerExitCode -eq $EXIT_SUCCESS) {
        $runnerExitCode = $EXIT_RUNTIME_ERROR
    }
}


# ==============================================================================
# 17. FINAL STATUS & EXIT
# ==============================================================================

Write-Host ""

$archicadProcessCount = @(Get-ArchicadProcesses).Count
$archicadFinalState = if ($archicadProcessCount -gt 0) { "running" } else { "not_running" }

if ($runnerExitCode -eq $EXIT_SUCCESS) {
    Write-AIStatus "DONE" "reason=completed build=$buildSucceeded tests=$testResultStatus archicad=$archicadFinalState" Green
    Write-Log "AI_RESULT status=success exit_code=0 reason=completed build=$buildSucceeded tests=$testResultStatus archicad=$archicadFinalState" Green
    Write-Log "==================================================" Green
    Write-Log "AUTOMATED TEST RUNNER: SUCCESS" Green
    Write-Log "Build:    $buildSucceeded" Green
    Write-Log "Tests:    $testResultStatus" Green
    Write-Log "Archicad: $archicadFinalState" Green
    Write-Log "Exit:     0" Green
    Write-Log "==================================================" Green
} else {
    Write-AIStatus "FAILED" "reason=$runnerFailureReason build=$buildSucceeded tests=$testResultStatus archicad=$archicadFinalState exit_code=$runnerExitCode" Red
    Write-Log "AI_RESULT status=failed exit_code=$runnerExitCode reason=$runnerFailureReason build=$buildSucceeded tests=$testResultStatus archicad=$archicadFinalState" Red
    Write-Log "==================================================" Red
    Write-Log "AUTOMATED TEST RUNNER: FAILED" Red
    Write-Log "Exit code: $runnerExitCode" Red
    Write-Log "Build:    $buildSucceeded" Red
    Write-Log "Tests:    $testResultStatus" Red
    Write-Log "Archicad: $archicadFinalState" Yellow
    Write-Log "==================================================" Red
}

exit $runnerExitCode
