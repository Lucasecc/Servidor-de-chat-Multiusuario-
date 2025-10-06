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
#include <csignal>
#include <atomic>
#include <chrono>
#include <cerrno>
#include <sys/stat.h>

#include "../include/libtslog.hpp"

using namespace tslog;

struct ClientConnection {
    int socket;
    std::string id;
};

std::vector<ClientConnection> g_clients;
std::mutex g_clients_mutex;
TSLogger logger;
std::atomic<bool> server_running(true);

void signal_handler(int signum) {
    logger.log(Level::WARN, "Sinal de interrupção recebido. Desligando o servidor...");
    server_running = false;
}

void broadcast_message(const std::string& message, int sender_socket) {
    std::lock_guard<std::mutex> lock(g_clients_mutex);
    for (const auto& client : g_clients) {
        if (client.socket != sender_socket) {
            ssize_t s = send(client.socket, message.c_str(), message.length(), 0);
            (void)s;
        }
    }
}

void handle_client(int client_socket) {
    char buffer[4096];
    std::string client_id = "client_" + std::to_string(client_socket);

    {
        std::lock_guard<std::mutex> lock(g_clients_mutex);
        g_clients.push_back({client_socket, client_id});
    }
    logger.log(Level::INFO, "Cliente conectado: " + client_id);

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    while (server_running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_received > 0) {
            std::string received_msg(buffer, bytes_received);
            logger.log(Level::INFO, "Mensagem de " + client_id + ": " + received_msg);
            broadcast_message(client_id + ": " + received_msg, client_socket);
        } else if (bytes_received == 0) {
            logger.log(Level::WARN, "Cliente desconectado (EOF): " + client_id);
            break;
        } else {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            } else {
                logger.log(Level::WARN, "Erro no recv() para " + client_id + ": " + std::string(strerror(errno)));
                break;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(g_clients_mutex);
        g_clients.erase(std::remove_if(g_clients.begin(), g_clients.end(),
                                     [client_socket](const ClientConnection& c) {
                                         return c.socket == client_socket;
                                     }),
                      g_clients.end());
    }
    logger.log(Level::WARN, "Cliente thread finalizando: " + client_id);

    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <porta>" << std::endl;
        return 1;
    }

    signal(SIGINT, signal_handler);

    int port = std::atoi(argv[1]);
    logger.start("server.log");
    logger.log(Level::INFO, "Servidor iniciando na porta " + std::to_string(port) + ". Pressione Ctrl+C para sair.");

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        logger.log(Level::ERROR, "Falha ao criar socket.");
        return 1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        logger.log(Level::ERROR, "Falha ao fazer bind.");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 25) < 0) {
        logger.log(Level::ERROR, "Falha ao escutar.");
        close(server_socket);
        return 1;
    }

    std::vector<std::thread> worker_threads;

    logger.log(Level::INFO, "Servidor escutando...");
    while (server_running) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (!server_running) break;
            continue;
        }

        worker_threads.emplace_back(handle_client, client_socket);
    }

    logger.log(Level::INFO, "Sinal de desligamento recebido. Aguardando todas as threads de cliente finalizarem...");

    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);

    {
        std::lock_guard<std::mutex> lock(g_clients_mutex);
        for (const auto &c : g_clients) {
            shutdown(c.socket, SHUT_RDWR);
        }
    }

    for (auto &t : worker_threads) {
        if (t.joinable()) t.join();
    }

    logger.log(Level::INFO, "Todas as threads de cliente foram finalizadas.");
    logger.log(Level::INFO, "Servidor desligado.");
    logger.stop();

    return 0;
}
