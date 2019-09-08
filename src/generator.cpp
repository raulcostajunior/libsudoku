#include <atomic>
#include <cstdint>
#include <functional>
#include <random>
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

GeneratorResult Generator::asyncGenerate(PuzzleDifficulty difficulty,
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
                        difficulty, fnFinished);

    return GeneratorResult::ASYNC_GEN_SUBMITTED;
}

void Generator::generate(PuzzleDifficulty difficulty,
                         const GeneratorFinishedCallback fnFinished)
{
    default_random_engine eng;
    uniform_int_distribution<uint8_t> dist(1,9);
    auto randValGen = bind(dist, eng);

    // Generate random candidates values vector.
    vector<uint8_t> candidates;
    while (candidates.size() < 9) {
        uint8_t val = randValGen();
        if (find(candidates.cbegin(), candidates.cend(), val) == candidates.cend())
        {
            candidates.push_back(val);
        }
        
        if (_asyncGenCancelled)
        {
            _asyncGenActive = false;
            _asyncGenCancelled = false;

            if (fnFinished != nullptr) 
            {
                fnFinished(GeneratorResult::ASYNC_GEN_CANCELLED, Board());
            }
            return;
        }

    }
    // Looks for last missing value
    vector<bool> valuesPresent(10, false);
    for (const uint8_t val  : candidates) {
        valuesPresent[val] = true;
    }
    uint8_t missingVal = distance(valuesPresent.cbegin(),
                                  find(valuesPresent.cbegin(), valuesPresent.cend(), false));
    candidates.push_back(missingVal);

    // Initializes the generated board with 9 random positions with random unique values.
    Board genBoard;
    uniform_int_distribution<uint8_t> posDist(0,80);
    auto randPosGen = bind(posDist, eng);
    unordered_set<uint8_t> initPositions;
    while (initPositions.size() < 10) 
    {
        uint8_t pos = randPosGen();
        initPositions.insert(pos);
    }
    size_t candidatesIdx  = 0;
    for (const uint8_t initPos : initPositions)
    {
        genBoard.setValueAt(initPos/9, initPos%9,
                            candidates[candidatesIdx]);
        candidatesIdx++;
    }

    if (_asyncGenCancelled)
    {
        _asyncGenActive = false;
        _asyncGenCancelled = false;

        if (fnFinished != nullptr) 
        {
            fnFinished(GeneratorResult::ASYNC_GEN_CANCELLED, Board());
        }
        return;
    }

    // Solves the genBoard and calls the position remover corresponding
    // to the difficulty level to derive genBoard from the solved genBoard.
    Board solvedGenBoard;
    Solver solver;
    solver.solve(genBoard, candidates, solvedGenBoard);
    // TODO: define and call the position removers for each difficulty level.

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



