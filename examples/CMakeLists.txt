include_directories(
  ${PROJECT_SOURCE_DIR}/jaulib/include
  ${PROJECT_SOURCE_DIR}/include
)

# These examples use the standard separate compilation
if (EMSCRIPTEN)
    file(GLOB SOURCES_IDIOMATIC_TARGETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
    #set(SOURCES_IDIOMATIC_TARGETS "RedSquareES2.cpp")
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
    target_compile_options(${target} PUBLIC ${gamp_CXX_FLAGS})
    if (DEFINED EMSCRIPTEN)
        # Launch html from CMAKE_CURRENT_SOURCE_DIR -> CMAKE_CURRENT_BINARY_DIR
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${name}.html)
            configure_file(${name}.html ${name}.html COPYONLY)
        else()
            set (APP_NAME ${name})
            configure_file(HTMLLauncher01.html ${name}.html)
            unset (APP_NAME)
        endif()
        file(COPY ${PROJECT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        # configure_file(${PROJECT_SOURCE_DIR}/resources/fonts/freefont/FreeSansBold.ttf resources/fonts/freefont/FreeSansBold.ttf COPYONLY)
        set( SDL_TARGETS_IDIOMATIC_FILES "${SDL_TARGETS_IDIOMATIC_FILES};${CMAKE_CURRENT_BINARY_DIR}/${name}.wasm;${CMAKE_CURRENT_BINARY_DIR}/${name}.html;${CMAKE_CURRENT_SOURCE_DIR}/style.css" )
        target_link_options(${target} PUBLIC ${gamp_EXE_LINKER_FLAGS} "--embed-file=resources")
    else()
        target_link_options(${target} PUBLIC ${gamp_EXE_LINKER_FLAGS})
    endif()
    target_link_libraries(${target} gamp ${SDL2_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endforeach()

install(TARGETS ${SDL_TARGETS_IDIOMATIC_TARGETS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${SDL_TARGETS_IDIOMATIC_FILES} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/resources/examples DESTINATION ${CMAKE_INSTALL_DATADIR}/gamp)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/resources/demos DESTINATION ${CMAKE_INSTALL_DATADIR}/gamp)
