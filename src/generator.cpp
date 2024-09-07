#include "generator.h"

#include <atomic>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <thread>
#include <unordered_set>
#include <vector>

#include "board.h"
#include "solver.h"

using namespace sudoku;
using namespace std;

vector<uint8_t> genCandidatesVector(mt19937 &randEngine) {
    // Ordered vector with values to be randomically transferred to the
    // candidates vector being generated.
    vector<uint8_t> availables(Board::MAX_VAL);
    iota(begin(availables), end(availables), 1);

    vector<uint8_t> candidates;
    candidates.reserve(Board::MAX_VAL);

    while (availables.size() > 1) {
        const size_t idx = randEngine() % availables.size();
        candidates.emplace_back(availables[idx]);
        availables.erase(availables.begin() + idx);
    }
    candidates.emplace_back(availables[0]);
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
    for (const auto &board : boards) {
        for (uint8_t row = 0; row < Board::NUM_ROWS; row++) {
            for (uint8_t col = 0; col < Board::NUM_COLS; col++) {
                const auto pos =
                    static_cast<uint8_t>(row * Board::NUM_COLS + col);
                valuesDistrib[pos][board.valueAt(row, col)] += 1;
            }
        }
    }
    // Sweeps all the frequencies looking for the less frequent value in a
    // position that has the biggest distance between the less frequent value
    // frequency and the accumulated frequencies for that position.
    uint8_t lfvPosition = 0;
    uint8_t lfvValue = 0;
    int maxDist = 0;
    for (size_t pos = 0; pos < Board::NUM_POS; pos++) {
        if (valuesDistrib[pos].size() < 2) {
            // Skips positions for with only one value.
            continue;
        }
        int minFreq = numeric_limits<int>::max();
        uint8_t minFreqValue = 0;
        int totalFreq = 0;
        for (auto iter : valuesDistrib[pos]) {
            totalFreq += iter.second;
            if (iter.second < minFreq) {
                minFreq = iter.second;
                minFreqValue = iter.first;
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
    static const uint8_t MAX_HARD = 58;
    static const uint8_t MAX_MEDIUM = 48;
    static const uint8_t MAX_EASY = 34;

    switch (difficulty) {
        case PuzzleDifficulty::Hard:
            return MAX_HARD;
        case PuzzleDifficulty::Medium:
            return MAX_MEDIUM;
        default:
            return MAX_EASY;
    }
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
    static const unsigned long long POLL_INTERVAL_SOLVE_MILLI = 100;
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

    clog << "Generating board with difficulty '" << static_cast<int>(difficulty)
         << "' and candidates vector: '" << candidates[0] << ", "
         << candidates[1] << ", " << candidates[2] << ", " << candidates[3]
         << ", ...'" << endl;

    if (processGenCancelled(fnFinished)) {
        return;
    }

    currentStep++;  // Step 2 -> valid random solved board seeding
    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }

    // Initializes the generated board with a random value at a random position.
    Board genBoard;
    uint8_t initialPos = randEngine() % Board::NUM_POS;
    size_t candidatePos = randEngine() % candidates.size();
    genBoard.setValueAt(initialPos / Board::NUM_COLS,
                        initialPos % Board::NUM_COLS, candidates[candidatePos]);
    if (processGenCancelled(fnFinished)) {
        return;
    }

    currentStep++;  // Step 3 -> valid random solved board generation
    if (fnProgress != nullptr) {
        fnProgress(currentStep, totalSteps);
    }
    // Solves the genBoard.
    Board solvedGenBoard;

    Solver::solveWithCandidates(genBoard, candidates, solvedGenBoard);

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
    while (true) {
        vector<Board> boardSolutions;
        atomic<bool> solvingFinished(false);
        atomic<bool> solvingCancelled(false);

        if (solvingCancelled) {
            // Solving cancelled (after generation cancelled). Nothing else to
            // do.
            return;
        }

        Solver solver;
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
            [&boardSolutions, &solvingFinished](
                SolverResult, const vector<Board> &solutions) {
                // Async solving finished - as we departed from a valid and
                // solvable board there's no need to test for SolverResult
                // value.
                solvingFinished = true;
                boardSolutions = solutions;
            },
            20);

        // Waits for the async search for solutions to finish.
        while (!solvingFinished && !solvingCancelled) {
            this_thread::sleep_for(
                chrono::milliseconds(POLL_INTERVAL_SOLVE_MILLI));
        }

        if (boardSolutions.size() == 1) {
            // Current genBoard only has one solution; it is a valid Sudoku
            // puzzle; leave reduction phase.
            break;
        }

        // Current genBoard still has more than one solution; continue
        // reduction of empty positions.
        const pair<uint8_t, uint8_t> lessFreqVariation =
            getLessFreqVariation(boardSolutions);
        const uint8_t lfvRow = lessFreqVariation.second / Board::NUM_COLS;
        const uint8_t lfvCol = lessFreqVariation.second % Board::NUM_COLS;
        genBoard.setValueAt(lfvRow, lfvCol, lessFreqVariation.first);

    }  // while (true);

    _asyncGenActive = false;
    _asyncGenCancelled = false;
    if (fnFinished != nullptr) {
        fnFinished(GeneratorResult::NoError, genBoard);
    }
}

void Generator::cancelAsyncGenerate() { _asyncGenCancelled = true; }

bool Generator::processGenCancelled(
    const GeneratorFinishedCallback &fnFinished) {
    bool cancelled = false;
    if (_asyncGenCancelled) {
        _asyncGenActive = false;
        _asyncGenCancelled = false;

        if (fnFinished != nullptr) {
            fnFinished(GeneratorResult::AsyncGenCancelled, Board());
        }
        cancelled = true;
    }
    return cancelled;
}
