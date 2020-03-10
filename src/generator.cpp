#include "generator.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <map>
#include <random>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "board.h"
#include "solver.h"

using namespace sudoku;
using namespace std;

vector<uint8_t> genCandidatesVector(mt19937 &randEngine) {
    vector<uint8_t> candidates;
    while (candidates.size() < Board::MAX_VAL - 1) {
        uint8_t val = randEngine() % Board::MAX_VAL + 1;
        if (find(candidates.cbegin(), candidates.cend(), val) ==
            candidates.cend()) {
            candidates.emplace_back(val);
        }
    }
    // Looks for last missing value to add to candidates vector.
    vector<bool> valuesPresent(Board::MAX_VAL, false);
    for (const uint8_t val : candidates) {
        valuesPresent[val - 1] = true;
    }
    uint8_t missingVal = static_cast<uint8_t>(
        distance(valuesPresent.cbegin(),
                 find(valuesPresent.cbegin(), valuesPresent.cend(), false)) +
        1);
    candidates.emplace_back(missingVal);

    return candidates;
}

/**
 * Returns the value and position that repeats the most in a given
 * collection of boards. The value and position are selected among the values
 * and positions that don't repeat for all the boards.
 *
 * @param boards the collection of boards in which the most repeated varying
 * value and position should be searched.
 *
 * @returns a pair with first being the value and the second being the position.
 * The position is an index for the board position and is in the interval
 * [0 .. Board::NUM_POS].
 */
pair<uint8_t, uint8_t> getMostFreqVariation(const vector<Board> &boards) {
    // Accumulates the frequencies of all the values in all the positions of all
    // the boards in the collection.
    using valuesFreqs = map<uint8_t, uint8_t>;
    vector<valuesFreqs> valuesDistrib(Board::NUM_POS, valuesFreqs());
    for (size_t nBoard = 0; nBoard < boards.size(); nBoard++) {
        for (size_t row = 0; row < Board::NUM_ROWS; row++) {
            for (size_t col = 0; col < Board::NUM_COLS; col++) {
                const uint8_t pos = row * Board::NUM_COLS + col;
                valuesDistrib[pos][boards[nBoard].valueAt(row, col)] += 1;
            }
        }
    }
    // Sweeps all the frequencies looking for the most frequent value in a given
    // position that is not the unique value for all the boards in that
    // position.
    uint8_t mfvPosition;
    uint8_t mfvValue;
    int maxFreq = 0;
    for (size_t pos = 0; pos < Board::NUM_POS; pos++) {
        if (valuesDistrib[pos].size() < 2) {
            // Skips positions for with only one value.
            continue;
        }
        for (auto it = valuesDistrib[pos].cbegin();
             it != valuesDistrib[pos].cend(); it++) {
            if (it->second > maxFreq) {
                // Found a new most frequent varying value
                maxFreq = it->second;
                mfvValue = it->first;
                mfvPosition = pos;
            }
        }
    }
    return make_pair(mfvValue, mfvPosition);
}

Generator::Generator() : _asyncGenCancelled(false), _asyncGenActive(false) {}

Generator::~Generator() {
    if (_asyncGenActive) {
        cancelAsyncGenerate();
    }
    if (_genWorker.joinable()) {
        _genWorker.join();
    }
}

uint8_t Generator::maxEmptyPositions(PuzzleDifficulty difficulty) noexcept {
    uint8_t max;

    switch (difficulty) {
        case PuzzleDifficulty::Hard:
            max = 52;
            break;
        case PuzzleDifficulty::Medium:
            max = 39;
            break;
        default:  // EASY
            max = 29;
    }
    return max;
}

uint8_t Generator::minEmptyPositions(PuzzleDifficulty difficulty) noexcept {
    uint8_t min;

    switch (difficulty) {
        case PuzzleDifficulty::Hard:
            min = 40;
            break;
        case PuzzleDifficulty::Medium:
            min = 30;
            break;
        default:  // EASY
            min = 20;
    }
    return min;
}

GeneratorResult Generator::asyncGenerate(
    PuzzleDifficulty difficulty, const GeneratorProgressCallback &fnProgress,
    const GeneratorFinishedCallback &fnFinished) {
    if (_asyncGenActive) {
        // Only one generating process can be active at once.
        return GeneratorResult::AsyncGenBusy;
    }

    _asyncGenActive = true;
    _asyncGenCancelled = false;

    _genWorker =
        thread(&Generator::generate, this, difficulty, fnProgress, fnFinished);

    return GeneratorResult::AsyncGenSubmitted;
}

void Generator::generate(PuzzleDifficulty difficulty,
                         const GeneratorProgressCallback &fnProgress,
                         const GeneratorFinishedCallback &fnFinished) {
    random_device randDev;
    mt19937 randEngine(randDev());

    const uint8_t totalSteps = 6;
    uint8_t currentStep = 1;  // Step 1 -> random candidate vector generation.

    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }

    // Generate random candidates values vector.
    vector<uint8_t> candidates = genCandidatesVector(randEngine);

    if (processGenCancelled(fnFinished)) {
        return;
    }

    currentStep++;  // Step 2 -> valid random solved board seeding
    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }

    // Initializes the generated board with a random value at a random position.
    Board genBoard;
    uint8_t initPosition = randEngine() % Board::NUM_POS;
    size_t candPosition = randEngine() % candidates.size();
    genBoard.setValueAt(initPosition / Board::NUM_COLS,
                        initPosition % Board::NUM_COLS,
                        candidates[candPosition]);
    if (processGenCancelled(fnFinished)) {
        return;
    }

    currentStep++;  // Step 3 -> valid random solved board generation
    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }
    // Solves the genBoard.
    Board solvedGenBoard;
    Solver solver;
    solver.solve(genBoard, candidates, solvedGenBoard);

    // Removes the maximum number of empty positions for the required difficulty
    // level.
    currentStep++;  // Step 4 -> empty maximum positions allowed for diff. level
    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }
    genBoard = solvedGenBoard;
    uint8_t numEmptyPos = Generator::maxEmptyPositions(difficulty);
    unordered_set<uint8_t> emptyPositions;
    while (emptyPositions.size() < numEmptyPos) {
        uint8_t pos = randEngine() % Board::NUM_POS;
        emptyPositions.insert(pos);
        if (processGenCancelled(fnFinished)) {
            return;
        }
    }
    for (const uint8_t emptyPos : emptyPositions) {
        genBoard.setValueAt(emptyPos / Board::NUM_COLS,
                            emptyPos % Board::NUM_COLS, 0);
    }

    // Reduces the number of solutions by setting some of the empty positions.
    // The positions will be optimally set to reduce the board solution set as
    // fast as possible before the minimum number of empty positions for the
    // level is reached. The positions that will be filled are those that
    // eliminate the biggest number of elements from the generated board
    // solutions set - the value and position that are most frequent among all
    // the solutions, but that are not the same for all of them. If the number
    // of solutions in the solution set reaches 1 before the minimum number of
    // empty positions in the generated board is reached, the reduction process
    // is also considered complete.
    vector<Board> boardSolutions;
    atomic<bool> solvingFinished(false);
    atomic<bool> solvingCancelled(false);
    solver.asyncSolveForGood(
        genBoard,
        [&solver, &fnFinished, &solvingCancelled, this](double progPercent,
                                                        unsigned numSolutions) {
            if (processGenCancelled(fnFinished)) {
                // Generation has been cancelled - cancel the async solving.
                solver.cancelAsyncSolving();
                solvingCancelled = true;
            }
        },
        [&boardSolutions, &solvingFinished](SolverResult result,
                                            vector<Board> solutions) {
            // Async solving finished - as we departed from a valid and solvable
            // board there's no need to test for SolverResult value.
            solvingFinished = true;
            boardSolutions = solutions;
        });
    currentStep++;  // Step 5 -> search for all solutions of the current board.
    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }
    while (!solvingFinished && !solvingCancelled) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    if (solvingCancelled) {
        // Solving cancelled (after generation cancelled). Nothing else to do.
        return;
    }

    currentStep++;  // Step 6 -> reduce number of board solutions.
    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }
    while (boardSolutions.size() > 1 &&
           genBoard.blankPositionCount() >
               Generator::minEmptyPositions(difficulty)) {
        const pair<uint8_t, uint8_t> mostFreqVarying =
            getMostFreqVariation(boardSolutions);
        // Sets the generated board position and values to the most frequent
        // value that varies and removes all boards with that value in that
        // position from the collection of solution boards.
        genBoard.setValueAt(mostFreqVarying.second / Board::NUM_COLS,
                            mostFreqVarying.second % Board::NUM_COLS,
                            mostFreqVarying.first);
        const auto newEnd = remove_if(
            boardSolutions.begin(), boardSolutions.end(),
            [&mostFreqVarying](const Board &board) {
                return board.valueAt(
                           mostFreqVarying.second / Board::NUM_COLS,
                           mostFreqVarying.second % Board::NUM_COLS) ==
                       mostFreqVarying.first;
            });
        boardSolutions.erase(newEnd, boardSolutions.end());
        if (processGenCancelled(fnFinished)) {
            return;
        }
    }

    _asyncGenActive = false;
    _asyncGenCancelled = false;
    if (fnFinished != nullptr) {
        fnFinished(GeneratorResult::NoError, genBoard);
    }
}

void Generator::cancelAsyncGenerate() { _asyncGenCancelled = true; }

bool Generator::processGenCancelled(
    const GeneratorFinishedCallback &fnFinished) {
    if (_asyncGenCancelled) {
        _asyncGenActive = false;
        _asyncGenCancelled = false;

        if (fnFinished != nullptr) {
            fnFinished(GeneratorResult::AsyncGenCancelled, Board());
        }
        return true;
    } else {
        return false;
    }
}
