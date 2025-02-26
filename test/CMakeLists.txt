include_directories(
  ${PROJECT_SOURCE_DIR}/jaulib/include
  ${PROJECT_SOURCE_DIR}/jaulib/include/catch2_jau
  ${PROJECT_SOURCE_DIR}/include
)

set (catch2_LIB_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/../jaulib/include/catch2/catch_amalgamated.cpp
)

add_library (catch2 STATIC ${catch2_LIB_SRCS})
target_compile_options(catch2 PRIVATE -DCATCH_AMALGAMATED_CUSTOM_MAIN=1 -Wno-error=unused-but-set-variable -Wno-unused-but-set-variable)
set_target_properties(catch2 PROPERTIES CXX_CLANG_TIDY "")
# install(TARGETS catch2 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# These examples use the standard separate compilation
file(GLOB_RECURSE SOURCES_IDIOMATIC_TEST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "test_*.cpp")

string( REPLACE ".cpp" "" BASENAMES_IDIOMATIC_TEST "${SOURCES_IDIOMATIC_TEST}" )
set( TARGETS_IDIOMATIC_TEST ${BASENAMES_IDIOMATIC_TEST} )

foreach( name ${TARGETS_IDIOMATIC_TEST} )
    add_executable(${name} ${name}.cpp)
    target_compile_options(${name} PUBLIC "${gamp_CXX_FLAGS}")
    target_link_options(${name} PUBLIC "${gamp_EXE_LINKER_FLAGS}")

    target_link_libraries(${name} gamp catch2 ${SDL2_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    add_dependencies(${name} gamp catch2)
    add_test (NAME ${name} COMMAND ${name})
    if(DEFINED CMAKE_CXX_CLANG_TIDY)
        set_target_properties(${name} PROPERTIES CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};--checks;-bugprone-chained-comparison")
    endif()
endforeach()

