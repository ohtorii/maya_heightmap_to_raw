@echo off
REM MAYAプラグインフォルダへコピーする
copy /Y "%~dp0build2019\Release\HeightMapToRaw.mll" "%HOMEDRIVE%%HOMEPATH%\Documents\maya\2019\ja_JP\plug-ins\"
copy /Y "%~dp0maya2018project\HeightMapToRaw\Release\HeightMapToRaw.mll" "%HOMEDRIVE%%HOMEPATH%\Documents\maya\2018\ja_JP\plug-ins\"
pause