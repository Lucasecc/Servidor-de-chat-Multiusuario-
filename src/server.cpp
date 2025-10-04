// src/server.cpp
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/libtslog.hpp"

using namespace tslog;

struct ClientConnection {
    int socket;
    std::string id;
};

std::vector<ClientConnection> clients;
std::mutex clients_mutex;
TSLogger logger;

void broadcast_message(const std::string& message, int sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        if (client.socket != sender_socket) {
            send(client.socket, message.c_str(), message.length(), 0);
        }
    }
}

void handle_client(int client_socket) {
    char buffer[4096];
    std::string client_id = "client_" + std::to_string(client_socket);

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back({client_socket, client_id});
    }

    logger.log(Level::INFO, "Cliente conectado: " + client_id);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received <= 0) {
            logger.log(Level::WARN, "Cliente desconectado: " + client_id);
            break;
        }

        std::string received_msg = std::string(buffer, bytes_received);
        logger.log(Level::INFO, "Mensagem de " + client_id + ": " + received_msg);

        std::string broadcast_msg = client_id + ": " + received_msg;
        broadcast_message(broadcast_msg, client_socket);
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     [client_socket](const ClientConnection& client) {
                                         return client.socket == client_socket;
                                     }),
                      clients.end());
    }

    close(client_socket);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <porta>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string logfile = "server.log";

    logger.start(logfile);
    logger.log(Level::INFO, "Servidor iniciando na porta " + std::to_string(port));

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        logger.log(Level::ERROR, "Falha ao criar o socket do servidor.");
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        logger.log(Level::ERROR, "Falha ao fazer bind na porta " + std::to_string(port));
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 10) < 0) {
        logger.log(Level::ERROR, "Falha ao escutar por conexões.");
        close(server_socket);
        return 1;
    }

    logger.log(Level::INFO, "Servidor escutando. Aguardando conexões...");

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

        if (client_socket < 0) {
            logger.log(Level::WARN, "Falha ao aceitar conexão de cliente.");
            continue;
        }

        std::thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    close(server_socket);
    logger.stop();
    return 0;
}
