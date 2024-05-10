#
# cmake build settings, modularized to be optionally included by parent projects
#

include_guard(GLOBAL)

macro(WasmSetup)

message(STATUS "Wasm Setup: ${PROJECT_NAME}")


if (EMSCRIPTEN)
    message(STATUS "${PROJECT_NAME} (wasm) EMSCRIPTEN build")
    # See https://emscripten.org/docs/tools_reference/settings_reference.html
    #
    # set(EMS_FLAGS "--use-port=sdl2 --use-port=sdl2_image --use-port=sdl2_ttf")
    set(EMS_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_SDL_TTF=2 -Wno-unused-command-line-argument")
    set(EMS_FLAGS "${EMS_FLAGS} -s WASM=1 -s LZ4=1 -s EXPORTED_RUNTIME_METHODS=cwrap")
    set(EMS_FLAGS "${EMS_FLAGS} -s FULL_ES2=1") # or even FULL_ES3 ?
    set(EMS_FLAGS "${EMS_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
    # set(EMS_FLAGS "${EMS_FLAGS} -pthread") # fights w/ ALLOW_MEMORY_GROWTH
    # set(EMS_FLAGS "${EMS_FLAGS} -s MEMORY64=1") # wasm64 end-to-end: wasm32 object file can't be linked in wasm64 mode
    # set(EMS_FLAGS "${EMS_FLAGS} -s ASSERTIONS=1")
    set(EMS_FLAGS "${EMS_FLAGS} -s STACK_OVERFLOW_CHECK=1") # cheap cockie magic, enables CHECK_NULL_WRITES
    # set(EMS_EXE_FLAGS "-s SIDE_MODULE=1")
    set(EMS_EXE_FLAGS "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMS_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMS_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMS_FLAGS}")
    set(USE_SFML OFF)
    # set(DONT_USE_RTTI ON)
    set(USE_LIBUNWIND OFF)
    set(SDL2_LIBS "")
else()
    message(STATUS "${PROJECT_NAME} Native build")
    set(SDL2_LIBS "SDL2;GLESv2")
endif()

message(STATUS "${PROJECT_NAME} CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "${PROJECT_NAME} CMAKE_SHARED_LINKER_FLAGS = ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "${PROJECT_NAME} CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "${PROJECT_NAME} CMAKE_CXX_STANDARD_LIBRARIES = ${CMAKE_CXX_STANDARD_LIBRARIES}")
message(STATUS "${PROJECT_NAME} LIB_INSTALL_DIR = ${LIB_INSTALL_DIR}")

endmacro()
