include_directories(
  ${PROJECT_SOURCE_DIR}/jaulib/include
  ${PROJECT_SOURCE_DIR}/include
)

# These examples use the standard separate compilation
if (EMSCRIPTEN)
    set(SOURCES_IDIOMATIC_TARGETS "redsquare01.cpp")
else()
    file(GLOB SOURCES_IDIOMATIC_TARGETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
endif()

string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_TARGETS "${SOURCES_IDIOMATIC_TARGETS}" )

set( SDL_TARGETS_IDIOMATIC_TARGETS "" )
set( SDL_TARGETS_IDIOMATIC_FILES "" )
foreach( name ${BASENAMES_IDIOMATIC_TARGETS} )
    set(target ${name})
    set( SDL_TARGETS_IDIOMATIC_TARGETS "${SDL_TARGETS_IDIOMATIC_TARGETS};${target}" )
    add_executable(${target} ${name}.cpp)
    target_link_libraries(${target} gamp ${SDL2_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    if (DEFINED EMSCRIPTEN)
        configure_file(${name}.html ${name}.html COPYONLY)
        # configure_file(${PROJECT_SOURCE_DIR}/fonts/freefont/FreeSansBold.ttf fonts/freefont/FreeSansBold.ttf COPYONLY)
        # set_target_properties(${target} PROPERTIES LINK_FLAGS "--embed-file=fonts ${EMS_EXE_FLAGS}")
        set( SDL_TARGETS_IDIOMATIC_FILES "${SDL_TARGETS_IDIOMATIC_FILES};${CMAKE_CURRENT_BINARY_DIR}/${name}.wasm;${name}.html" )
        set_target_properties(${target} PROPERTIES LINK_FLAGS "${EMS_EXE_FLAGS}")
        set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${EMS_EXE_FLAGS}")
    endif()
endforeach()

install(TARGETS ${SDL_TARGETS_IDIOMATIC_TARGETS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${SDL_TARGETS_IDIOMATIC_FILES} DESTINATION ${CMAKE_INSTALL_BINDIR})
