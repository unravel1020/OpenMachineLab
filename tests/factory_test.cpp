// factory_test - DeviceFactory: register a type, create by name + config, and
// reject unknown types.
#include "oml_test.h"

#include "config/DeviceConfig.h"
#include "device/Device.h"
#include "device/DeviceFactory.h"

#include <memory>
#include <string>

using namespace oml;
using namespace oml::test;

namespace {

// A trivial device that remembers one config value, to prove config flows
// through the factory.
class TestDevice : public Device {
public:
    explicit TestDevice(const DeviceConfig& cfg = {}) : Device("TestDevice"),
        value_(cfg.GetLong("value", 0)) {}

    long Value() const { return value_; }

private:
    long value_ = 0;
};

} // namespace

class FactoryTest : public TestBase {
public:
    std::string Name() const override { return "factory"; }

    void Run() override {
        DeviceFactory factory;
        factory.Register("TestDevice",
                         [](const DeviceConfig& c) { return std::make_unique<TestDevice>(c); });

        Invariant(factory.Knows("TestDevice"), "registered type is known");
        Invariant(!factory.Knows("Nope"), "unregistered type is unknown");

        // Create by name with a config; the config reaches the device.
        DeviceConfig cfg;
        cfg.Set("value", "42");
        std::unique_ptr<Device> d = factory.Create("TestDevice", cfg);

        Invariant(d != nullptr, "Create returns a device for a known type");
        Invariant(d->Name() == "TestDevice", "created device has the right name");
        Invariant(static_cast<TestDevice*>(d.get())->Value() == 42,
                  "config flows through the factory into the device");

        // Default config (no arg) works too.
        std::unique_ptr<Device> d2 = factory.Create("TestDevice");
        Invariant(d2 != nullptr && static_cast<TestDevice*>(d2.get())->Value() == 0,
                  "Create with default config uses defaults");

        // Unknown type yields nullptr (no throw, no partial device).
        Invariant(factory.Create("Unknown", {}) == nullptr, "unknown type -> nullptr");
    }
};

int main() {
    return RunAll(std::make_unique<FactoryTest>());
}
