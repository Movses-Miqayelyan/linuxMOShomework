#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>

#define READ_END 0
#define WRITE_END 1

bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int calculate_mth_prime(int m) {
    int count = 0, num = 1;
    while (count < m) {
        num++;
        if (is_prime(num)) count++;
    }
    return num;
}

void child_process(int read_fd, int write_fd) {
    int m;
    while (read(read_fd, &m, sizeof(m)) > 0) {
        int result = calculate_mth_prime(m);
        if (write(write_fd, &result, sizeof(result)) == -1) break;
    }
    
    close(read_fd);
    close(write_fd);
    exit(0);
}

void parent_process(int write_fd, int read_fd, pid_t child_pid) {
    std::string input;
    std::cout << "[Parent] Calculator active. Child PID: " << child_pid << "\n";

    while (true) {
        std::cout << "[Parent] Enter m-th prime to find (or 'exit'): " << std::flush;
        if (!std::getline(std::cin, input) || input == "exit") break;

        try {
            int m = std::stoi(input);
            if (m <= 0) {
                std::cout << "Enter a positive number.\n";
                continue;
            }

            write(write_fd, &m, sizeof(m));

            int result;
            if (read(read_fd, &result, sizeof(result)) > 0) {
                std::cout << "[Result] " << m << "-th prime is: " << result << "\n\n";
            } else {
                std::cerr << "[Parent] Child process stopped unexpectedly.\n";
                break;
            }
        } catch (...) {
            std::cout << "Invalid input.\n";
        }
    }

    close(write_fd);
    close(read_fd);

    int status;
    waitpid(child_pid, &status, 0);
    std::cout << "[Parent] Child exited. Goodbye!\n";
}

int main() {
    int p_to_c[2], c_to_p[2];

    if (pipe(p_to_c) == -1 || pipe(c_to_p) == -1) {
        perror("Pipe failed");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }

    if (pid > 0) { 
        close(p_to_c[READ_END]);
        close(c_to_p[WRITE_END]);
        parent_process(p_to_c[WRITE_END], c_to_p[READ_END], pid);
    } else { 
        close(p_to_c[WRITE_END]);
        close(c_to_p[READ_END]);
        child_process(p_to_c[READ_END], c_to_p[WRITE_END]);
    }

    return 0;
}