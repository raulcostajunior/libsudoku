cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

project (libsudoku
    LANGUAGES
      CXX
)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)


###### Library definitions

find_package (Threads)

add_library (libsudoku_static
    STATIC
        src/board.cpp
        src/solver.cpp
        src/generator.cpp
)

# Explicitly set the library public headers so install can figure out what to
# put in ${CMAKE_INSTALL_INCLUDEDIR}. One alternative practice would be to 
# segregate those public headers in their own directory and then install the
# whole directory.
set_target_properties(libsudoku_static
    PROPERTIES 
        PUBLIC_HEADER "src/board.h;src/solver.h;src/generator.h"
)

target_link_libraries(libsudoku_static ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(libsudoku_static
    PROPERTIES
      OUTPUT_NAME "sudoku"
)


###### Tests definitions

add_executable(board_tests src/test/board_tests.cpp)
target_include_directories(board_tests PRIVATE src)
target_link_libraries(board_tests libsudoku_static)

add_executable(solver_tests src/test/solver_tests.cpp)
target_include_directories(solver_tests PRIVATE src)
target_link_libraries(solver_tests libsudoku_static)

add_executable(generator_tests src/test/generator_tests.cpp)
target_include_directories(generator_tests PRIVATE src)
target_link_libraries(generator_tests libsudoku_static)

enable_testing()

add_test(
  NAME board_tests
  COMMAND $<TARGET_FILE:board_tests> --success
)

add_test(
  NAME solver_tests
  COMMAND $<TARGET_FILE:solver_tests> --success
)

add_test(
  NAME generator_tests
  COMMAND $<TARGET_FILE:generator_tests> --success
)

###### Installation definition

include(GNUInstallDirs)

install(
  TARGETS libsudoku_static
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  TARGETS board_tests generator_tests solver_tests
  RUNTIME 
      DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
)