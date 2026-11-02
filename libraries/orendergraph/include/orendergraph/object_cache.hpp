#ifndef ORENDERGRAPH_OBJECT_CACHE_HPP
#define ORENDERGRAPH_OBJECT_CACHE_HPP

#include <orendergraph/object_traits/all.hpp>

#include <memory>
#include <typeinfo>
#include <utility>
#include <vector>

namespace orendergraph
{

class IObjectCacheEntry
{
  public:
    virtual ~IObjectCacheEntry() = default;

    virtual bool matches(const std::type_info& object_type, const std::type_info& desc_type, const void* desc) const = 0;

    virtual void* get() const = 0;
};

template <typename TObject> 
class ObjectCacheEntry final : public IObjectCacheEntry
{
  public:
    using DescType = typename ObjectTraits<TObject>::DescType;

    ObjectCacheEntry(std::unique_ptr<TObject> object, const DescType& desc);

    bool matches(const std::type_info& object_type, const std::type_info& desc_type, const void* desc) const override;

    void* get() const override;

  private:
    DescType m_desc;
    std::unique_ptr<TObject> m_object;
};

class ObjectCache
{
  public:
    ObjectCache(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device);

    void clear();

    template <typename TObject> TObject* create(const typename ObjectTraits<TObject>::DescType& desc);
    template <typename TObject> TObject* get_or_create(const typename ObjectTraits<TObject>::DescType& desc);

  private:
    const ogfx::PhysicalDevice& m_physical_device;
    const ogfx::Device& m_device;

    std::vector<std::unique_ptr<IObjectCacheEntry>> m_entries;
};

} // namespace orendergraph

#include <orendergraph/object_cache.tpp>

#endif // ORENDERGRAPH_OBJECT_CACHE_HPP