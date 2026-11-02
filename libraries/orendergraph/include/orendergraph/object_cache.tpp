#ifndef ORENDERGRAPH_OBJECT_CACHE_TPP
#define ORENDERGRAPH_OBJECT_CACHE_TPP

namespace orendergraph
{

template <typename TObject>
ObjectCacheEntry<TObject>::ObjectCacheEntry(std::unique_ptr<TObject> object, const DescType& desc)
    : m_desc(desc), m_object(std::move(object))
{
}

template <typename TObject>
bool ObjectCacheEntry<TObject>::matches(const std::type_info& object_type, const std::type_info& desc_type,
                                        const void* desc) const
{
    return object_type == typeid(TObject) && desc_type == typeid(DescType) &&
           ObjectTraits<TObject>::matches(m_desc, *static_cast<const DescType*>(desc));
}

template <typename TObject> 
void* ObjectCacheEntry<TObject>::get() const
{
    return m_object.get();
}

template <typename TObject> 
TObject* ObjectCache::create(const typename ObjectTraits<TObject>::DescType& desc)
{
    auto object = ObjectTraits<TObject>::create(m_physical_device, m_device, desc);

    if (!object)
        return nullptr;

    TObject* result = object.get();
    m_entries.push_back(std::make_unique<ObjectCacheEntry<TObject>>(std::move(object), desc));
    return result;
}

template <typename TObject> 
TObject* ObjectCache::get_or_create(const typename ObjectTraits<TObject>::DescType& desc)
{
    using DescType = typename ObjectTraits<TObject>::DescType;

    for (const auto& entry : m_entries)
    {
        if (entry->matches(typeid(TObject), typeid(DescType), &desc))
            return static_cast<TObject*>(entry->get());
    }

    return create<TObject>(desc);
}

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_CACHE_TPP