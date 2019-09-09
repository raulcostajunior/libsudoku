#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include "board.h"
#include "generator.h"
#include "solver.h"

using namespace sudoku;
using namespace std;

Generator::Generator(): _asyncGenCancelled(false), _asyncGenActive(false) {}

Generator::~Generator() 
{
    if (_asyncGenActive) 
    {
        cancelAsyncGenerate();
    }
    if (_genWorker.joinable())
    {
        _genWorker.join();
    }
}

uint8_t Generator::maxEmptyPositions(PuzzleDifficulty difficulty) noexcept
{
    uint8_t max;

    switch (difficulty) 
    {
        case PuzzleDifficulty::HARD:
        max = 61;
        break;
        case PuzzleDifficulty::MEDIUM:
        max = 49;
        break;
        default: // EASY
        max = 37;
    }
    return max;
}

uint8_t Generator::minEmptyPositions(PuzzleDifficulty difficulty) noexcept
{
    uint8_t min;

    switch (difficulty) 
    {
        case PuzzleDifficulty::HARD:
        min = 50;
        break;
        case PuzzleDifficulty::MEDIUM:
        min = 38;
        break;
        default: // EASY
        min = 26;
    }
    return min;
}

GeneratorResult Generator::asyncGenerate(PuzzleDifficulty difficulty,
                                         const GeneratorProgressCallback &fnProgress,
                                         const GeneratorFinishedCallback &fnFinished)
{
    if (_asyncGenActive)
    {
        // Only one generating process can be active at once.
        return GeneratorResult::ASYNC_GEN_BUSY;
    }

    _asyncGenActive = true;
    _asyncGenCancelled = false;

    _genWorker = thread(&Generator::generate, this,
                        difficulty, fnProgress, fnFinished);

    return GeneratorResult::ASYNC_GEN_SUBMITTED;
}

void Generator::generate(PuzzleDifficulty difficulty,
                         GeneratorProgressCallback fnProgress,
                         GeneratorFinishedCallback fnFinished)
{
    srand(time(nullptr));

    const uint8_t totalSteps = 4;
    uint8_t currentStep = 1;

    if (fnProgress != nullptr) 
    {
        fnProgress(currentStep, totalSteps);
    }

    // Generate random candidates values vector.
    vector<uint8_t> candidates;
    while (candidates.size() < 8) {
        uint8_t val =rand()%9 + 1;
        if (find(candidates.cbegin(), candidates.cend(), val) == candidates.cend())
        {
            candidates.push_back(val);
        }

        if (processGenCancelled(fnFinished)) 
        {
            return;
        }
    }
    // Looks for last missing value to add to candidates vector.
    vector<bool> valuesPresent(9, false);
    for (const uint8_t val  : candidates) {
        valuesPresent[val-1] = true;
    }
    uint8_t missingVal = distance(valuesPresent.cbegin(),
                                  find(valuesPresent.cbegin(), valuesPresent.cend(), false)) + 1;
    candidates.push_back(missingVal);

    currentStep++;
    if (fnProgress != nullptr) 
    {
        fnProgress(currentStep, totalSteps);
    }

    // Initializes the generated board with a random value at a random position.
    Board genBoard;
    uint8_t initPosition = rand()%81;
    genBoard.setValueAt(initPosition/9, initPosition%9, candidates[5]);
    if (processGenCancelled(fnFinished)) {
        return;
    }

    currentStep++;
    if (fnProgress != nullptr) 
    {
        fnProgress(currentStep, totalSteps);
    }
    // Solves the genBoard. 
    Board solvedGenBoard;
    Solver solver;
    solver.solve(genBoard, candidates, solvedGenBoard);

    // Removes a number of solved positions depending on the difficulty level.
    currentStep++;
    if (fnProgress != nullptr) 
    {
        fnProgress(currentStep, totalSteps);
    }
    uint8_t dist = Generator::maxEmptyPositions(difficulty) - Generator::minEmptyPositions(difficulty);
    genBoard = solvedGenBoard;
    uint8_t numEmptyPos = rand()%dist + Generator::minEmptyPositions(difficulty);
    unordered_set<uint8_t> emptyPositions;
    while (emptyPositions.size() <= numEmptyPos) 
    {
        uint8_t pos = rand()%81;
        emptyPositions.insert(pos);

        if (processGenCancelled(fnFinished)) 
        {
            return;
        }
    }

    for (const uint8_t emptyPos : emptyPositions)
    {
        genBoard.setValueAt(emptyPos/9, emptyPos%9, 0);
    }

    _asyncGenActive = false;
    _asyncGenCancelled = false;
    if (fnFinished != nullptr) 
    {
        fnFinished(GeneratorResult::NO_ERROR, genBoard);
    }
}

void Generator::cancelAsyncGenerate()
{
    _asyncGenCancelled = true;
}

bool Generator::processGenCancelled(const GeneratorFinishedCallback &fnFinished) 
{
    if (_asyncGenCancelled)
    {
        _asyncGenActive = false;
        _asyncGenCancelled = false;

        if (fnFinished != nullptr) 
        {
            fnFinished(GeneratorResult::ASYNC_GEN_CANCELLED, Board());
        }
        return true;
    }
    else 
    {
        return false;
    }

}
