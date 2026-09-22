cd ..
"c:\program files\python310\python.exe" Tools/BuildAddOn.py --configFile config.json --acVersion 25
if errorlevel 1 (
    echo BUILD FAILED, skipping test runner
    exit /b %errorlevel%
)
powershell -ExecutionPolicy Bypass -File "Tools\restart_archicad_for_test.ps1"
exit /b %errorlevel%