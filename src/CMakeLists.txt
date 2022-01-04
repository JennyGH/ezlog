project(ezlog VERSION 2.0.0 DESCRIPTION "ezlog")

if(MSVC)
    set(VERSIONINFO_RC ${PROJECT_BINARY_DIR}/VersionInfo.rc)
    configure_file(${PROJECT_SOURCE_DIR}/VersionInfo.rc.in ${VERSIONINFO_RC})
endif() # ENDIF MSVC

file(GLOB_RECURSE EXPORTS_FILES
    ${PROJECT_SOURCE_DIR}/*.def
    ${PROJECT_SOURCE_DIR}/*.map
)

file(GLOB_RECURSE PUBLIC_INCLUDE_FILES
    ${CMAKE_SOURCE_DIR}/include/*.h
)

file(GLOB_RECURSE INCLUDE_FILES
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/*.hpp
    ${PUBLIC_INCLUDE_FILES}
)

file(GLOB_RECURSE SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/*.c
    ${PROJECT_SOURCE_DIR}/*.cpp
)

source_group("Header Files" FILES "${INCLUDE_FILES}")
source_group("Source Files" FILES "${SOURCE_FILES}")

set(BUILD_TYPE STATIC)
if(BUILD_SHARED_LIBS)
    set(BUILD_TYPE SHARED)
endif() # BUILD_SHARED_LIBS

add_library(
    ${PROJECT_NAME}
    ${BUILD_TYPE}
    ${INCLUDE_FILES}
    ${SOURCE_FILES}
    ${EXPORTS_FILES}  # Add export file of C APIs.
    ${VERSIONINFO_RC} # Add version info.
)

if(CMAKE_CXX_CPPCHECK)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        PRE_BUILD
        COMMAND ${CPPCHECK_COMMAND} ${PROJECT_SOURCE_DIR}
    )
endif() # ENDIF `cppcheck` was found.

if(UNIX AND NOT ANDROID)
    set(LINK_LIBS pthread)
endif() # ENDIF UNIX && !ANDROID

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ${LINK_LIBS}
)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    FILES ${PUBLIC_INCLUDE_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
if(UNIX AND BUILD_SHARED_LIBS)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,--version-script=${PROJECT_SOURCE_DIR}/exports.map")
endif() # ENDIF UNIX && BUILD_SHARED_LIBS