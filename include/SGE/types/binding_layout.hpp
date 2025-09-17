#ifndef _SGE_TYPES_BINDING_LAYOUT_HPP_
#define _SGE_TYPES_BINDING_LAYOUT_HPP_

#include <initializer_list>

#include <LLGL/PipelineLayoutFlags.h>

namespace sge {

class BindingLayout;

class BindingLayoutItem {
    friend class BindingLayout;

public:

    static BindingLayoutItem TextureStorage(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Texture, LLGL::BindFlags::Storage, stage, slot, 0, name);
    }

    static BindingLayoutItem Texture(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled, stage, slot, 0, name);
    }

    static BindingLayoutItem StorageBuffer(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Buffer, LLGL::BindFlags::Storage, stage, slot, 0, name);
    }

    static BindingLayoutItem Buffer(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Buffer, LLGL::BindFlags::Sampled, stage, slot, 0, name);
    }

    static BindingLayoutItem ConstantBuffer(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, stage, slot, 0, name);
    }

    static BindingLayoutItem Sampler(int slot = -1, LLGL::StringLiteral name = {}, long stage = -1) {
        return BindingLayoutItem(LLGL::ResourceType::Sampler, 0, stage, slot, 0, name);
    }

private:
    BindingLayoutItem(LLGL::ResourceType resource_type, long bind_flags, long stage, int slot, uint32_t array_size, LLGL::StringLiteral name) :
        name(std::move(name)),
        resource_type(resource_type),
        bind_flags(bind_flags),
        stage(stage),
        slot(slot),
        array_size(array_size) {}

    LLGL::StringLiteral name;
    LLGL::ResourceType resource_type = LLGL::ResourceType::Undefined;
    long bind_flags = 0;
    long stage = -1;
    int slot = -1;
    uint32_t array_size = 0;
};

class BindingLayout {
public:
    BindingLayout(std::initializer_list<BindingLayoutItem> items) : m_items(items) {}
    BindingLayout(long stage, std::initializer_list<BindingLayoutItem> items) :
        m_items(items),
        m_stage(stage) {}

    inline void AddItem(BindingLayoutItem item) {
        m_items.push_back(item);
    }

    std::vector<LLGL::BindingDescriptor> ToLLGL() {
        std::vector<LLGL::BindingDescriptor> items;
        items.reserve(m_items.size());

        uint32_t last_index = 0;
        for (const BindingLayoutItem& item : m_items) {
            uint32_t index = item.slot >= 0 ? item.slot : last_index;
            long stage = item.stage >= 0 ? item.stage : m_stage;

            items.emplace_back(item.name, item.resource_type, item.bind_flags, stage, index, item.array_size);
            last_index = index + 1;
        }

        return items;
    }

    operator std::vector<LLGL::BindingDescriptor>() {
        return ToLLGL();
    }

private:
    std::vector<BindingLayoutItem> m_items;
    long m_stage = 0;
};

}

#endif