#ifndef SOLVER_H
#define SOLVER_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace sudoku {

class Board;

enum class SolverResult : uint8_t {
    NoError,
    InvalidBoard,
    EmptyBoard,
    AlreadySolved,
    HasNoSolution,
    InvalidatesCandidatesVector,
    AsyncSolvingCancelled,
    AsyncSolvingSubmitted,
    AsyncSolvingBusy
};

// Signature of callback to report progress of an async solving process.
using SolverProgressCallback = std::function<void(
    double /* progressPercentage */, unsigned /* unsolvablesFound */, unsigned /* numSolutions */)>;

// Signature of callback to report result of an async solving process.
using SolverFinishedCallback = std::function<void(
    SolverResult /* result */, std::vector<Board> /* solvedBoards */)>;

class Solver {
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
     * Solves a Sudoku puzzle in a given board, if it is solvable, using a
     * vector of unique candidate values to search for the solution. The vector
     * defines the order in which candidate values for an empty cell of the
     * board being solved will be tried. Different vectors can lead to different
     * solutions of the puzzle whenever there's more than one solution.
     *
     * @param board the board with the puzzle to be solved.
     *
     * @param candidates a vector of unique candidate values to be used
     * in the search for the solution. The vector must contain all the integers
     * in the range of 1 to 9 (included) without any repetition. If the
     * candidates doesn't conform to those constraints, a
     * SolverResult::INVALID_VALUES_VECTOR is returned.
     *
     * @param solvedBoard the board with the solution found for the puzzle.
     *
     * @return a SolverResult indicating the result of the operation.
     */
    SolverResult solve(const Board &board,
                       const std::vector<uint8_t> &candidates,
                       Board &solvedBoard);

    /**
     * Assynchronously finds the solutions for a Sudoku puzzle in a given
     * board, if the board is solvable.
     *
     * @param board the board with the puzzle to be solved.
     *
     * @param fnProgress the callback for reporting progress of the solving
     * process.
     *
     * @param fnFinished the callback for reporing result of the solving
     * process.
     *
     * @param maxSolutions the maximum number of solutions to find.
     *
     * @return SolverResult::ASYNC_SOLVING_SUBMITTED if the asynchronous request
     * for finding all solutions has been accepted or
     * SolverResult::ASYNC_SOLVING_BUSY if there's already an active solving
     * process and the request got rejected.
     */
    SolverResult asyncSolveForGood(const Board &board,
                                   const SolverProgressCallback &fnProgress,
                                   const SolverFinishedCallback &fnFinished,
                                   unsigned maxSolutions = 50u);

    /**
     * Cancels an async solving processing if there's one going on.
     *
     * Solver instances don't support more than one active async processing at a
     * time.
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

    // Internal method that does the real work for finding the solutions
    // to a given board. "level"
    /**
     * Recursevilty searches for all the possible solutions for a board, up to
     * the current value of _maxSolutions.
     *
     * @param board a board whose solutions should be found.
     *
     * @param fnProgress the callback for reporting progress of the solving
     * process.
     *
     * @param fnFinished the callback for reporing result of the solving
     * process.
     *
     * @param solutions the solutions found so far by searchSolutions.
     *
     * @param maxSolutions the maximum number of solutions to find.
     *
     * @param level the search level, 0 being the original board, 1 the level
     * where 1 empty position from the original board has been resolved and the
     * search is for possible solutions with a given first level set, and so on.
     *
     */
    void searchSolutions(const Board &board,
                         const SolverProgressCallback &fnProgress,
                         const SolverFinishedCallback &fnFinished,
                         const std::shared_ptr<std::vector<Board>> solutions,
                         unsigned maxSolutions, unsigned level);

    std::atomic<bool> _asyncSolvingCancelled;
    std::atomic<bool> _asyncSolvingActive;

    // The worker thread spawned by asyncSolveForGood.
    std::thread _solveForGoodWorker;
};

}  // namespace sudoku

#endif
