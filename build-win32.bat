@ECHO ON
@SETLOCAL

@REM =============== Set some local variables here. ===============
@REM You can change the name of build directory here:
@SET BUILD_DIR_NAME=build-win32

@REM Specify vc toolkit here, default toolkit is Visual Studio 2008:
@SET TOOLKIT=v90

@REM Specify build type:
@SET BUILD_TYPE=Debug

@REM Specify install dir, binary files will be installed to here:
@SET INSTALL_DIR=%CD%/built/win32

@REM =================== Try to make directory. ===================
@IF NOT EXIST %BUILD_DIR_NAME% (
    @MKDIR %BUILD_DIR_NAME%
)
@REM ==============================================================
@CD %BUILD_DIR_NAME%

@REM Now, you can build your project.
@CALL cmake -T %TOOLKIT% ^
            -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
            ..
            
@CALL cmake --build . --config %BUILD_TYPE%

@REM Install built binary files:
@REM @CALL cmake --build . --config %BUILD_TYPE% --target INSTALL

@CD ..
@ENDLOCAL