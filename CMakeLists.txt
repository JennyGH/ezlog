set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 编译工程所需最低CMake版本
cmake_minimum_required(VERSION 3.20)

project(ezlog VERSION 2.0.0)

include(CheckCXXCompilerFlag)

# 输出工程相关信息
message(STATUS "PROJECT INFO:")
message(STATUS "    CMAKE_PROJECT_NAME:    ${CMAKE_PROJECT_NAME}")
message(STATUS "    CMAKE_BUILD_TYPE:      ${CMAKE_BUILD_TYPE}")
message(STATUS "    CMAKE_SOURCE_DIR:      ${CMAKE_SOURCE_DIR}")
message(STATUS "    CMAKE_PROJECT_VERSION: ${CMAKE_PROJECT_VERSION}")

# 输出系统相关信息
message(STATUS "SYSTEM INFO:")
message(STATUS "    CMAKE_SYSTEM_NAME:              ${CMAKE_SYSTEM_NAME}")
message(STATUS "    CMAKE_SYSTEM_PROCESSOR:         ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "    CMAKE_SIZEOF_VOID_P:            ${CMAKE_SIZEOF_VOID_P}")
message(STATUS "    CMAKE_C_COMPILER_ID:            ${CMAKE_C_COMPILER_ID}")
message(STATUS "    CMAKE_EXECUTABLE_SUFFIX:        ${CMAKE_EXECUTABLE_SUFFIX}")
message(STATUS "    CMAKE_SHARED_LIBRARY_PREFIX:    ${CMAKE_SHARED_LIBRARY_PREFIX}")
message(STATUS "    CMAKE_SHARED_LIBRARY_SUFFIX:    ${CMAKE_SHARED_LIBRARY_SUFFIX}")
message(STATUS "    CMAKE_STATIC_LIBRARY_PREFIX:    ${CMAKE_STATIC_LIBRARY_PREFIX}")
message(STATUS "    CMAKE_STATIC_LIBRARY_SUFFIX:    ${CMAKE_STATIC_LIBRARY_SUFFIX}")
message(STATUS "    MSVC:                           ${MSVC}")
message(STATUS "    MINGW:                          ${MINGW}")
message(STATUS "    WIN32:                          ${WIN32}")
message(STATUS "    UNIX:                           ${UNIX}")
message(STATUS "    APPLE:                          ${APPLE}")
message(STATUS "    IOS:                            ${IOS}")
message(STATUS "    ANDROID:                        ${ANDROID}")
message(STATUS "    MSVC_VERSION:                   ${MSVC_VERSION}")

# 自定义选项
OPTION(BUILD_SHARED_LIBS "Build shared libraries." ON)
OPTION(ENABLE_STATIC_CXX_RUNTIME "Link C++ static library." ON)
OPTION(BUILD_TEST "Build test." ON)
OPTION(BUILD_EXAMPLE "Build example." ON)
OPTION(BUILD_BENCHMARK "Build benchmark." ON)

# 检查当前编译器是否64位
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(CMAKE_SYSTEM_ADDRESS_MODE 64)
    set(x86_64                    TRUE)
else()
    set(CMAKE_SYSTEM_ADDRESS_MODE 32)
    set(x86_64                    FALSE)
endif() # endif x64
message(STATUS "    CMAKE_SYSTEM_ADDRESS_MODE:      ${CMAKE_SYSTEM_ADDRESS_MODE}")
message(STATUS "    x86_64:                         ${x86_64}")

# 设置编译输出目录，带 DEBUG/RELEASE 后缀的为对应编译 Debug/Release 版本时的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY           "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY           "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY           "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG     "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE   "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG     "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE   "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG     "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE   "${CMAKE_BINARY_DIR}/bin")

# 设置静态代码检查命令
set(CPPCHECK_COMMAND cppcheck -j 4 --quiet --enable=warning,performance,portability --library=std --std=c++11)
if(MSVC)
    set(CPPCHECK_COMMAND ${CPPCHECK_COMMAND} --template=vs)
endif()

# 设置默认安装目录， CMAKE_INSTALL_PREFIX 在 cmake 命令行参数中被指定
set(CMAKE_INSTALL_BINDIR        bin)     # 安装运行时文件，例如 .exe, .dll
set(CMAKE_INSTALL_INCLUDEDIR    include) # 安装头文件
set(CMAKE_INSTALL_LIBDIR        lib64)   # 安装库文件，例如 .lib, .so, .a
if(NOT EXISTS ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR} OR NOT x86_64)
    set(CMAKE_INSTALL_LIBDIR    lib)
endif() # endif x86_64

# 用于判断平台类型的宏定义
add_compile_definitions("$<$<BOOL:${MSVC}>:MSVC>")
add_compile_definitions("$<$<BOOL:${MINGW}>:MINGW>")
add_compile_definitions("$<$<BOOL:${UNIX}>:UNIX>")
add_compile_definitions("$<$<BOOL:${WIN32}>:WIN32>")
add_compile_definitions("$<$<BOOL:${APPLE}>:OSX>")
add_compile_definitions("$<$<BOOL:${APPLE}>:APPLE>")
add_compile_definitions("$<$<BOOL:${IOS}>:iOS>")
add_compile_definitions("$<$<BOOL:${ANDROID}>:ANDROID>")

# 用于区分 DEBUG/RELEASE 的宏定义
add_compile_definitions("$<$<CONFIG:Debug>:DEBUG=1>")
add_compile_definitions("$<$<CONFIG:Debug>:_DEBUG=1>")
add_compile_definitions("$<$<CONFIG:Release>:DEBUG=0>")
add_compile_definitions("$<$<CONFIG:Release>:_DEBUG=0>")

# 用于区分 32/64 位的宏定义
add_compile_definitions("$<$<BOOL:${x86_64}>:X64>")
add_compile_definitions("$<$<BOOL:${x86_64}>:X86_64>")

# 针对 MSVC 特有的宏定义
add_compile_definitions("$<$<BOOL:${MSVC}>:WINVER=0x0501>")
add_compile_definitions("$<$<BOOL:${MSVC}>:_MSC_VER=${MSVC_VERSION}>")
add_compile_definitions("$<$<BOOL:${MSVC}>:_WIN32_WINNT=0x0501>")

# 检查是否支持C++11
if(MSVC)
    set(COMPILER_SUPPORTS_CXX11 $<VERSION_GREATER:${MSVC_VERSION},1600>)
else()
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
endif() # endif MSVC
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    add_definitions(-DENABLE_CXX11)
endif() # endif COMPILER_SUPPORTS_CXX11

# 设置编译时选项.
if(MSVC)
    check_cxx_compiler_flag("/MP"    COMPILER_SUPPORTS_FLAG_MP)
    check_cxx_compiler_flag("/EHa"   COMPILER_SUPPORTS_FLAG_EHA)
    check_cxx_compiler_flag("/utf-8" COMPILER_SUPPORTS_FLAG_UTF8)
    add_compile_options(
        "$<$<BOOL:${COMPILER_SUPPORTS_FLAG_MP}>:/MP>"
        "$<$<BOOL:${COMPILER_SUPPORTS_FLAG_EHA}>:/EHa>"
        "$<$<BOOL:${COMPILER_SUPPORTS_FLAG_UTF8}>:/utf-8>"
    )
    if(ENABLE_STATIC_CXX_RUNTIME)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # 设置在 MSVC 中编译时使用 /MT 与 /MTd 模式，注释后使用 /MD 与 /MDd 模式
    endif() # endif ENABLE_STATIC_CXX_RUNTIME
else()
    check_cxx_compiler_flag("-s"              COMPILER_SUPPORTS_FLAG_STRIP)
    check_cxx_compiler_flag("-frtti"          COMPILER_SUPPORTS_FLAG_RTTI)
    check_cxx_compiler_flag("-frtti"          COMPILER_SUPPORTS_FLAG_RTTI)
    check_cxx_compiler_flag("-static-libgcc"  COMPILER_SUPPORTS_FLAG_STATIC_LIBGCC)
    add_compile_options(
        -fPIC
        -Wno-unused-but-set-variable
        -Wfatal-errors
        -Wextra
        "$<$<BOOL:${ENABLE_STATIC_CXX_RUNTIME}>:-static-libstdc++>"
        "$<$<AND:$<BOOL:${ENABLE_STATIC_CXX_RUNTIME}>,$<BOOL:${COMPILER_SUPPORTS_FLAG_STATIC_LIBGCC}>>:-static-libgcc>"
        "$<$<BOOL:${COMPILER_SUPPORTS_FLAG_RTTI}>:-frtti>"
        "$<$<BOOL:${COMPILER_SUPPORTS_FLAG_EXCEPTIONS}>:-fexceptions>"
        "$<$<CONFIG:Debug>:-g>"
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Debug>:-Wall>"
        "$<$<CONFIG:Debug>:-ggdb>"
        "$<$<AND:$<CONFIG:Release>,${COMPILER_SUPPORTS_FLAG_STRIP}>:-s>"
    )
endif() # endif MSVC

# 添加一些通自定义的宏定义
add_definitions(
    -DUNICODE  # 设置工程使用 UNICODE 字符集
    -D_UNICODE # 设置工程使用 UNICODE 字符集
    -DPROJECT_ROOT="${CMAKE_SOURCE_DIR}" # 用于获取当前工程根目录的宏
)

# 整个工程需要包含的库目录.
include_directories(
)

# 整个工程寻找链接库的目录.
link_directories(
)

# 添加项目目录.
add_subdirectory(src)

if(BUILD_TEST)
    add_subdirectory(test)
endif() # endif BUILD_TEST

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif() # endif BUILD_EXAMPLE

if(BUILD_BENCHMARK)
    find_package(benchmark CONFIG)
    if(benchmark_FOUND)
        add_subdirectory(benchmark)
    endif() # endif benchmark_FOUND
endif() # endif BUILD_BENCHMARK

# 往 CMakePredefinedTargets 中追加一个重新加载工程的指令
add_custom_target(RELOAD COMMAND ${CMAKE_SOURCE_DIR}/dev.bat SOURCES ${CMAKE_SOURCE_DIR}/dev.bat)
set_target_properties(RELOAD PROPERTIES FOLDER "CMakePredefinedTargets")
set_target_properties(${PRJ_TARGET} PROPERTIES FOLDER "CMakePredefinedTargets")

if(DEFINED PACK_CONFIG_FILE AND EXISTS ${PACK_CONFIG_FILE})
    include(${PACK_CONFIG_FILE})
endif() # endif PACK_CONFIG_FILE