#include "LLGL/TypeInfo.h"
#include "SGE/assert.hpp"
#include <SGE/renderer/dependency_graph.hpp>


void sge::Node::AddChild(const sge::Node* node) const {
    SGE_ASSERT(node != nullptr);
    m_children.insert(node);
    node->m_parents.insert(this);
}

void sge::DependencyGraph::AddNode(LLGL::RenderSystemChild& resource) {
    auto node = m_nodes.find(Node(&resource));
    if (node == m_nodes.end()) {
        node = m_nodes.insert(Node(&resource)).first;
    }
}

void sge::DependencyGraph::AddNode(LLGL::RenderSystemChild& resource, LLGL::RenderSystemChild& dependsOn) {
    auto node = m_nodes.find(Node(&resource));
    if (node == m_nodes.end()) {
        node = m_nodes.insert(Node(&resource)).first;
    }

    auto child = m_nodes.find(Node(&dependsOn));
    if (child == m_nodes.end()) {
        child = m_nodes.insert(Node(&dependsOn)).first;
    }

    node->AddChild(&*child);
}

void sge::DependencyGraph::RemoveNode(LLGL::RenderSystemChild& resource) {
    auto it = m_nodes.find(Node(&resource));
    SGE_ASSERT(it != m_nodes.end());

    it->SetToBeDeleted(true);

    Update();
}

static void Release(const LLGL::RenderSystemPtr& context, LLGL::RenderSystemChild& resource) {
    if (LLGL::BufferArray* r = LLGL::CastTo<LLGL::BufferArray>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::Buffer* r = LLGL::CastTo<LLGL::Buffer>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::Texture* r = LLGL::CastTo<LLGL::Texture>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::SwapChain* r = LLGL::CastTo<LLGL::SwapChain>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::RenderTarget* r = LLGL::CastTo<LLGL::RenderTarget>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::RenderPass* r = LLGL::CastTo<LLGL::RenderPass>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::PipelineLayout* r = LLGL::CastTo<LLGL::PipelineLayout>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::PipelineState* r = LLGL::CastTo<LLGL::PipelineState>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::Shader* r = LLGL::CastTo<LLGL::Shader>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::Sampler* r = LLGL::CastTo<LLGL::Sampler>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::Fence* r = LLGL::CastTo<LLGL::Fence>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::ResourceHeap* r = LLGL::CastTo<LLGL::ResourceHeap>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::PipelineCache* r = LLGL::CastTo<LLGL::PipelineCache>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::QueryHeap* r = LLGL::CastTo<LLGL::QueryHeap>(&resource)) {
        context->Release(*r);
    }
    else if (LLGL::CommandBuffer* r = LLGL::CastTo<LLGL::CommandBuffer>(&resource)) {
        context->Release(*r);
    }
}

void sge::DependencyGraph::Update() {
    SGE_ASSERT(m_context != nullptr);

    (*m_context)->GetCommandQueue()->WaitIdle();

    for (auto it = m_nodes.begin(); it != m_nodes.end();) {
        if (it->IsToBeDeleted() && it->IsOrphan()) {
            Release(*m_context, *it->resource());
            it = m_nodes.erase(it);
        } else {
            ++it;
        }
    }
}

void sge::DependencyGraph::DeleteAll() {
    for (const Node& node : m_nodes) {
        Release(*m_context, *node.resource());
    }
    m_nodes.clear();
}