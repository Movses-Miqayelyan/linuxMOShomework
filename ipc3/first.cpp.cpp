#include "shared_array.hpp"
#include <iostream>
#include <unistd.h>
#include <stdexcept>
#include <csignal>
#include <atomic>

constexpr size_t ARRAY_SIZE = 1000;
const std::string ARRAY_NAME = "ipc3_test_array";

std::atomic<bool> keep_running(true);

void signal_handler(int signum) {
    std::cout << "\n[System] Signal (" << signum << ") received. Cleaning up...\n";
    keep_running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {
        SharedArray::unlink_resources(ARRAY_NAME); 
        
        SharedArray arr(ARRAY_NAME, ARRAY_SIZE);
        std::cout << "[FIRST] Writer started. PID: " << getpid() << "\n";
        std::cout << "[INFO] Using shared memory: /dev/shm/" << ARRAY_NAME << "\n";

        int counter = 0;

        while (keep_running) {
            {

                arr.lock();

                size_t index = counter % ARRAY_SIZE;
                arr[index] = counter;

                std::cout << "[FIRST] Writing: arr[" << index << "] = " << counter << "\n" << std::flush;

                arr.unlock();
            }

            counter++;
            sleep(1); 
        }

        SharedArray::unlink_resources(ARRAY_NAME);
        std::cout << "[FIRST] Resources unlinked. Exiting.\n";

    } catch (const std::exception& e) {
        std::cerr << "[FIRST] Fatal Error: " << e.what() << "\n";
        SharedArray::unlink_resources(ARRAY_NAME);
        return 1;
    }

    return 0;
}