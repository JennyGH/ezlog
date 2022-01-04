@ECHO OFF
SETLOCAL
SET PROJECT_ROOT=%CD%
CALL:Format "%PROJECT_ROOT%\\src"
CALL:Format "%PROJECT_ROOT%\\test"
CALL:Format "%PROJECT_ROOT%\\example"
CALL:Format "%PROJECT_ROOT%\\benchmark"

@CALL python %PROJECT_ROOT%\pyscripts\convert-to-utf8.py

GOTO:EOF

:Format
SET SOURCE_DIR=%1
FOR /r %SOURCE_DIR% %%i IN (*.h) DO (
    @CALL clang-format -i "%%i" --style=file --verbose
)
FOR /r %SOURCE_DIR% %%i IN (*.c) DO (
    @CALL clang-format -i "%%i" --style=file --verbose
)
FOR /r %SOURCE_DIR% %%i IN (*.cpp) DO (
    @CALL clang-format -i "%%i" --style=file --verbose
)
GOTO:EOF

ENDLOCAL