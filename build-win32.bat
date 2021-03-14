@ECHO ON
@SETLOCAL

@REM =============== Set some local variables here. ===============
@REM You can change the name of build directory here:
@SET BUILD_DIR_NAME=build-win32

@REM =================== Try to make directory. ===================
@IF NOT EXIST %BUILD_DIR_NAME% (
    @MKDIR %BUILD_DIR_NAME%
)
@REM ==============================================================
@CD %BUILD_DIR_NAME%

@REM Now, you can build your project.
@CALL cmake .. -DCMAKE_BUILD_TYPE=Debug
@CALL cmake --build . --config Debug --target INSTALL

@CD ..
@ENDLOCAL