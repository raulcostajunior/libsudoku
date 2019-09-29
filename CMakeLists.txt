cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

project (libsudoku
    LANGUAGES 
      CXX
)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (Threads)

add_library (libsudoku_objs
    OBJECT
      src/board.h
      src/board.cpp
      src/solver.h
      src/solver.cpp
      src/generator.h
      src/generator.cpp
)

set_target_properties (libsudoku_objs 
    PROPERTIES 
      POSITION_INDEPENDENT_CODE 1
)

add_library (libsudoku_static
    STATIC
      $<TARGET_OBJECTS:libsudoku_objs>
)

target_link_libraries(libsudoku_static ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(libsudoku_static
    PROPERTIES
      OUTPUT_NAME "sudoku"
)

add_executable(board_tests test/board_tests.cpp)
target_include_directories(board_tests PRIVATE src)
target_link_libraries(board_tests libsudoku_static)

add_executable(solver_tests test/solver_tests.cpp)
target_include_directories(solver_tests PRIVATE src)
target_link_libraries(solver_tests libsudoku_static)

add_executable(generator_tests test/generator_tests.cpp)
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
