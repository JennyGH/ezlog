project(ezlog_benchmark_test)

file(GLOB_RECURSE INCLUDE_FILES
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.hpp
)

file(GLOB_RECURSE SOURCE_FILES 
    ${PROJECT_SOURCE_DIR}/*.c
    ${PROJECT_SOURCE_DIR}/*.cpp
)
    
source_group("Header Files" FILES "${INCLUDE_FILES}")
source_group("Source Files" FILES "${SOURCE_FILES}")

add_executable(
    ${PROJECT_NAME}
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
)

set(EX_LINK_LIBRARIES )
if(MSVC)
    LIST(APPEND EX_LINK_LIBRARIES shlwapi)
else()
    LIST(APPEND EX_LINK_LIBRARIES pthread)
endif() # ENDIF MSVC

if(benchmark_FOUND)
    LIST(APPEND EX_LINK_LIBRARIES benchmark::benchmark)
endif() # ENDIF benchmark_FOUND

target_link_libraries(
    ${PROJECT_NAME}
    ${EX_LINK_LIBRARIES}
    ezlog
)

install(
    TARGETS ezlog_benchmark_test
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)