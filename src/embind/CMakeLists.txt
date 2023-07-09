# NOTE: the target static library from the parent scope can be referenced as a dependency as it is
#       visible from here. The coupling between this directory and the project is not an issue, given that
#       this directory only makes sense in the context of the project.
add_executable(sudoku_wasm
        em_board.cpp
        em_generator.cpp
        em_solver.cpp)

set_target_properties(sudoku_wasm
        PROPERTIES
          LINK_FLAGS "-s ENVIRONMENT=web -s SINGLE_FILE=1 -s MODULARIZE -s 'EXPORT_NAME=SudokuWasm' --bind")

target_link_libraries(sudoku_wasm LibSudoku::sudoku)