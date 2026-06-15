#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <ostream>
#include <string_view>

namespace oml {

// A minimal, thread-safe logger with a redirectable sink.
// 最小化的线程安全日志器，支持可重定向输出目标。
//
// The runtime model logs through this (via the process-wide Log()) instead of
// std::cout directly, so a test can silence the trace and a real device can
// route it to a console, file, or host. The atomic sink gives a lock-free fast
// path when silenced, and writes are mutex-guarded so concurrent loggers (Phase
// 3) produce clean, interleaving-free output.
// 运行模型通过全局 Log() 而非直接 std::cout 输出，测试可屏蔽 trace，
// 真实设备可重定向到控制台/文件/host。原子 sink 在屏蔽时为无锁快路径，
// 写入受互斥保护，并发输出无交错。
class Logger {
public:
    enum class Level { Info, Warn, Error };

    void Info (std::string_view msg) { write(Level::Info,  msg); }
    void Warn (std::string_view msg) { write(Level::Warn,  msg); }
    void Error(std::string_view msg) { write(Level::Error, msg); }

    // Where lines go. Pass nullptr to discard output.
    // 输出目标。传 nullptr 丢弃所有输出。
    void        SetSink(std::ostream* sink);
    std::ostream* Sink() const { return sink_.load(); }

private:
    void write(Level level, std::string_view msg);

    std::atomic<std::ostream*> sink_{&std::cout};
    std::mutex                 mutex_;
};

// Process-wide logger instance.
// 进程级日志器实例。
Logger& Log();

} // namespace oml
