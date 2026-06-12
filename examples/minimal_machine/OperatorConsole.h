#pragma once

#include "machine/Machine.h"

#include <string>
#include <thread>

namespace oml::example {

// The operator console: a background thread that blocks on stdin until a line
// arrives (press Enter), then requests the machine to stop. This is how an
// external exit command reaches the run loop. Host/operator specific, so it
// lives in the example.
class OperatorConsole {
public:
    explicit OperatorConsole(Machine& machine)
        : machine_(machine), thread_([this] { AwaitStop(); }) {}

    // Block until the operator has issued the stop command.
    void Wait() {
        if (thread_.joinable()) thread_.join();
    }

    ~OperatorConsole() { Wait(); }

private:
    void AwaitStop() {
        std::string line;
        std::getline(std::cin, line);
        machine_.Stop();
    }

    Machine&  machine_;
    std::thread thread_;
};

} // namespace oml::example
