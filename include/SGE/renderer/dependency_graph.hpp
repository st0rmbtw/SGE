#ifndef SGE_RENDERER_DEPENDENCY_GRAPH_HPP_
#define SGE_RENDERER_DEPENDENCY_GRAPH_HPP_

#include <LLGL/RenderSystem.h>
#include <LLGL/Resource.h>
#include <queue>
#include <SGE/assert.hpp>
#include <unordered_set>
#include <utility>

namespace sge {

class Node {
public:
    using NodeSet = std::unordered_set<const Node*>;

public:
    Node() = default;
    explicit Node(LLGL::RenderSystemChild* resource) : m_resource(resource) {}

    Node& operator=(const Node&) = delete;
    Node& operator=(Node&) = delete;
    Node(Node&) = delete;
    Node(const Node&) = delete;

    Node& operator=(Node&&) = default;
    Node(Node&&) noexcept = default;

    void AddChild(const Node* node) const;

    void SetToBeDeleted(bool to_be_deleted) const {
        m_to_be_deleted = to_be_deleted;
    }

    [[nodiscard]]
    bool IsOrphan() const {
        return m_parents.empty();
    }

    [[nodiscard]]
    bool IsToBeDeleted() const {
        return m_to_be_deleted;
    }

    [[nodiscard]]
    LLGL::RenderSystemChild* resource() const {
        return m_resource;
    }

    [[nodiscard]]
    const NodeSet& parents() const {
        return m_parents;
    }
    
    [[nodiscard]]
    const NodeSet& children() const {
        return m_children;
    }

    inline constexpr bool operator==(const Node& other) const noexcept {
        return resource() == other.resource();
    }
    
private:
    mutable NodeSet m_children;
    mutable NodeSet m_parents;
    LLGL::RenderSystemChild* m_resource = nullptr;
    mutable bool m_to_be_deleted = false;
};

} // namespace sge

template <>
struct std::hash<sge::Node> {
    static size_t operator()(const sge::Node& node) {
        return std::hash<LLGL::RenderSystemChild*>{}(node.resource());
    }
};

namespace sge {

class DependencyGraph {

public:
    DependencyGraph() = default;

    DependencyGraph(const LLGL::RenderSystemPtr* render_context) :
        m_context(render_context)
    {
    }

    void AddNode(LLGL::RenderSystemChild& resource);
    void AddNode(LLGL::RenderSystemChild& resource, LLGL::RenderSystemChild& dependensOn);
    void RemoveNode(LLGL::RenderSystemChild& resource);

    void Update();

    void DeleteAll();

    template <typename TParent>
    requires (std::derived_from<TParent, LLGL::RenderSystemChild>)
    inline constexpr void AddNode(TParent& resource) {
        AddNode(static_cast<LLGL::RenderSystemChild&>(resource));
    }

    template <typename TParent, typename TChild>
    requires (std::derived_from<TParent, LLGL::RenderSystemChild> && std::derived_from<TChild, LLGL::RenderSystemChild>)
    inline constexpr void AddNode(TParent& resource, TChild& dependensOn) {
        AddNode(static_cast<LLGL::RenderSystemChild&>(resource), static_cast<LLGL::RenderSystemChild&>(dependensOn));
    }

private:
    template <typename F>
    void TraverseChildren(const Node& node, F&& callback) {
        m_visited.clear();
        m_node_queue = {};

        m_node_queue.push(&node);

        while (!m_node_queue.empty()) {
            for (auto node : m_node_queue.front()->children()) {
                if (m_visited.find(node) == m_visited.end()) {
                    std::forward<F>(callback)(*node);
                    m_node_queue.push(node);
                }
            }

            m_node_queue.pop();
        }
    }

private:
    std::unordered_set<Node> m_nodes;
    std::unordered_set<const Node*> m_visited;
    std::queue<const Node*> m_node_queue;
    const LLGL::RenderSystemPtr* m_context = nullptr;
};

} // namespace sge

#endif // SGE_RENDERER_DEPENDENCY_GRAPH_HPP_