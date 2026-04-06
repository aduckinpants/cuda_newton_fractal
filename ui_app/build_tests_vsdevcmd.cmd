@echo off
setlocal

call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64
if errorlevel 1 exit /b 1

where cl >NUL 2>NUL
if errorlevel 1 (
  echo cl.exe not found after VsDevCmd
  exit /b 1
)

cd /d C:\code\cuda_newton_fractal_clone\ui_app

if "%SALT_FRACTAL_ROOT%"=="" set SALT_FRACTAL_ROOT=D:\salt-fractal
set TESTROOT=%SALT_FRACTAL_ROOT%\cuda_newton_fractal_clone\build_tests

if not exist "%TESTROOT%" mkdir "%TESTROOT%"

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\tests\test_explaino_seed.cpp ^
  /Fe:"%TESTROOT%\test_explaino_seed.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\explaino_seed.cpp .\src\explaino_seed_dynamics.cpp .\tests\test_explaino_seed_dynamics.cpp ^
  /Fe:"%TESTROOT%\test_explaino_seed_dynamics.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\tests\test_fractal_derived_fields.cpp ^
  /Fe:"%TESTROOT%\test_fractal_derived_fields.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\runtime_reset.cpp .\tests\test_runtime_reset.cpp ^
  /Fe:"%TESTROOT%\test_runtime_reset.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\lens_sdf.cpp .\tests\test_lens_sdf.cpp ^
  /Fe:"%TESTROOT%\test_lens_sdf.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\sweep_player.cpp .\tests\test_sweep_player.cpp ^
  /Fe:"%TESTROOT%\test_sweep_player.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\view_hp_sync.cpp .\tests\test_view_hp_sync.cpp ^
  /Fe:"%TESTROOT%\test_view_hp_sync.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\explaino_seed.cpp .\src\diagnostics_state_io.cpp .\tests\test_diagnostics_state_io.cpp ^
  /Fe:"%TESTROOT%\test_diagnostics_state_io.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\diagnostics_state_io.cpp .\src\view_hp_sync.cpp .\src\explaino_seed.cpp .\src\fractal_derived_fields.cpp .\src\finding_state_actions.cpp .\tests\test_finding_state_actions.cpp ^
  /Fe:"%TESTROOT%\test_finding_state_actions.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\schema_startup_policy.cpp .\tests\test_schema_startup_policy.cpp ^
  /Fe:"%TESTROOT%\test_schema_startup_policy.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\finding_archive_actions.cpp .\src\diagnostics_capture.cpp .\tests\test_finding_archive_actions.cpp ^
  /Fe:"%TESTROOT%\test_finding_archive_actions.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\tests\test_fractal_family_rules.cpp ^
  /Fe:"%TESTROOT%\test_fractal_family_rules.exe"
if errorlevel 1 exit /b 1

cl /nologo /EHsc /MD /std:c++17 /O2 /I. /I.\src ^
  .\src\json_min.cpp .\src\ui_schema.cpp .\tests\test_ui_schema.cpp ^
  /Fe:"%TESTROOT%\test_ui_schema.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_seed.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_explaino_seed_dynamics.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_derived_fields.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_runtime_reset.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_lens_sdf.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_sweep_player.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_view_hp_sync.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_diagnostics_state_io.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_finding_state_actions.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_schema_startup_policy.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_finding_archive_actions.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_fractal_family_rules.exe"
if errorlevel 1 exit /b 1

"%TESTROOT%\test_ui_schema.exe"
if errorlevel 1 exit /b 1

echo All helper tests passed.
exit /b 0