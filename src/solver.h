#ifndef SOLVER_H
#define SOLVER_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>

namespace sudoku
{

class Board;

enum class SolverResult : uint8_t
{
    NO_ERROR,
    INVALID_BOARD,
    EMPTY_BOARD,
    ALREADY_SOLVED,
    HAS_NO_SOLUTION,
    SOLVING_CANCELLED
};

// Signature of callback to report progress of an async solving process.
using SolverProgressCallback =
    std::function<void(double /* progressPercentage */, unsigned /* numSolutions */)>;

// Signature of callback to report result of an async solving process.
using SolverFinishedCallback =
    std::function<void(SolverResult /* result */, std::vector<Board> /* solvedBoards */)>;

class Solver
{

public:
 
    Solver();

    ~Solver();

    /**
     * Solves a Sudoku puzzle in a given board, if it is solvable.
     * 
     * @param board the board with the puzzle to be solved.
     * 
     * @param solvedBoard the board with the solution found for the puzzle.
     * 
     * @return a SolverResult indicating the result of the operation.
     */
    SolverResult solve(const Board &board, Board &solvedBoard);

    /**
     * Assynchronously finds all the solutions for a Sudoku puzzle in a given board, 
     * if the board is solvable.
     * 
     * @param board the board with the puzzle to be solved.
     * 
     * @param fnProgress the callback for reporting progress of the solving process.
     * 
     * @param fnFinished the callback for reporing result of the solving process.
     * 
     * @return true if the asynchronous request for finding all solutions has been
     * accepted. false if there's already an active solving process and the request
     * got rejected.
     */
    bool asyncSolveForGood(const Board &board,
                           const SolverProgressCallback &fnProgress,
                           const SolverFinishedCallback &fnFinished);

    /**
     * Cancels an async solving processing if there's one going on.
     * 
     * Solver instances don't support more than one active async processing at a time.
     */
    void cancelAsyncSolving();

private:

    /**
     * Checks whether a given board is potentially solvable.
     * If the board is not solvable, the reason for its insolvability
     * is also returned.
     * 
     * @param board the board to be checked.
     * 
     * @return a SolverResult that indicates that board is potentially
     * solvable when it equals SolverResult::NO_ERROR. Any other code
     * indicates that the board is not solvable and indicate the reason
     * why the board cannot be solved. 
     */
    SolverResult checkBoard(const Board &board);

    // Internal method that does the real work for finding all the solutions 
    // to a given board.
    void solveForGood(Board board,
                      SolverProgressCallback fnProgress,
                      SolverFinishedCallback fnFinished);

    std::atomic<bool> _asyncSolvingCancelled;
    std::atomic<bool> _asyncSolvingActive;

    // The worker thread spawned by asyncSolveForGood.
    std::thread _solveForGoodWorker;
};

} // namespace sudoku

#endif