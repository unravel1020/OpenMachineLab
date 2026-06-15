#include "log/Logger.h"

namespace oml {

void Logger::SetSink(std::ostream* sink) {
    const std::lock_guard<std::mutex> lock(mutex_);
    sink_.store(sink);
}

void Logger::write(Level level, std::string_view msg) {
    // Fast path: a null sink (e.g. during tests) costs no lock.
    // 快路径：null sink（如测试中）无锁开销。
    if (sink_.load() == nullptr) return;

    const std::lock_guard<std::mutex> lock(mutex_);
    std::ostream* sink = sink_.load();
    if (sink == nullptr) return;

    switch (level) {
        case Level::Info:  break;
        case Level::Warn:  *sink << "[warn] ";  break;  // 警告
        case Level::Error: *sink << "[error] "; break;  // 错误
    }
    *sink << msg << '\n';
}

// Meyers singleton — thread-safe initialization.
// Meyers 单例——线程安全初始化。
Logger& Log() {
    static Logger instance;
    return instance;
}

} // namespace oml
