
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

void error_exit(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char **argv) {
    const size_t buffer_size  = 4096;

    if (argc < 2) {
        printf("Usage: %s <filename", argv[0]);
        return 1;
    }

    int file = open(argv[1], O_RDONLY);
    if (file == -1) {
        error_exit("open");
    }

    char *buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        close(file);
        error_exit("malloc");
    }

    ssize_t bytesread;
    while ((bytesread = read(file, buffer, buffer_size)) > 0) {
        write(1, buffer, bytesread);
    }

    if (bytesread == -1) {
        error_exit("read");
    }

    close(file);
    free(buffer);
    return 0;
}
