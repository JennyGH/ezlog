@ECHO OFF
SETLOCAL
@REM ===========================================
@REM 这是用于开发时产生工程使用的脚本
@REM ===========================================

@REM 构建工程的目录名，.sln 会生成在这里面
SET BUILD_DIR_NAME=dev

@REM 工程根目录
SET PROJECT_ROOT=%CD%

@REM 构建工程的路径
SET BUILD_DIR=%PROJECT_ROOT%\%BUILD_DIR_NAME%

IF NOT EXIST %BUILD_DIR% (
    MKDIR %BUILD_DIR%
)
CD /D %BUILD_DIR%

@REM 开始构建工程
CALL cmake ..                                     ^
           -DCMAKE_BUILD_TYPE=DEBUG               ^
           -DCMAKE_CXX_CPPCHECK:FILEPATH=cppcheck ^

ENDLOCAL