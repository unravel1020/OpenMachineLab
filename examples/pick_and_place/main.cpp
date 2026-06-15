// pick_and_place - runs the PickAndPlace device profile.
// pick_and_place - 运行 PickAndPlace 设备 profile。
//
// Same main shape as minimal_machine: create a device, drive its lifecycle, stop
// on stdin. The device-specifics (resources, modules, recipe) live in the
// profile; only the profile differs between this and the die bonder.
// 与 minimal_machine 相同的 main 结构：创建设备、驱动生命周期、stdin 停止。
// 设备相关内容（资源、模块、配方）在 profile 中；
// 与固晶机的区别仅在于 profile 不同。
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
