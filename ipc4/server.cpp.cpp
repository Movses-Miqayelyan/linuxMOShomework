#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <algorithm>

struct Client {
    int socket;
    std::string name;
};

std::vector<Client> clients;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast(const std::string& message, int sender_fd) {
    pthread_mutex_lock(&clients_mutex);
    for (const auto& client : clients) {
        if (client.socket != sender_fd) {
            send(client.socket, message.c_str(), message.size(), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void* handle_client(void* arg) {
    int client_fd = *static_cast<int*>(arg);
    delete static_cast<int*>(arg);
    
    char buffer[1024];
    
    ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        close(client_fd);
        return nullptr;
    }
    buffer[n] = '\0';
    
    std::string client_name(buffer);
    client_name.erase(std::remove(client_name.begin(), client_name.end(), '\n'), client_name.end());
    client_name.erase(std::remove(client_name.begin(), client_name.end(), '\r'), client_name.end());

    if (client_name.empty()) client_name = "Anonymous";

    pthread_mutex_lock(&clients_mutex);
    clients.push_back({client_fd, client_name});
    pthread_mutex_unlock(&clients_mutex);

    std::string welcome = ">>> Server: " + client_name + " joined the chat.\n";
    std::cout << welcome << std::flush;
    broadcast(welcome, client_fd);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (n <= 0) break; 

        std::string msg(buffer);
        
        if (msg.find("/list") == 0) {
            std::string list = ">>> Server Active users: ";
            pthread_mutex_lock(&clients_mutex);
            for (const auto& c : clients) list += c.name + " ";
            pthread_mutex_unlock(&clients_mutex);
            list += "\n";
            send(client_fd, list.c_str(), list.size(), 0);
        }