cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

project (libsudoku
    LANGUAGES
      CXX
)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)


###### Library definitions

find_package (Threads)

add_library(sudoku
    STATIC
        src/board.cpp
        src/solver.cpp
        src/generator.cpp
)

add_library(LibSudoku::sudoku ALIAS sudoku)

# Explicitly set the library public headers so install can figure out what to
# put in ${CMAKE_INSTALL_INCLUDEDIR}. One alternative practice would be to
# segregate those public headers in their own directory and then install the
# whole directory.
set_target_properties(sudoku
    PROPERTIES
        PUBLIC_HEADER "src/board.h;src/solver.h;src/generator.h"
)

target_link_libraries(sudoku ${CMAKE_THREAD_LIBS_INIT})

###### Tests definitions

add_executable(board_tests src/test/board_tests.cpp)
target_include_directories(board_tests PRIVATE src)
target_link_libraries(board_tests sudoku)

add_executable(solver_tests src/test/solver_tests.cpp)
target_include_directories(solver_tests PRIVATE src)
target_link_libraries(solver_tests sudoku)

add_executable(generator_tests src/test/generator_tests.cpp)
target_include_directories(generator_tests PRIVATE src)
target_link_libraries(generator_tests sudoku)

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

# include(GNUInstallDirs)

# install(
#   TARGETS sudoku
#   EXPORT libsudoku
#   INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
# )

# install(
#   TARGETS board_tests generator_tests solver_tests
#   RUNTIME
#       DESTINATION ${CMAKE_INSTALL_BINDIR}/tests
# )

# install(EXPORT libsudoku
#   DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LibSudoku
#   NAMESPACE LibSudoku::
#   FILE LibSudokuConfig.cmake
# )