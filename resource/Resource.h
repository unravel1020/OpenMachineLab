#pragma once

#include <string>

namespace oml {

// A physical or logical capability owned by a machine: an axis, a camera, an
// IO line, a light, ... Phase 1 only needs a name — concrete resources are
// defined by the application, not by the runtime model.
// 机器拥有的物理或逻辑能力：轴、相机、IO 线、光源……第一阶段只需要一个名字——
// 具体资源由应用代码定义，不由运行模型定义。
class Resource {
public:
    virtual ~Resource() = default;
    virtual std::string Name() const = 0;
};

} // namespace oml
