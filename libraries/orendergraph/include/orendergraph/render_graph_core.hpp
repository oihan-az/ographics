#ifndef ORENDERGRAPH_RENDER_GRAPH_CORE_HPP
#define ORENDERGRAPH_RENDER_GRAPH_CORE_HPP

#include <orendergraph/resource_traits/all.hpp>
#include <orendergraph/object_traits/all.hpp>
#include <orendergraph/object_cache.hpp>

#include <orendergraph/node_context/all.hpp>
#include <orendergraph/node.hpp>

#include <memory>
#include <vector>

namespace orendergraph
{

class RenderGraph
{
  public:
    RenderGraph(ogfx::PhysicalDevice& pd, ogfx::Device& device, ogfx::ResourceAllocator& resource_allocator);
    ~RenderGraph();

    template <typename TResource> ResourceHandle<TResource> create_resource(ResourceTraits<TResource>::DescType desc);
    template <typename TResource> void release_resource(ResourceHandle<TResource> handle);
    template <typename TResource> ResourceHandle<TResource> import_resource(TResource* resource);
    template <typename TResource> bool replace_resource(ResourceHandle<TResource> handle, TResource* new_resource);
    template <typename TResource> bool replace_resource(ResourceHandle<TResource> handle, typename ResourceTraits<TResource>::DescType desc);

    template <typename TResource> TResource* get_resource(ResourceHandle<TResource> handle);

    template <typename TResource>void add_begin_frame_transition(ResourceHandle<TResource> handle, 
                                                                 typename ResourceTraits<TResource>::StateType state, 
                                                                 typename ResourceTraits<TResource>::DataType data = {});
    template <typename TResource> void add_end_frame_transition(ResourceHandle<TResource> handle, 
                                                                typename ResourceTraits<TResource>::StateType state, 
                                                                typename ResourceTraits<TResource>::DataType data = {});

    template <typename TObject> TObject* create_object(const typename ObjectTraits<TObject>::DescType& desc);
    template <typename TObject> TObject* get_or_create_object(const typename ObjectTraits<TObject>::DescType& desc);

    template <typename TContext, typename TSetup> RenderGraphNode<TContext>* add_node(const std::string& name, TSetup&& setup_func);

    template <typename TResource>
    void read(ResourceHandle<TResource> handle, 
              typename ResourceTraits<TResource>::StateType requested_state,
              typename ResourceTraits<TResource>::DataType data = {});

    template <typename TResource>
    void write(ResourceHandle<TResource> handle, 
               typename ResourceTraits<TResource>::StateType requested_state = {},
               typename ResourceTraits<TResource>::DataType data = {},
               typename ResourceTraits<TResource>::StateType final_state = {}, bool update_state = false);

    void compile();
    void execute(ogfx::CommandBuffer& cmd);
    void reset();

    ogfx::Device& device() { return *m_device; }
    ogfx::ResourceAllocator& get_resource_allocator() { return *m_resource_allocator; }

  private:
    ogfx::PhysicalDevice* m_physical_device = nullptr;
    ogfx::Device* m_device = nullptr;
    ogfx::ResourceAllocator* m_resource_allocator = nullptr;

    ObjectCache m_object_cache;
    ResourcePoolRegistry m_registry;
    std::vector<std::unique_ptr<IResourceAccess>> m_begin_frame_transitions;
    std::vector<std::unique_ptr<IResourceAccess>> m_end_frame_transitions;

    std::vector<std::unique_ptr<IRenderGraphNode>> m_nodes;
    IRenderGraphNode* m_active_node = nullptr;

    struct NodeDependencies
    {
        std::vector<uint32_t> dependencies;
        std::vector<uint32_t> dependents;
    };
    std::vector<NodeDependencies> m_dependencies;
    void add_dependency(uint32_t from, uint32_t to);
    void build_dependencies();

    std::vector<IRenderGraphNode*> m_execution_order;
    void build_execution_order();
};

} // namespace orendergraph

#include <orendergraph/render_graph_core.tpp>

#endif // ORENDERGRAPH_RENDER_GRAPH_CORE_HPP