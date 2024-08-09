cmake_minimum_required (VERSION 3.21.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 20 CACHE STRING "" FORCE)
# set(CMAKE_CXX_STANDARD 17 CACHE STRING "" FORCE)
set(USE_LIBCURL OFF CACHE BOOL "" FORCE)
set(BUILDJAVA OFF CACHE BOOL "" FORCE)
# set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
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

if(EMSCRIPTEN)
    message(STATUS "Gamp: EMSCRIPTEN (wasm)")
    # See https://emscripten.org/docs/tools_reference/settings_reference.html
    #
    set(gamp_EMS_FLAGS "-Wno-unused-command-line-argument")
    # set(gamp_EMS_FLAGS ${${PROJECT_NAME}_EMS_FLAGS} "SHELL:-s STANDALONE_WASM")
    set(gamp_EMS_FLAGS ${${PROJECT_NAME}_EMS_FLAGS} "SHELL:-s WASM=1" "SHELL:-s LZ4=1" "SHELL:-s EXPORTED_RUNTIME_METHODS=cwrap")
    set(gamp_EMS_FLAGS ${${PROJECT_NAME}_EMS_FLAGS} "SHELL:-s ALLOW_MEMORY_GROWTH=1")
    # set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "-pthread") # fights w/ ALLOW_MEMORY_GROWTH
    # set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "SHELL:-s MEMORY64=1") # wasm64 end-to-end: wasm32 object file can't be linked in wasm64 mode
    # set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "SHELL:-s ASSERTIONS=1")
    set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "SHELL:-s STACK_OVERFLOW_CHECK=1") # cheap cockie magic, enables CHECK_NULL_WRITES
    #
    # #set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "--use-port=sdl2" "--use-port=sdl2_image" "--use-port=sdl2_ttf")
    # #set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "SHELL:-s FULL_ES2=1") # would use clientSHELL:-side memory like FULL_ES3 -> bad performance
    # #set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "SHELL:-s USE_SDL=2" "SHELL:-s USE_SDL_IMAGE=2" "SHELL:-s USE_SDL_TTF=2")
    set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "SHELL:-s USE_SDL=2" "SHELL:-s USE_SDL_IMAGE=2")
    set(gamp_EMS_FLAGS ${gamp_EMS_FLAGS} "SHELL:-s MAX_WEBGL_VERSION=2") # WebGL 2 -> ES3

    #
    # set(EMS_EXE_LD_FLAGS "SHELL:-s SIDE_MODULE=1")
    set(gamp_EMS_EXE_LD_FLAGS ${gamp_EMS_FLAGS})
    set(gamp_EMS_STATIC_LD_FLAGS )
    message(STATUS "Gamp: gamp_EMS_FLAGS = ${gamp_EMS_FLAGS}")
    message(STATUS "Gamp: gamp_EMS_EXE_LD_FLAGS = ${gamp_EMS_EXE_LD_FLAGS}")
    message(STATUS "Gamp: gamp_EMS_STATIC_LD_FLAGS = ${gamp_EMS_STATIC_LD_FLAGS}")
else()
    message(STATUS "Gamp: No EMSCRIPTEN (wasm)")
endif()

JaulibSetup()

# Fixup clangd w/ absolute path
set (gamp_clangd_source_args "-isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/include, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/src, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/java_jni/jni, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/java_jni/jni/jau")
set (gamp_clangd_source_args "${gamp_clangd_source_args}, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/include/catch2_jau, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/jaulib/test")
set (gamp_clangd_source_args "${gamp_clangd_source_args}, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/include, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/src, -isystem, ${CMAKE_CURRENT_SOURCE_DIR}/test")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/clangd.in
                ${CMAKE_CURRENT_BINARY_DIR}/.clangd)
file(COPY_FILE ${CMAKE_CURRENT_BINARY_DIR}/.clangd ${CMAKE_CURRENT_SOURCE_DIR}/.clangd)

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

if (EMSCRIPTEN)
    message(STATUS "${PROJECT_NAME} EMSCRIPTEN build")
    set(USE_SFML OFF)
    set(SDL2_LIBS "")
else()
    message(STATUS "${PROJECT_NAME} Native build")
    set(SDL2_LIBS "SDL2;GLESv2")
endif()


find_path (SYSTEM_USR_DIR "stdlib.h")
include_directories (${SYSTEM_USR_DIR})

message(STATUS "${PROJECT_NAME} SYSTEM_USR_DIR ${SYSTEM_USR_DIR}")
message(STATUS "${PROJECT_NAME} SYS_INCLUDE_DIRS ${SYS_INCLUDE_DIRS}")
message(STATUS "${PROJECT_NAME} CMAKE_CXX_STANDARD_LIBRARIES = ${CMAKE_CXX_STANDARD_LIBRARIES}")

if (BUILD_TESTING)
  add_subdirectory (jaulib)
  enable_testing ()
  # add_subdirectory (test)
endif(BUILD_TESTING)

add_subdirectory (src)
add_subdirectory (examples)


