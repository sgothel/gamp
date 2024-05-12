cmake_minimum_required (VERSION 3.21.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
# set(CMAKE_CXX_STANDARD 17 CACHE STRING "" FORCE)
set(USE_LIBCURL OFF CACHE BOOL "" FORCE)
set(BUILDJAVA OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(DONT_USE_RTTI ON CACHE BOOL "" FORCE)
if (EMSCRIPTEN)
  set(USE_LIBUNWIND OFF CACHE BOOL "" FORCE)
endif()
include(jaulib/JaulibSetup.cmake)
JaulibPreset()

project (gamp LANGUAGES C CXX)

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

if(NOT DEFINED DONT_USE_RTTI)
    set(DONT_USE_RTTI OFF)
    message(STATUS "${PROJECT_NAME} DONT_USE_RTTI ${DONT_USE_RTTI} (gamp default)")
else()
    message(STATUS "${PROJECT_NAME} DONT_USE_RTTI ${DONT_USE_RTTI} (user)")
endif()

include(WasmSetup.cmake)
JaulibSetup()

if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.clangd)
# Fixup clangd w/ absolute path
set (gamp_clangd_source_args "-isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/include, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/src, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/java_jni/jni, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/java_jni/jni/jau, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/test")
set (gamp_clangd_source_args "${gamp_clangd_source_args} -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/include, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/src, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/test")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/clangd.in
                ${CMAKE_CURRENT_SOURCE_DIR}/.clangd)
endif()

# this is the library version, derived from git tag and revision
set (gamp_VERSION_MAJOR      ${VERSION_MAJOR})
set (gamp_VERSION_MINOR      ${VERSION_MINOR})
set (gamp_VERSION_PATCH      ${VERSION_PATCH})
set (gamp_VERSION_COMMITS    ${VERSION_COMMITS})
set (gamp_VERSION_SHA1       ${VERSION_SHA1})
set (gamp_VERSION_SHA1_SHORT ${VERSION_SHA1_SHORT})
set (gamp_GIT_DIRTY          ${GIT_WORKDIR_DIRTY})
set (gamp_VERSION_LONG       ${VERSION_LONG})
set (gamp_VERSION_SHORT      ${VERSION_SHORT})
set (gamp_VERSION_API        ${VERSION_API})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/version.cpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/src/version.cpp)

WasmSetup()

find_path (SYSTEM_USR_DIR "stdlib.h")
include_directories (${SYSTEM_USR_DIR})

message(STATUS "${PROJECT_NAME} SYSTEM_USR_DIR ${SYSTEM_USR_DIR}")
message(STATUS "${PROJECT_NAME} SYS_INCLUDE_DIRS ${SYS_INCLUDE_DIRS}")
message(STATUS "${PROJECT_NAME} CMAKE_CXX_STANDARD_LIBRARIES = ${CMAKE_CXX_STANDARD_LIBRARIES}")

# add_subdirectory (jaulib)
add_subdirectory (src)
add_subdirectory (examples)
