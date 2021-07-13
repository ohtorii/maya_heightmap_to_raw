@echo off
REM �p�b�P�[�W�t�@�C�����쐬����
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
	echo �p�b�P�[�W�̃f�B���N�g�����폜�ł��܂���ł����B
	echo �f�B���N�g��=%TARGET_DIR%
	exit /b 1
)

md "%TARGET_DIR%"
if not exist "%TARGET_DIR%" (
	echo �p�b�P�[�W�̃f�B���N�g�����쐬�ł��܂���ł����B
	echo �f�B���N�g��=%TARGET_DIR%
	exit /b 1
)
md "%SCRIPT_DIR%"
if not exist "%SCRIPT_DIR%" (
	echo �p�b�P�[�W�̃f�B���N�g�����쐬�ł��܂���ł����B
	echo �f�B���N�g��=%SCRIPT_DIR%
	exit /b 1
)

md "%PLUGINS_DIR%"
if not exist "%PLUGINS_DIR%" (
	echo �p�b�P�[�W�̃f�B���N�g�����쐬�ł��܂���ł����B
	echo �f�B���N�g��=%PLUGINS_DIR%
	exit /b 1
)
exit /b 0


:Copy
REM copy /Y "%~dp0build2019\Release\HeightMapToRaw.mll" "%PLUGINS_DIR%"
copy /Y "%~dp0maya2018project\HeightMapToRaw\Release\HeightMapToRaw.mll" "%PLUGINS_DIR%"
copy /Y "%~dp0gui\*.*" "%SCRIPT_DIR%"

exit /b 0
