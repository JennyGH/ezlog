@ECHO OFF
SETLOCAL

REM =============== Set some local variables here. ===============
REM You can change the name of build directory here:
SET BUILD_DIR_NAME=build-win32

REM Specify build type:
SET BUILD_TYPE=Release

REM Specify install dir, binary files will be installed to here:
SET INSTALL_DIR="%CD%/built/win32"

REM =================== Try to make directory. ===================
IF NOT EXIST %BUILD_DIR_NAME% (
    MKDIR %BUILD_DIR_NAME%
)
REM ==============================================================
CD %BUILD_DIR_NAME%

REM Now, you can build your project.
CALL cmake -G "NMake Makefiles"                 ^
           -DCMAKE_BUILD_TYPE=%BUILD_TYPE%      ^
           -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR% ^
           ..
            
CALL cmake --build . --config %BUILD_TYPE% --target INSTALL

CD ..
ENDLOCAL