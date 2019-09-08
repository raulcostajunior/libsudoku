#ifndef GENERATOR_H
#define GENERATOR_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <thread>
#include <utility>

namespace sudoku
{

class Board;

enum class PuzzleDifficulty : uint8_t
{
    EASY,
    MEDIUM,
    HARD
};

enum class GeneratorResult: uint8_t
{
    NO_ERROR,
    ASYNC_GEN_CANCELLED,
    ASYNC_GEN_SUBMITTED,
    ASYNC_GEN_BUSY
};

// Signature of callback to report result of an async generation process.
using GeneratorFinishedCallback =
    std::function<void(GeneratorResult /* generated */, Board /* generated */)>;

// Signature of callback to report progress of an async generation process.
using GeneratorProgressCallback =
    std::function<void(uint8_t /* currentStep */, uint8_t /* totalSteps */)>;

class Generator
{

public:

    Generator();

    ~Generator();

    GeneratorResult asyncGenerate(PuzzleDifficulty difficulty,
                                  const GeneratorProgressCallback &fnProgress,
                                  const GeneratorFinishedCallback &fnFinished);

    void cancelAsyncGenerate();

    /**
     * The maximum number of empty positions in a board generated for a 
     * given difficulty level.
     * 
     * @param the difficulty level of the board to generate.
     * @return the maximum number of empty positions for the provided 
     *         difficulty level.
     */
    static uint8_t maxEmptyPositions(PuzzleDifficulty difficulty) noexcept;


    /**
     * The minimum number of empty positions in a board generated for a 
     * given difficulty level.
     * 
     * @param the difficulty level of the board to generate.
     * @return the minimum number of empty positions for the provided 
     *         difficulty level.
     */
    static uint8_t minEmptyPositions(PuzzleDifficulty difficulty) noexcept;

private:

    void generate(PuzzleDifficulty difficulty,
                  GeneratorProgressCallback fnProgress,
                  GeneratorFinishedCallback fnFinished);

    bool processGenCancelled(const GeneratorFinishedCallback &fnFinished);

    std::atomic<bool> _asyncGenCancelled;
    std::atomic<bool> _asyncGenActive;

    std::thread _genWorker;

};

} // namespace sudoku

#endif