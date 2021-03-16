@ECHO OFF
SETLOCAL
SET EXE="%CD%\built\win32\ezlog_test.exe"
SET /A BUFF_SIZE=1024*1024
SET /A ROLL_SIZE=1024*1024*10
SET OUTPUT_DIR=%CD%\test_logs

IF NOT EXIST "%OUTPUT_DIR%" (
    MKDIR "%OUTPUT_DIR%"
)

CALL .\build-win32.bat

DEL /f /s /q "%CD%\test_report_win32.txt"

FOR /L %%i IN (0,1,1) DO (
    FOR /L %%j IN (0,1,1) DO (
        CALL :do_test %BUFF_SIZE% %%i %ROLL_SIZE% %%j
    )
)
GOTO:EOF
ENDLOCAL

:do_test
    DEL /f /s /q "%OUTPUT_DIR%\*.log"
    SET /A _BUFF_SIZE=%~1 * %~2
    SET /A _ROLL_SIZE=%~3 * %~4
    %EXE% -output_dir="%OUTPUT_DIR%" -thread_count=8 -async_buffer_size=%_BUFF_SIZE% -roll_size=%_ROLL_SIZE% >> test_report_win32.txt
GOTO:EOF
