
# libsudoku

[![Build and Test Status](https://github.com/raulcostajunior/libsudoku/actions/workflows/cmake.yml/badge.svg)](https://github.com/raulcostajunior/libsudoku/actions/workflows/cmake.yml)

C++ library for generating and solving Sudoku puzzles.

## Building

The library and the unit test programs can be built with CMake 3.5 or later.

Using CMake with the default C/C++ toolchain, the library will be built as a static library against which the unit tests programs will be linked: 

```
git clone https://github.com/raulcostajunior/libsudoku.git
cd libsudoku
mkdir build
cmake -S . -B ./build  
cmake --build ./build
```

Building with [emscripten](https://emscripten.org) is also supported. WASM binary modules, compatible with [nodejs](https://nodejs.org) will be produced as the build artifacts:

```
git clone https://github.com/raulcostajunior/libsudoku.git
cd libsudoku
mkdir build_wasm
emcmake cmake -S . -B ./build_wasm
cmake --build ./build_wasm  
```

To run the unit tests WASM modules:

```
cd build_wasm
node board_tests.js
node generator_tests.js
node solver_tests.js
```



