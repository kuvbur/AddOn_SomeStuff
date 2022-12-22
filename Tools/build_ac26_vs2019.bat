pushd %~dp0
@echo off

set AC_Version=AC26
set Dev_Kit_Name=API Development Kit 26.3000
set VS_Version=Visual Studio 16 2019
set VS_Toolset=v142



set Dev_Kit_Path_WIN="C:/Program Files/GRAPHISOFT/%Dev_Kit_Name%"
set Dev_Kit_Path_Local="../Build/%Dev_Kit_Name%"
set Build_Path="../Build/%AC_Version%"
if not exist %Dev_Kit_Path_Local%\ (
	echo %Dev_Kit_Path_Local% not found
	if exist %Dev_Kit_Path_WIN%\ (
		mkdir %Dev_Kit_Path_Local%
		mkdir %Dev_Kit_Path_Local%/Support
		robocopy %Dev_Kit_Path_WIN%/Support %Dev_Kit_Path_Local%/Support /e /NFL /NDL /NJH /NJS /nc /ns
		echo %Dev_Kit_Path_Local% create
	) else (
		echo ERROR %Dev_Kit_Path_WIN% not found
		goto :error
	)
)
if not exist %Build_Path%\ (
	mkdir %Build_Path%
)
echo on
cmake -B %Build_Path% -G "%VS_Version%" -A "x64" -T "%VS_Toolset%" -DAC_API_DEVKIT_DIR=%Dev_Kit_Path_Local% .. || goto :error
cmake --build %Build_Path% --config Debug || goto :error
popd
exit /b 0
:error
echo Build Failed with Error %errorlevel%.
popd
exit /b 1
