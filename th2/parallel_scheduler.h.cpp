#ifndef PARALLEL_SCHEDULER_H
#define PARALLEL_SCHEDULER_H

#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <future>

using Task = std::function<void()>;

class parallel_scheduler {
public:
    explicit parallel_scheduler(size_t capacity) : stop_flag(false) {
        for (size_t i = 0; i < capacity; ++i) {
            workers.emplace_back(&parallel_scheduler::worker_loop, this);
        }
    }

    ~parallel_scheduler() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop_flag = true;
        }
        condition.notify_all(); 
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // Запуск задачи
    template<class F, class... Args>
    void run(F&& f, Args&&... args) {
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop_flag) {
                return;
            }
            tasks.emplace(task);
        }
        condition.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<Task> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop_flag;

    void worker_loop() {
        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                condition.wait(lock, [this] {
                    return stop_flag || !tasks.empty();
                });

                if (stop_flag && tasks.empty()) {
                    return;
                }

                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }
};

#endif