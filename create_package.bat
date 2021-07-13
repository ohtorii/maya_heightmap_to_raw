@echo off
REM パッケージファイルを作成する
set TARGET_DIR=%~dp0package\
set SCRIPT_DIR=%TARGET_DIR%scripts\
set PLUGINS_DIR=%TARGET_DIR%plug-ins\

call :MakeDir
if "%errorlevel%" neq "0" (
	exit /b 1
)

call :Copy
if "%errorlevel%" neq "0" (
	exit /b 1
)

pause
exit /b 0


:MakeDir
rd /Q /S "%TARGET_DIR%"
if exist "%TARGET_DIR%" (
	echo パッケージのディレクトリを削除できませんでした。
	echo ディレクトリ=%TARGET_DIR%
	exit /b 1
)

md "%TARGET_DIR%"
if not exist "%TARGET_DIR%" (
	echo パッケージのディレクトリを作成できませんでした。
	echo ディレクトリ=%TARGET_DIR%
	exit /b 1
)
md "%SCRIPT_DIR%"
if not exist "%SCRIPT_DIR%" (
	echo パッケージのディレクトリを作成できませんでした。
	echo ディレクトリ=%SCRIPT_DIR%
	exit /b 1
)

md "%PLUGINS_DIR%"
if not exist "%PLUGINS_DIR%" (
	echo パッケージのディレクトリを作成できませんでした。
	echo ディレクトリ=%PLUGINS_DIR%
	exit /b 1
)
exit /b 0


:Copy
REM copy /Y "%~dp0build2019\Release\HeightMapToRaw.mll" "%PLUGINS_DIR%"
copy /Y "%~dp0maya2018project\HeightMapToRaw\Release\HeightMapToRaw.mll" "%PLUGINS_DIR%"
copy /Y "%~dp0gui\*.*" "%SCRIPT_DIR%"

exit /b 0
