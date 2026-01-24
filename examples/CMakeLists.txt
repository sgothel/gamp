include_directories(
  ${PROJECT_SOURCE_DIR}/jaulib/include
  ${PROJECT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/../ports/glutess2/include
)

# These examples use the standard separate compilation
file(GLOB SOURCES_IDIOMATIC_TARGETS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_TARGETS "${SOURCES_IDIOMATIC_TARGETS}" )

set(TARGETS_USING_GLUTESS2 Primitives02 Primitives03 Unused)

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
    set(${target}_LIBS "gamp" ${SDL2_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    if (${target} IN_LIST TARGETS_USING_GLUTESS2)
      set(${target}_LIBS ${${target}_LIBS} "glutess2")
    endif()
    message(STATUS "${target} libs: ${${target}_LIBS}")
    target_link_libraries(${target} ${${target}_LIBS})
endforeach()

install(TARGETS ${SDL_TARGETS_IDIOMATIC_TARGETS} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${SDL_TARGETS_IDIOMATIC_FILES} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/resources/examples DESTINATION ${CMAKE_INSTALL_DATADIR}/gamp)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/resources/demos DESTINATION ${CMAKE_INSTALL_DATADIR}/gamp)
