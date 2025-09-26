#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>

#include "../include/libtslog.hpp"

using namespace tslog;

int main(int argc, char** argv) {
    int nthreads = 8;
    int messages_per_thread = 100;
    std::string logfile = "test.log";

    if (argc >= 2) nthreads = std::atoi(argv[1]);
    if (argc >= 3) messages_per_thread = std::atoi(argv[2]);
    if (argc >= 4) logfile = argv[3];

    TSLogger logger(5000);
    try {
        logger.start(logfile);
    } catch (const std::exception& e) {
        std::cerr << "Erro ao iniciar logger: " << e.what() << std::endl;
        return 1;
    }

    std::vector<std::thread> producers;
    for (int i = 0; i < nthreads; ++i) {
        producers.emplace_back([i, messages_per_thread, &logger]() {
            for (int m = 0; m < messages_per_thread; ++m) {
                logger.log(Level::INFO, "thread " + std::to_string(i) + " message " + std::to_string(m));
                if ((m % 100) == 0) std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        });
    }

    for (auto &t : producers) if (t.joinable()) t.join();

    logger.stop();
    std::cout << "Teste concluído. Log em: " << logfile << std::endl;
    return 0;
}
