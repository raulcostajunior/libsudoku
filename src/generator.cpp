#include "generator.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <limits>
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
 * Returns the less frequent value in a position where the difference between
 * the total frequency for the different values for the position and the minimal
 * frequency across a given set of boards is maximized.
 *
 * @param boards the collection of boards where the less frequent value and its
 * position are to be found.
 *
 * @returns a pair where first is the value and second is the position being
 * searched across the collection of boards. The position is an index for the
 * board position and is in the interval [0 .. Board::NUM_POS].
 */
pair<uint8_t, uint8_t> getLessFreqVariation(const vector<Board> &boards) {
    // Accumulates the frequencies of all the values in all the positions of all
    // the boards in the collection.
    using valuesFreqs = map<uint8_t, uint8_t>;
    vector<valuesFreqs> valuesDistrib(Board::NUM_POS, valuesFreqs());
    for (size_t nBoard = 0; nBoard < boards.size(); nBoard++) {
        for (uint8_t row = 0; row < Board::NUM_ROWS; row++) {
            for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
                const uint8_t pos =
                    static_cast<uint8_t>(row * Board::NUM_COLS + col);
                valuesDistrib[pos][boards[nBoard].valueAt(row, col)] += 1;
            }
        }
    }
    // Sweeps all the frequencies looking for the less frequent value in a
    // position that has the biggest distance between the less frequent value
    // frequency and the accumulated frequencies for that position.
    uint8_t lfvPosition;
    uint8_t lfvValue;
    int maxDist = 0;
    for (size_t pos = 0; pos < Board::NUM_POS; pos++) {
        if (valuesDistrib[pos].size() < 2) {
            // Skips positions for with only one value.
            continue;
        }
        int minFreq = numeric_limits<int>::max();
        uint8_t minFreqValue = 0;
        int totalFreq = 0;
        for (auto it = valuesDistrib[pos].cbegin();
             it != valuesDistrib[pos].cend(); it++) {
            totalFreq += it->second;
            if (it->second < minFreq) {
                minFreq = it->second;
                minFreqValue = it->first;
            }
        }
        if (totalFreq - minFreq > maxDist) {
            // Found a new maximum for the difference between the accumulated
            // frequency for a position and the smaller value frequency for that
            // position.
            maxDist = totalFreq - minFreq;
            lfvValue = minFreqValue;
            lfvPosition = static_cast<uint8_t>(pos);
        }
    }
    return make_pair(lfvValue, lfvPosition);
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
            max = 58;
            break;
        case PuzzleDifficulty::Medium:
            max = 48;
            break;
        default:  // EASY
            max = 34;
    }
    return max;
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

    // The last step, reduction of empty positions to guarantee single solution,
    // is the one that takes longer, specially for the Hard level.
    const uint8_t totalSteps = 5;

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

    // Steps 5 -> Fill the empty positions one by one until the generated
    // board has only one solution.
    currentStep++;
    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }

    // The positions will be optimally set to reduce the board solution set as
    // fast as possible.
    do {
        vector<Board> boardSolutions;
        atomic<bool> solvingFinished(false);
        atomic<bool> solvingCancelled(false);

        if (solvingCancelled) {
            // Solving cancelled (after generation cancelled). Nothing else to
            // do.
            return;
        }

        solver.asyncSolveForGood(
            genBoard,
            [&solver, &fnFinished, &solvingCancelled, this](double, unsigned,
                                                            unsigned) {
                if (processGenCancelled(fnFinished)) {
                    // Generation has been cancelled - cancel the async solving.
                    solver.cancelAsyncSolving();
                    solvingCancelled = true;
                }
            },
            [&boardSolutions, &solvingFinished](SolverResult,
                                                vector<Board> solutions) {
                // Async solving finished - as we departed from a valid and
                // solvable board there's no need to test for SolverResult
                // value.
                solvingFinished = true;
                boardSolutions = solutions;
            },
            2);

        // Waits for the async search for solutions to finish.
        while (!solvingFinished && !solvingCancelled) {
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        if (boardSolutions.size() == 1) {
            // Current genBoard only has one solution; it is a valid Sudoku
            // puzzle; leave reduction phase.
            break;
        } else {
            // Current genBoard still has more than one solution; continue
            // reduction of empty positions.
            const pair<uint8_t, uint8_t> lessFreqVariation =
                getLessFreqVariation(boardSolutions);
            const uint8_t lfvRow = lessFreqVariation.second / Board::NUM_COLS;
            const uint8_t lfvCol = lessFreqVariation.second % Board::NUM_COLS;
            genBoard.setValueAt(lfvRow, lfvCol, lessFreqVariation.first);
        }

    } while (true);

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
