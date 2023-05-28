#define CATCH_CONFIG_MAIN

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "../board.h"
#include "../generator.h"
#include "../solver.h"
#include "catch.hpp"

using namespace sudoku;
using namespace std;

static unsigned _timeoutSecs = 1200u;

GeneratorResult generate(PuzzleDifficulty difficulty, Board &generatedBoard,
                         unsigned timeoutSecs) {
    GeneratorResult result;
    atomic<bool> finished{false};
    Generator gen;

    auto asyncGenFinished = [&generatedBoard, &result, &finished](
                                GeneratorResult genResult,
                                const Board &genBoard) {
        result = genResult;
        generatedBoard = genBoard;
        finished = true;
    };

    auto asyncGenProgress = [](uint8_t currentStep, uint8_t totalSteps) {
        clog << "Performing generating step '" << static_cast<int>(currentStep)
             << "' of '" << static_cast<int>(totalSteps) << "'..." << endl;
    };

    auto startTime = chrono::system_clock::now();
    clog << "Generating board with difficulty level '"
         << static_cast<int>(difficulty) << "' ..." << endl;

    result = gen.asyncGenerate(difficulty, asyncGenProgress, asyncGenFinished);
    if (result != GeneratorResult::AsyncGenSubmitted) {
        return result;
    }

    unsigned int numOfWaits = 0;
    while (!finished && numOfWaits < timeoutSecs) {
        this_thread::sleep_for(chrono::seconds(1));
        numOfWaits++;
    }

    if (numOfWaits < timeoutSecs) {
        auto stopTime = chrono::system_clock::now();

        clog << "... generated in "
             << chrono::duration_cast<chrono::milliseconds>(stopTime -
                                                            startTime)
                    .count()
             << " milliseconds:\n"
             << generatedBoard
             << "\nblanks:" << (int)generatedBoard.blankPositionCount()
             << "\nmaxBlank allowed:"
             << (int)Generator::maxEmptyPositions(difficulty) << endl;
    } else {
        // Timed-out: cancel
        gen.cancelAsyncGenerate();
    }

    return result;
}

TEST_CASE("asyncGenerate can generate solvable EASY puzzle") {
    Board genBoard;

    auto resultGen = generate(PuzzleDifficulty::Easy, genBoard, _timeoutSecs);
    REQUIRE(resultGen == GeneratorResult::NoError);
    REQUIRE(genBoard.isValid());

    auto nBlanks = genBoard.blankPositionCount();
    REQUIRE((nBlanks <= Generator::maxEmptyPositions(PuzzleDifficulty::Easy)));

    Solver solver;
    Board solvedBoard;
    auto resultSolve = solver.solve(genBoard, solvedBoard);
    REQUIRE(resultSolve == SolverResult::NoError);
    REQUIRE(solvedBoard.isComplete());

    vector<Board> genBoardSolutions;
    SolverResult resultSolveAll;
    resultSolveAll = solver.asyncSolveForGood(
        genBoard, nullptr,
        [&resultSolveAll, &genBoardSolutions](SolverResult result,
                                              const vector<Board> &solutions) {
            resultSolveAll = result;
            genBoardSolutions = solutions;
        },
        2);
    while (resultSolveAll != SolverResult::NoError &&
           resultSolveAll != SolverResult::AsyncSolvingCancelled) {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    clog << "Number of solutions for generated board: "
         << genBoardSolutions.size() << endl;
    for (size_t i = 0; i < genBoardSolutions.size(); i++) {
        clog << "Board #" << i << ":\n" << genBoardSolutions[i] << endl;
    }
    REQUIRE(genBoardSolutions.size() == 1);
}

TEST_CASE("asyncGenerate can generate solvable MEDIUM puzzle") {
    Board genBoard;

    auto resultGen = generate(PuzzleDifficulty::Medium, genBoard, _timeoutSecs);
    REQUIRE(resultGen == GeneratorResult::NoError);
    REQUIRE(genBoard.isValid());

    auto nBlanks = genBoard.blankPositionCount();

    REQUIRE(
        (nBlanks <= Generator::maxEmptyPositions(PuzzleDifficulty::Medium)));

    Solver solver;
    Board solvedBoard;
    auto resultSolve = solver.solve(genBoard, solvedBoard);
    REQUIRE(resultSolve == SolverResult::NoError);
    REQUIRE(solvedBoard.isComplete());

    vector<Board> genBoardSolutions;
    SolverResult resultSolveAll;
    resultSolveAll = solver.asyncSolveForGood(
        genBoard, nullptr,
        [&resultSolveAll, &genBoardSolutions](SolverResult result,
                                              const vector<Board> &solutions) {
            resultSolveAll = result;
            genBoardSolutions = solutions;
        },
        2);
    while (resultSolveAll != SolverResult::NoError &&
           resultSolveAll != SolverResult::AsyncSolvingCancelled) {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    clog << "Number of solutions for generated board: "
         << genBoardSolutions.size() << endl;
    for (size_t i = 0; i < genBoardSolutions.size(); i++) {
        clog << "Board #" << i << ":\n" << genBoardSolutions[i] << endl;
    }
    REQUIRE(genBoardSolutions.size() == 1);
}

TEST_CASE("asyncGenerate can generate solvable HARD puzzle") {
    Board genBoard;

    auto resultGen = generate(PuzzleDifficulty::Hard, genBoard, _timeoutSecs);
    REQUIRE(resultGen == GeneratorResult::NoError);
    REQUIRE(genBoard.isValid());

    auto nBlanks = genBoard.blankPositionCount();
    REQUIRE((nBlanks <= Generator::maxEmptyPositions(PuzzleDifficulty::Hard)));

    Solver solver;
    Board solvedBoard;
    auto resultSolve = solver.solve(genBoard, solvedBoard);
    REQUIRE(resultSolve == SolverResult::NoError);
    REQUIRE(solvedBoard.isComplete());

    vector<Board> genBoardSolutions;
    SolverResult resultSolveAll;
    resultSolveAll = solver.asyncSolveForGood(
        genBoard, nullptr,
        [&resultSolveAll, &genBoardSolutions](SolverResult result,
                                              const vector<Board> &solutions) {
            resultSolveAll = result;
            genBoardSolutions = solutions;
        },
        2);
    while (resultSolveAll != SolverResult::NoError &&
           resultSolveAll != SolverResult::AsyncSolvingCancelled) {
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    clog << "Number of solutions for generated board: "
         << genBoardSolutions.size() << endl;
    for (size_t i = 0; i < genBoardSolutions.size(); i++) {
        clog << "Board #" << i << ":\n" << genBoardSolutions[i] << endl;
    }
    REQUIRE(genBoardSolutions.size() == 1);
}

TEST_CASE("Cannot spawn more than one asyncGenerate simultaneously") {
    Generator gen;
    auto result = gen.asyncGenerate(PuzzleDifficulty::Hard, nullptr, nullptr);

    auto secondResult =
        gen.asyncGenerate(PuzzleDifficulty::Hard, nullptr, nullptr);

    REQUIRE(result == GeneratorResult::AsyncGenSubmitted);
    REQUIRE(secondResult == GeneratorResult::AsyncGenBusy);

    gen.cancelAsyncGenerate();
}
