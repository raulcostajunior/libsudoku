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

class Generator
{

public:

    Generator();

    ~Generator();

    GeneratorResult asyncGenerate(PuzzleDifficulty difficulty,
                                  const GeneratorFinishedCallback &fnFinished);

    void cancelAsyncGenerate();

private:

    void generate(PuzzleDifficulty difficulty,
                  GeneratorFinishedCallback fnFinished);

    std::atomic<bool> _asyncGenCancelled;
    std::atomic<bool> _asyncGenActive;

    std::thread _genWorker;

};

} // namespace sudoku

#endif