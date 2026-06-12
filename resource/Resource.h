#pragma once

#include <string>

namespace oml {

// A physical or logical capability owned by a machine: an axis, a camera, an
// IO line, a light, ... Phase 1 only needs a name — concrete resources are
// defined by the application, not by the runtime model.
class Resource {
public:
    virtual ~Resource() = default;
    virtual std::string Name() const = 0;
};

} // namespace oml
