#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include "grid.h"
#include "concurrentqueue.h"

using namespace std::literals::chrono_literals;

const char* sudoku0 =
"003020600\n"
"900305001\n"
"001806400\n"
"008102900\n"
"700000008\n"
"006708200\n"
"002609500\n"
"800203009\n"
"005010300";

const char* sudoku1 =
"005300000\n"
"800000020\n"
"070010500\n"
"400005300\n"
"010070006\n"
"003200080\n"
"060500009\n"
"004000030\n"
"000009700";

const char* sudoku2 =
"850002400\n"
"720000009\n"
"004000000\n"
"000107002\n"
"305000900\n"
"040000000\n"
"000080070\n"
"017000000\n"
"000036040";

moodycamel::ConcurrentQueue<Grid>   todo_queue;
std::atomic_bool done{false};
std::mutex write_lock;
Grid result;

void worker()
{
    Grid grid;
    do {
        bool isDone;
        while (!(isDone = done)) {
            if (todo_queue.try_dequeue(grid))
                break;
        }
        if (isDone)
            return;

        while (grid.update()) {}
        const auto state = grid.getState();
        if (state == Grid::SOLVED && grid.isCorrectSolution()) {
            std::lock_guard<std::mutex> guard(write_lock);
            done = true;
            result = grid;
            return;
        } else if (state == Grid::INCOMPLETE) {
            struct TodoItem
            {
                TodoItem(std::uint8_t idx_, std::uint8_t size_) : idx{idx_}, size{size_} {}
                std::uint8_t idx;
                std::uint8_t size;
            };
            std::pair<std::uint8_t, std::uint8_t> todo(0, 128);
            auto& digits = grid.getDigits();
            for (std::uint8_t i = 0; i < 9 * 9; ++i) {
                const std::uint8_t size = digits[i].size();
                if (size > 1 && size < todo.second) {
                    todo.first = i;
                    todo.second = size;
                    if (size == 2)
                        break;
                }
            }

            for (const auto& possible : digits[todo.first]) {
                Grid tmp = grid;
                tmp.getDigits()[todo.first] = possible;
                todo_queue.enqueue(tmp);
            }
        }
    } while (true);
}


int main()
{
    const auto num_threads = std::thread::hardware_concurrency() == 0 ? 1 : std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (std::size_t i = 0; i < num_threads; ++i)
        threads.emplace_back(worker);
    Grid grid(sudoku1);
    std::cout << "num threads: " << num_threads << '\n';

    const auto start = std::chrono::high_resolution_clock::now();
    todo_queue.enqueue(grid);
    for (auto& thread : threads)
        thread.join();
    const auto end = std::chrono::high_resolution_clock::now();

    std::cout << result;
    std::cout << (result.isCorrectSolution() ? "solution checks out\n" : "solution is not correct!\n");
    std::cout << "took: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
}
