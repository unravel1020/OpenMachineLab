// minimal_machine - runs the DieBonder device profile.
// minimal_machine - 运行 DieBonder 设备 profile。
//
// main() only creates a device and drives its lifecycle. All composition lives
// in the device profile (DieBonder); the generic model and reusable parts live
// in the library. This is the template every concrete device's main follows.
// main() 只创建设备并驱动其生命周期。所有组装逻辑在设备 profile（DieBonder）中；
// 通用模型和可复用部件在库里。这是每个具体设备 main 的模板。
#include "DieBonder.h"
#include "log/Logger.h"

#include <iostream>
#include <string>
#include <thread>

using namespace oml::example;

int main() {
    DieBonder device;

    oml::Log().Info("\n>>> press <Enter> to stop the " + device.Name() + " <<<");
    // Operator stop: a background thread reads stdin and requests Stop.
    // 操作员停止：后台线程读 stdin 并请求 Stop。
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
