#ifndef SHARED_ARRAY_HPP
#define SHARED_ARRAY_HPP

#include <string>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <cerrno>

class SharedArray {
private:
    std::string name_;
    size_t size_;
    int shm_fd_ = -1;
    int* data_ = nullptr;
    sem_t* semaphore_ = SEM_FAILED;

    static constexpr mode_t SHM_PERMS = 0666; 
    static constexpr mode_t SEM_PERMS = 0666;
    static constexpr int INITIAL_SEM_VALUE = 1;

    void cleanup() {
        if (data_ != MAP_FAILED && data_ != nullptr) {
            munmap(data_, size_ * sizeof(int));
            data_ = nullptr;
        }
        if (semaphore_ != SEM_FAILED && semaphore_ != nullptr) {
            sem_close(semaphore_);
            semaphore_ = SEM_FAILED;
        }
        if (shm_fd_ != -1) {
            close(shm_fd_);
            shm_fd_ = -1;
        }
    }

public:
    SharedArray(const std::string& name, size_t size) : name_(name), size_(size) {
        if (size_ == 0 || size_ > 1000000000) {
            throw std::out_of_range("Invalid array size.");
        }

        std::string shm_name = (name_[0] == '/') ? name_ + "_shm" : "/" + name_ + "_shm";
        std::string sem_name = (name_[0] == '/') ? name_ + "_sem" : "/" + name_ + "_sem";
        size_t total_size = size_ * sizeof(int);

        shm_fd_ = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, SHM_PERMS);
        if (shm_fd_ == -1) {
            throw std::runtime_error("shm_open failed: " + std::string(strerror(errno)));
        }

        struct stat shm_stat;
        if (fstat(shm_fd_, &shm_stat) == -1) {
            cleanup();
            throw std::runtime_error("fstat failed.");
        }

        if (shm_stat.st_size == 0) {
            if (ftruncate(shm_fd_, total_size) == -1) {
                cleanup();
                throw std::runtime_error("ftruncate failed.");
            }
        }

        data_ = static_cast<int*>(mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0));
        if (data_ == MAP_FAILED) {
            cleanup();
            throw std::runtime_error("mmap failed.");
        }

        semaphore_ = sem_open(sem_name.c_str(), O_CREAT, SEM_PERMS, INITIAL_SEM_VALUE);
        if (semaphore_ == SEM_FAILED) {
            cleanup();
            throw std::runtime_error("sem_open failed: " + std::string(strerror(errno)));
        }
    }

    ~SharedArray() {
        cleanup();
    }

    SharedArray(const SharedArray&) = delete;
    SharedArray& operator=(const SharedArray&) = delete;

    int& operator[](size_t index) {
        if (index >= size_) throw std::out_of_range("Index out of bounds.");
        return data_[index];
    }

    int operator[](size_t index) const {
        if (index >= size_) throw std::out_of_range("Index out of bounds.");
        return data_[index];
    }

    void lock() {
        while (sem_wait(semaphore_) == -1) {
            if (errno != EINTR) throw std::runtime_error("sem_wait failed.");
        }
    }

    void unlock() {
        if (sem_post(semaphore_) == -1) {
            throw std::runtime_error("sem_post failed.");
        }
    }

    static void unlink_resources(const std::string& name) {
        std::string shm_name = (name[0] == '/') ? name + "_shm" : "/" + name + "_shm";
        std::string sem_name = (name[0] == '/') ? name + "_sem" : "/" + name + "_sem";
        shm_unlink(shm_name.c_str());
        sem_unlink(sem_name.c_str());
    }
};

#endif