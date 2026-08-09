#ifndef _SGE_TYPES_BINDING_LAYOUT_HPP_
#define _SGE_TYPES_BINDING_LAYOUT_HPP_

#include <initializer_list>
#include <utility>

#include <LLGL/PipelineLayoutFlags.h>

namespace sge {

struct BindingLayoutItem {
    explicit BindingLayoutItem(LLGL::ResourceType resource_type, long bind_flags, long stage, int slot, uint32_t array_size, LLGL::StringLiteral name) :
        name(std::move(name)),
        resource_type(resource_type),
        bind_flags(bind_flags),
        stage(stage),
        slot(slot),
        array_size(array_size) {}

    static BindingLayoutItem TextureStorage(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Texture, LLGL::BindFlags::Storage, stage, slot, 0, std::move(name));
    }

    static BindingLayoutItem Texture(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, stage, slot, 0, std::move(name));
    }

    static BindingLayoutItem StorageBuffer(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Buffer, LLGL::BindFlags::Storage, stage, slot, 0, std::move(name));
    }

    static BindingLayoutItem Buffer(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Buffer, LLGL::BindFlags::Sampled, stage, slot, 0, std::move(name));
    }

    static BindingLayoutItem ConstantBuffer(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, stage, slot, 0, std::move(name));
    }

    static BindingLayoutItem Sampler(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Sampler, 0, stage, slot, 0, std::move(name));
    }

    LLGL::StringLiteral name;
    LLGL::ResourceType resource_type = LLGL::ResourceType::Undefined;
    long bind_flags = 0;
    long stage = -1;
    int slot = -1;
    uint32_t array_size = 0;
};

inline std::vector<LLGL::BindingDescriptor> BindingLayout(long stage, std::initializer_list<BindingLayoutItem> items) {
    std::vector<LLGL::BindingDescriptor> bindings;
    bindings.reserve(items.size());

    uint32_t last_index = 0;
    for (const BindingLayoutItem& item : items) {
        uint32_t index = item.slot >= 0 ? item.slot : last_index;
        long binding_stage = item.stage >= 0 ? item.stage : stage;

        bindings.emplace_back(item.name, item.resource_type, item.bind_flags, binding_stage, index, item.array_size);
        last_index = index + 1;
    }

    return bindings;
}

inline std::vector<LLGL::BindingDescriptor> BindingLayout(std::initializer_list<BindingLayoutItem> items) {
    return BindingLayout(0, items);
}

} // namespace sge

#endif