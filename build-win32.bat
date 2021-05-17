@ECHO OFF
SETLOCAL

REM =============== Set some local variables here. ===============
REM You can change the name of build directory here:
SET TARGET_ARCH=%1
IF "%TARGET_ARCH%" == "" (
    SET TARGET_ARCH=x86
)
@ECHO Target arch: %TARGET_ARCH%
SET BUILD_DIR_NAME=build-win-%TARGET_ARCH%
CALL:Build Debug
CALL:Build Release
GOTO:EOF

:Build
SET PROJECT_ROOT=%CD%

REM Specify build type:
SET BUILD_TYPE=%1

REM Specify install dir, binary files will be installed to here:
SET INSTALL_DIR="%PROJECT_ROOT%/built/windows/%TARGET_ARCH%/%BUILD_TYPE%"

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
           -DCMAKE_VERBOSE_MAKEFILE=FALSE       ^
           -DBUILD_SHARED=FALSE                 ^
           ..
            
CALL cmake --build . --config %BUILD_TYPE% --target INSTALL

CD %PROJECT_ROOT%

GOTO:EOF

ENDLOCAL