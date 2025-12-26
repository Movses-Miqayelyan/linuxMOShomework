#include "parallel_scheduler.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

std::mutex cout_mutex;

void compute_task(int id, int sleep_ms) {
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[START] Task " << id << " on thread " 
                  << std::this_thread::get_id() << "\n";
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms)); 

    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "[END]   Task " << id << " finished\n";
    }
}

int main() {
    const size_t POOL_CAPACITY = 4;
    const size_t TOTAL_TASKS = 20;

    try {
        parallel_scheduler pool(POOL_CAPACITY);

        std::cout << "\n>>> Enqueuing " << TOTAL_TASKS 
                  << " tasks (Pool capacity: " << POOL_CAPACITY << ") <<<\n\n";

        for (int i = 0; i < (int)TOTAL_TASKS; ++i) {
            int sleep_time = 100 + (i % 5) * 50; 
            pool.run(compute_task, i, sleep_time);
        }

        std::cout << "\n>>> All tasks enqueued. Pool is working... <<<\n\n";

        std::this_thread::sleep_for(std::chrono::seconds(5));

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "\n>>> Main thread exiting. Scheduler will stop now. <<<\n";
    return 0;
}