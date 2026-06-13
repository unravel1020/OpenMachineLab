// pick_and_place - runs the PickAndPlace device profile.
//
// Same main shape as minimal_machine: create a device, drive its lifecycle, stop
// on stdin. The device-specifics (resources, modules, recipe) live in the
// profile; only the profile differs between this and the die bonder.
#include "PickAndPlace.h"
#include "log/Logger.h"

#include <iostream>
#include <string>
#include <thread>

using namespace oml::example;

int main() {
    PickAndPlace device;

    oml::Log().Info("\n>>> press <Enter> to stop the " + device.Name() + " <<<");
    std::thread stopper([&device] {
        std::string line;
        std::getline(std::cin, line);
        device.Stop();
    });

    device.Initialize();
    device.Run();
    stopper.join();
    device.Shutdown();

    return 0;
}
