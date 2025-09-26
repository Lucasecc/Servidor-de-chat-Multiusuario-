#include "../include/libtslog.hpp"
#include <sstream>
#include <iomanip>

namespace tslog {

TSLogger::TSLogger(size_t max_queue_size): max_queue_size_(max_queue_size), running_(false) {}

TSLogger::~TSLogger() {
    stop();
}

void TSLogger::start(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (running_) return;
    out_.open(filename, std::ios::out | std::ios::app);
    if (!out_.is_open()) throw std::runtime_error("TSLogger: failed to open log file");
    running_ = true;
    worker_ = std::thread(&TSLogger::worker_loop, this);
}

void TSLogger::stop() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!running_) return;
        running_ = false;
    }
    cv_.notify_one();
    if (worker_.joinable()) worker_.join();
    if (out_.is_open()) out_.close();
}

void TSLogger::log(Level level, const std::string& msg) {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]{ return queue_.size() < max_queue_size_; });

    queue_.push(format_message(level, msg));
    lock.unlock();
    cv_.notify_one();
}

std::string TSLogger::format_message(Level level, const std::string& msg) {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_MSC_VER)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
       << " [" << level_to_string(level) << "] "
       << msg;
    return ss.str();
}

void TSLogger::worker_loop() {
    std::unique_lock<std::mutex> lock(mtx_);
    while (running_ || !queue_.empty()) {
        cv_.wait(lock, [this]{ return !running_ || !queue_.empty(); });
        while (!queue_.empty()) {
            std::string line = std::move(queue_.front());
            queue_.pop();
            lock.unlock();
            if (out_.is_open()) {
                out_ << line << '\n';
                out_.flush();
            }
            lock.lock();
            cv_.notify_all();
        }
    }
}

std::string TSLogger::level_to_string(Level l) {
    switch (l) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO: return "INFO";
        case Level::WARN: return "WARN";
        case Level::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace tslog
