@echo off
setlocal

set "RUNTIME_DIR=%~dp0"
set "ACTIVE_FILE=%RUNTIME_DIR%fractal_ui_active.txt"
if not exist "%ACTIVE_FILE%" (
  echo Missing active runtime metadata: %ACTIVE_FILE%
  exit /b 1
)

set /p FRACTAL_UI_ACTIVE=<"%ACTIVE_FILE%"
if not defined FRACTAL_UI_ACTIVE (
  echo Active runtime metadata is empty: %ACTIVE_FILE%
  exit /b 1
)

set "TARGET_PATH=%RUNTIME_DIR%%FRACTAL_UI_ACTIVE%"
if not exist "%TARGET_PATH%" (
  echo Active runtime is missing: %TARGET_PATH%
  exit /b 1
)

if "%~1"=="" (
  start "" "%TARGET_PATH%"
  exit /b 0
)

"%TARGET_PATH%" %*
exit /b %ERRORLEVEL%