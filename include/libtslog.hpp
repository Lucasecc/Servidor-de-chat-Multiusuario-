#ifndef LIBTSLOG_HPP
#define LIBTSLOG_HPP

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <fstream>
#include <chrono>

namespace tslog {

enum class Level { DEBUG, INFO, WARN, ERROR };

class TSLogger {
public:
    TSLogger(size_t max_queue_size = 1000);
    ~TSLogger();

    TSLogger(const TSLogger&) = delete;
    TSLogger& operator=(const TSLogger&) = delete;

    void start(const std::string& filename);
    void stop();
    void log(Level level, const std::string& msg);

    static std::string level_to_string(Level l);

private:
    void worker_loop();
    std::string format_message(Level level, const std::string& msg);

    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::string> queue_;
    size_t max_queue_size_;

    std::thread worker_;
    std::atomic<bool> running_;
    std::ofstream out_;
};

} // namespace tslog

#endif // LIBTSLOG_HPP
