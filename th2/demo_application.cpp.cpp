#include "parallel_scheduler.h"
#include <iostream>

parallel_scheduler::parallel_scheduler(size_t capacity) : stop_flag(false) {
    if (capacity == 0) {

        capacity = std::thread::hardware_concurrency();
        if (capacity == 0) capacity = 1; 
    }
    
    workers.reserve(capacity); 
    for (size_t i = 0; i < capacity; ++i) {
        workers.emplace_back(&parallel_scheduler::worker_loop, this);
    }
    std::cout << "Scheduler initialized with " << capacity << " worker threads.\n";
}

parallel_scheduler::~parallel_scheduler() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop_flag = true;
    } 

    condition.notify_all();

    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    std::cout << "Scheduler successfully stopped and threads joined.\n";
}

void parallel_scheduler::worker_loop() {
    while (true) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            condition.wait(lock, [this]{
                return stop_flag || !tasks.empty();
            });

            if (stop_flag && tasks.empty()) {
                return;
            }

            task = std::move(tasks.front());
            tasks.pop();
        } 

        if (task) {
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "Worker caught exception: " << e.what() << "\n";
            } catch (...) {
                std::cerr << "Worker caught unknown exception.\n";
            }
        }
    }
}