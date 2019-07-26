cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

project (libsudoku
    LANGUAGES 
      CXX
)

set (CMAKE_CXX_STANDARD 11)

add_library (libsudoku_objs
    OBJECT
      board.h
      board.cpp
      solver.h
      solver.cpp
      generator.h
      generator.cpp
)

set_target_properties (libsudoku_objs 
    PROPERTIES 
      POSITION_INDEPENDENT_CODE 1
)

add_library (libsudoku_static
    STATIC
      $<TARGET_OBJECTS:libsudoku_objs>
)

set_target_properties(libsudoku_static
    PROPERTIES
      OUTPUT_NAME "libsudoku"
)

