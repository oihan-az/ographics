#ifndef ORENDERGRAPH_RENDER_GRAPH_CORE_TPP
#define ORENDERGRAPH_RENDER_GRAPH_CORE_TPP

#include <stdexcept>
#include <utility>

namespace orendergraph
{

template <typename TResource> 
ResourceHandle<TResource> RenderGraph::create_resource(ResourceTraits<TResource>::DescType desc)
{
    return m_registry.create_resource<TResource>(*m_physical_device, *m_device, desc);
}

template <typename TResource> 
ResourceHandle<TResource> RenderGraph::import_resource(TResource* resource)
{
    return m_registry.import_resource<TResource>(resource);
}

template <typename TResource> 
void RenderGraph::replace_resource(ResourceHandle<TResource> handle, TResource* new_resource)
{
    auto* pool = m_registry.get_pool<TResource>();
    pool->replace_resource(handle, new_resource);
}

template <typename TResource> 
TResource* RenderGraph::get_resource(ResourceHandle<TResource> handle)
{
    return m_registry.get_resource<TResource>(handle);
}

template <typename TResource>
void RenderGraph::add_begin_frame_transition(ResourceHandle<TResource> handle, 
                                             typename ResourceTraits<TResource>::StateType state,
                                             typename ResourceTraits<TResource>::DataType data)
{
    m_begin_frame_transitions.push_back(std::make_unique<ResourceAccess<TResource>>(handle, state, data));
}

template <typename TResource>
void RenderGraph::add_end_frame_transition(ResourceHandle<TResource> handle, 
                                           typename ResourceTraits<TResource>::StateType state,
                                           typename ResourceTraits<TResource>::DataType data)
{
    m_end_frame_transitions.push_back(std::make_unique<ResourceAccess<TResource>>(handle, state, data));
}

template <typename TObject> 
TObject* RenderGraph::create_object(const typename ObjectTraits<TObject>::DescType& desc)
{
    return m_object_cache.create<TObject>(desc);
}

template <typename TObject>
TObject* RenderGraph::get_or_create_object(const typename ObjectTraits<TObject>::DescType& desc)
{
    return m_object_cache.get_or_create<TObject>(desc);
}

template <typename TContext, typename TSetup>
RenderGraphNode<TContext>* RenderGraph::add_node(const std::string& name, TSetup&& setup_func)
{
    auto node = std::make_unique<RenderGraphNode<TContext>>(name);

    m_active_node = node.get();
    setup_func(*this, node->descriptor());
    node->compile(*this);
    m_active_node = nullptr;

    auto* ptr = node.get();
    m_nodes.push_back(std::move(node));

    return ptr;
}

template <typename TResource>
void RenderGraph::read(ResourceHandle<TResource> handle, 
                       typename ResourceTraits<TResource>::StateType requested_state,
                       typename ResourceTraits<TResource>::DataType data)
{
    if (!m_active_node)
        return;

    auto access = std::make_unique<ResourceAccess<TResource>>(handle, requested_state, data);
    m_active_node->reads().push_back(std::move(access));
}

template <typename TResource>
void RenderGraph::write(ResourceHandle<TResource> handle, 
                        typename ResourceTraits<TResource>::StateType requested_state,
                        typename ResourceTraits<TResource>::DataType data)
{
    if (!m_active_node)
        return;

    auto access = std::make_unique<ResourceAccess<TResource>>(handle, requested_state, data);
    m_active_node->writes().push_back(std::move(access));
}

} // namespace orendergraph

#endif // ORENDERGRAPH_RENDER_GRAPH_CORE_TPP
