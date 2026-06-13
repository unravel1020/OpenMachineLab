#include "log/Logger.h"

namespace oml {

void Logger::SetSink(std::ostream* sink) {
    const std::lock_guard<std::mutex> lock(mutex_);
    sink_.store(sink);
}

void Logger::write(Level level, std::string_view msg) {
    // Fast path: a null sink (e.g. during tests) costs no lock.
    if (sink_.load() == nullptr) return;

    const std::lock_guard<std::mutex> lock(mutex_);
    std::ostream* sink = sink_.load();
    if (sink == nullptr) return;

    switch (level) {
        case Level::Info:  break;
        case Level::Warn:  *sink << "[warn] ";  break;
        case Level::Error: *sink << "[error] "; break;
    }
    *sink << msg << '\n';
}

Logger& Log() {
    static Logger instance;
    return instance;
}

} // namespace oml
