// examples/test_clients.cpp
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

void simulate_client(int id, int messages_to_send, const char* server_ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return;
    }

    for (int i = 0; i < messages_to_send; ++i) {
        std::string msg = "Cliente " + std::to_string(id) + " envia a mensagem #" + std::to_string(i);
        send(sock, msg.c_str(), msg.length(), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    close(sock);
    std::cout << "Cliente " << id << " concluiu o teste." << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Uso: " << argv[0] << " <ip_servidor> <porta> <num_clientes> <msgs_por_cliente>" << std::endl;
        return 1;
    }

    const char* server_ip = argv[1];
    int port = std::atoi(argv[2]);
    int num_clients = std::atoi(argv[3]);
    int msgs_per_client = std::atoi(argv[4]);

    std::vector<std::thread> client_threads;

    std::cout << "Iniciando teste com " << num_clients << " clientes..." << std::endl;

    for (int i = 0; i < num_clients; ++i) {
        client_threads.emplace_back(simulate_client, i, msgs_per_client, server_ip, port);
    }

    for (auto& t : client_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "Teste concluído." << std::endl;
    return 0;
}
