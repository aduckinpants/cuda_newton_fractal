@echo off

set "VSDEVCMD="
if defined VSDEVCMD_BAT if exist "%VSDEVCMD_BAT%" set "VSDEVCMD=%VSDEVCMD_BAT%"

if not defined VSDEVCMD (
  set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
  if exist "%VSWHERE%" (
    for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find Common7\Tools\VsDevCmd.bat`) do (
      set "VSDEVCMD=%%I"
      goto :found_vsdevcmd
    )
  )
)

for %%I in (
  "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat"
  "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
  "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
) do (
  if not defined VSDEVCMD if exist %%~I set "VSDEVCMD=%%~I"
)

:found_vsdevcmd
if not defined VSDEVCMD (
  echo Failed to locate VsDevCmd.bat with an installed MSVC toolchain.
  exit /b 1
)

echo [call_vsdevcmd] Using "%VSDEVCMD%"
call "%VSDEVCMD%" -arch=amd64 -host_arch=amd64
if errorlevel 1 exit /b 1

where cl >NUL 2>NUL
if errorlevel 1 (
  echo cl.exe not found after VsDevCmd
  exit /b 1
)

exit /b 0
