#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string_view>

namespace oml {

// A minimal, thread-safe logger with a redirectable sink.
//
// The runtime model logs through this (via the process-wide Log()) instead of
// std::cout directly, so a test can silence the trace and a real device can
// route it to a console, file, or host. The atomic sink gives a lock-free fast
// path when silenced, and writes are mutex-guarded so concurrent loggers (Phase
// 3) produce clean, interleaving-free output.
class Logger {
public:
    enum class Level { Info, Warn, Error };

    void Info (std::string_view msg) { write(Level::Info,  msg); }
    void Warn (std::string_view msg) { write(Level::Warn,  msg); }
    void Error(std::string_view msg) { write(Level::Error, msg); }

    // Where lines go. Pass nullptr to discard output.
    void        SetSink(std::ostream* sink);
    std::ostream* Sink() const { return sink_.load(); }

private:
    void write(Level level, std::string_view msg);

    std::atomic<std::ostream*> sink_{&std::cout};
    std::mutex                 mutex_;
};

// Process-wide logger instance.
Logger& Log();

} // namespace oml
