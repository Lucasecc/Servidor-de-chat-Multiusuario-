#include <iostream>
#include <string>
#include <thread>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "../include/libtslog.hpp"

using namespace tslog;

void receive_messages(int client_socket) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cout << "Desconectado do servidor." << std::endl;
            break;
        }
        std::cout << std::string(buffer, bytes_received) << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0] << " <ip_servidor> <porta>" << std::endl;
        return 1;
    }
    
    mkdir("logs", 0777);

    TSLogger logger;
    logger.start("logs/client_chat.log");

    const char* server_ip = argv[1];
    int port = std::atoi(argv[2]);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "Falha ao criar o socket." << std::endl;
        logger.log(Level::ERROR, "Falha ao criar o socket.");
        logger.stop();
        return 1;
    }
    logger.log(Level::INFO, "Socket do cliente criado com sucesso.");

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    logger.log(Level::INFO, "Tentando conectar ao servidor em " + std::string(server_ip) + ":" + std::to_string(port) + "...");
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Falha ao conectar ao servidor." << std::endl;
        logger.log(Level::ERROR, "Falha ao conectar ao servidor.");
        close(client_socket);
        logger.stop();
        return 1;
    }
    logger.log(Level::INFO, "Conectado ao servidor com sucesso!");

    std::cout << "Conectado ao servidor. Você pode começar a enviar mensagens." << std::endl;

    std::thread receiver_thread(receive_messages, client_socket);
    receiver_thread.detach();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "/quit") break;
        send(client_socket, line.c_str(), line.length(), 0);
        logger.log(Level::INFO, "Enviando mensagem: " + line);
    }

    close(client_socket);
    logger.log(Level::INFO, "Conexao Fechada.");
    logger.stop();
    return 0;
}
