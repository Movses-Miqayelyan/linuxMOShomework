#include "shared_array.hpp"
#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <random>
#include <csignal>
#include <atomic>

constexpr size_t ARRAY_SIZE = 1000;
const std::string ARRAY_NAME = "ipc3_test_array";

std::atomic<bool> keep_running(true);

void signal_handler(int signum) {
    std::cout << "\n[System] Signal received, stopping reader...\n";
    keep_running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, ARRAY_SIZE - 1);

    try {

        SharedArray arr(ARRAY_NAME, ARRAY_SIZE); 

        std::cout << "[SECOND] Reader/Modifier started. PID: " << getpid() << "\n";
        std::cout << "[INFO] Watching shared memory: /dev/shm/" << ARRAY_NAME << "\n";

        while (keep_running) {
            {
                arr.lock();

                size_t index = dist(gen);
                int value = arr[index];

                std::cout << "[SECOND] Reading index " << index << ": " << value;

                arr[index] = value + 1;
                std::cout << " -> Incrementing to: " << arr[index] << "\n" << std::flush;

                arr.unlock();
            }

            sleep(2);
        }

    } catch (const std::exception& e) {
        std::cerr << "[SECOND] Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "[SECOND] Process gracefully stopped.\n";
    return 0;
}