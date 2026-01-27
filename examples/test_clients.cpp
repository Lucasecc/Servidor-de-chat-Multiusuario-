#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <cstring>
#include <cstdlib>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <sys/stat.h>

#include "../include/libtslog.hpp"

using namespace tslog;

void simulate_client(int id, int messages_to_send, const char* server_ip, int port, TSLogger& logger) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        logger.log(Level::ERROR, "Cliente " + std::to_string(id) + " falhou ao criar socket.");
        return;
    }
    logger.log(Level::INFO, "Cliente " + std::to_string(id) + ": Socket criado com sucesso.");

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    logger.log(Level::INFO, "Cliente " + std::to_string(id) + ": Tentando conectar ao servidor...");
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        logger.log(Level::ERROR, "Cliente " + std::to_string(id) + " falhou ao conectar.");
        close(sock);
        return;
    }
    logger.log(Level::INFO, "Cliente " + std::to_string(id) + ": Conectado com sucesso!");

    for (int i = 0; i < messages_to_send; ++i) {
        std::string msg = "Cliente " + std::to_string(id) + " envia a mensagem #" + std::to_string(i);
        send(sock, msg.c_str(), msg.length(), 0);
        logger.log(Level::INFO, "Cliente " + std::to_string(id) + ": Enviando mensagem #" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    close(sock);
    logger.log(Level::INFO, "Cliente " + std::to_string(id) + ": Conexao Fechada.");
    std::cout << "Cliente " << id << " concluiu o teste." << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Uso: " << argv[0] << " <ip_servidor> <porta> <num_clientes> <msgs_por_cliente>" << std::endl;
        return 1;
    }

    mkdir("logs", 0777);

    TSLogger logger;
    logger.start("logs/test_clients.log");

    const char* server_ip = argv[1];
    int port = std::atoi(argv[2]);
    int num_clients = std::atoi(argv[3]);
    int msgs_per_client = std::atoi(argv[4]);

    std::vector<std::thread> client_threads;

    std::cout << "Iniciando teste com " << num_clients << " clientes..." << std::endl;
    logger.log(Level::INFO, "Iniciando simulacao com " + std::to_string(num_clients) + " clientes.");

    for (int i = 0; i < num_clients; ++i) {
        client_threads.emplace_back(simulate_client, i, msgs_per_client, server_ip, port, std::ref(logger));
    }

    for (auto& t : client_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "Teste concluído." << std::endl;
    logger.log(Level::INFO, "Simulacao concluida.");
    logger.stop();
    return 0;
}
