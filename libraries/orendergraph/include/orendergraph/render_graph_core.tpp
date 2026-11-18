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
void RenderGraph::release_resource(ResourceHandle<TResource> handle)
{
    m_registry.release_resource<TResource>(handle);
}

template <typename TResource> 
ResourceHandle<TResource> RenderGraph::import_resource(TResource* resource)
{
    return m_registry.import_resource<TResource>(resource);
}

template <typename TResource> 
bool RenderGraph::replace_resource(ResourceHandle<TResource> handle, TResource* new_resource)
{
    return m_registry.replace_resource(handle, new_resource);
}

template <typename TResource>
bool RenderGraph::replace_resource(ResourceHandle<TResource> handle, typename ResourceTraits<TResource>::DescType desc)
{
    if (!handle.is_valid())
        return false;

    TResource* new_resource = ResourceTraits<TResource>::create(*m_physical_device, *m_device, desc);
    if (!new_resource)
        return false;

    ResourceTraits<TResource>::allocate(new_resource);

    return replace_resource(handle, new_resource);;
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

template <typename TResource>
void RenderGraph::remove_begin_frame_transition(ResourceHandle<TResource> handle)
{
    std::erase_if(m_begin_frame_transitions,
    [&handle](const std::unique_ptr<IResourceAccess>& access)
    {
        return access->resource_id() == handle.id && access->resource_type_idx() == typeid(TResource);
    });
}

template <typename TResource>
void RenderGraph::remove_end_frame_transition(ResourceHandle<TResource> handle)
{
    std::erase_if(m_end_frame_transitions,
    [&handle](const std::unique_ptr<IResourceAccess>& access)
    {
        return access->resource_id() == handle.id && access->resource_type_idx() == typeid(TResource);
    });
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

template <typename TObject>
bool RenderGraph::release_object(TObject* object)
{
    return m_object_cache.release(object);
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
                        typename ResourceTraits<TResource>::DataType data,
                        typename ResourceTraits<TResource>::StateType final_state, bool update_state)
{
    if (!m_active_node)
        return;

    auto access = std::make_unique<ResourceAccess<TResource>>(handle, requested_state, data, final_state, update_state);
    m_active_node->writes().push_back(std::move(access));
}

template <typename TResource>
void RenderGraph::reset_resource_state(ResourceHandle<TResource> handle)
{
    const auto reset = [this, &handle](auto& accesses)
    {
        for (auto& access : accesses)
        {
            if (access->resource_id() == handle.id &&
                access->resource_type_idx() == typeid(TResource))
            {
                access->reset_state(m_registry);
            }
        }
    };

    for (auto& node : m_nodes)
    {
        reset(node->reads());
        reset(node->writes());
    }
}

} // namespace orendergraph

#endif // ORENDERGRAPH_RENDER_GRAPH_CORE_TPP
