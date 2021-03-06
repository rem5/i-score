#pragma once
#include <ossia/detail/algorithms.hpp>
#include <iscore/model/ComponentSerialization.hpp>
#include <iscore/tools/IdentifierGeneration.hpp>
#include <nano_observer.hpp>
#include <vector>

namespace iscore
{
/**
 * \class ComponentHierarchyManager
 * \brief Manages simple hierarchies of components
 *
 * For instance, if the object Foo is a container for Bar elements,
 * this class can be used so that every time a new Bar is created, a matching component
 * will be added to Bar.
 *
 */
template <
    typename ParentComponent_T, typename ChildModel_T,
    typename ChildComponent_T>
class ComponentHierarchyManager : public ParentComponent_T,
                                  public Nano::Observer
{
public:
  using hierarchy_t = ComponentHierarchyManager;

  struct ChildPair
  {
    ChildPair(ChildModel_T* m, ChildComponent_T* c) : model{m}, component{c}
    {
    }
    ChildModel_T* model{};
    ChildComponent_T* component{};
  };

  template <typename... Args>
  ComponentHierarchyManager(Args&&... args)
      : ParentComponent_T{std::forward<Args>(args)...}
  {
    auto& child_models = ParentComponent_T::template models<ChildModel_T>();
    for (auto& child_model : child_models)
    {
      add(child_model);
    }

    child_models.mutable_added
        .template connect<hierarchy_t, &hierarchy_t::add>(this);

    child_models.removing.template connect<hierarchy_t, &hierarchy_t::remove>(
        this);
  }

  const auto& children() const
  {
    return m_children;
  }


  void add(ChildModel_T& element)
  {
    add(element,
        typename iscore::is_component_serializable<ChildComponent_T>::type{});
  }

  void add(ChildModel_T& element, iscore::serializable_tag)
  {
    // Since the component may be serializable, we first look if
    // we can deserialize it.
    auto comp = iscore::deserialize_component<ChildComponent_T>(
          element.components(),
          [&] (auto&& deserializer) {
      ParentComponent_T::template load<ChildComponent_T>(deserializer, element);
    });

    // Maybe we could not deserialize it
    if(!comp)
    {
      comp = ParentComponent_T::template make<ChildComponent_T>(
            getStrongId(element.components()), element);
    }

    // We try to add it
    if (comp)
    {
      element.components().add(comp);
      m_children.emplace_back(ChildPair{&element, comp});
    }
  }

  void add(ChildModel_T& model, iscore::not_serializable_tag)
  {
    // The subclass should provide this function to construct
    // the correct component relative to this process.
    auto proc_comp
        = ParentComponent_T::make(getStrongId(model.components()), model);
    if (proc_comp)
    {
      model.components().add(proc_comp);
      m_children.emplace_back(ChildPair{&model, proc_comp});
    }
  }

  void remove(const ChildModel_T& model)
  {
    auto it = ossia::find_if(
        m_children, [&](auto pair) { return pair.model == &model; });

    if (it != m_children.end())
    {
      remove(*it);
      m_children.erase(it);
    }
  }

  void remove(const ChildPair& pair)
  {
    ParentComponent_T::removing(*pair.model, *pair.component);
    pair.model->components().remove(*pair.component);
  }

  void clear()
  {
    for (const auto& element : m_children)
    {
      remove(element);
    }
    m_children.clear();
  }

  ~ComponentHierarchyManager()
  {
    clear();
  }

private:
  std::vector<ChildPair> m_children; // todo map ? multi_index with both index
                                     // of the component and of the process ?
};

/**
 * \class PolymorphicComponentHierarchyManager
 * \brief Manages polymorphic hierarchies of components
 *
 * Like ComponentHierarchyManager, but used when the components of the child class are polymorphic.
 * For instance, if we have a Process, we want to create components that will be specific to
 * each Process type. But then we have to source them from a factory somehow.
 */
template <
    typename ParentComponent_T, typename ChildModel_T,
    typename ChildComponent_T, typename ChildComponentFactoryList_T>
class PolymorphicComponentHierarchyManager : public ParentComponent_T,
                                             public Nano::Observer
{
public:
  using hierarchy_t = PolymorphicComponentHierarchyManager;

  struct ChildPair
  {
    ChildPair(ChildModel_T* m, ChildComponent_T* c) : model{m}, component{c}
    {
    }
    ChildModel_T* model{};
    ChildComponent_T* component{};
  };

  template <typename... Args>
  PolymorphicComponentHierarchyManager(Args&&... args)
      : ParentComponent_T{std::forward<Args>(args)...}
      , m_componentFactory{
            ParentComponent_T::system()
                .context()
                .app.components
                .template interfaces<ChildComponentFactoryList_T>()}
  {
    auto& child_models = ParentComponent_T::template models<ChildModel_T>();
    for (auto& child_model : child_models)
    {
      add(child_model);
    }

    child_models.mutable_added
        .template connect<hierarchy_t, &hierarchy_t::add>(this);

    child_models.removing.template connect<hierarchy_t, &hierarchy_t::remove>(
        this);
  }

  const auto& children() const
  {
    return m_children;
  }

  void add(ChildModel_T& element)
  {
    add(element,
        typename iscore::is_component_serializable<ChildComponent_T>::type{});
  }

  void add(ChildModel_T& element, iscore::serializable_tag)
  {
    // Will return a factory for the given process if available
    if (auto factory = m_componentFactory.factory(model))
    {
      // Since the component may be serializable, we first look if
      // we can deserialize it.
      auto comp = iscore::deserialize_component<ChildComponent_T>(
            element.components(),
            [&] (auto&& deserializer) {
        ParentComponent_T::template load<ChildComponent_T>(deserializer, *factory, element);
      });

      // Maybe we could not deserialize it
      if(!comp)
      {
        comp = ParentComponent_T::template make<ChildComponent_T>(
              getStrongId(element.components()), *factory, element);
      }

      // We try to add it
      if (comp)
      {
        element.components().add(comp);
        m_children.emplace_back(ChildPair{&element, comp});
      }
    }
  }
  void add(ChildModel_T& model, iscore::not_serializable_tag)
  {
    // Will return a factory for the given process if available
    if (auto factory = m_componentFactory.factory(model))
    {
      // The subclass should provide this function to construct
      // the correct component relative to this process.
      auto proc_comp = ParentComponent_T::make(
          getStrongId(model.components()), *factory, model);
      if (proc_comp)
      {
        model.components().add(proc_comp);
        m_children.emplace_back(ChildPair{&model, proc_comp});
      }
    }
  }

  void remove(const ChildModel_T& model)
  {
    auto it = ossia::find_if(
        m_children, [&](auto pair) { return pair.model == &model; });

    if (it != m_children.end())
    {
      remove(*it);
      m_children.erase(it);
    }
  }

  void remove(const ChildPair& pair)
  {
    ParentComponent_T::removing(*pair.model, *pair.component);
    pair.model->components().remove(*pair.component);
  }

  void clear()
  {
    for (const auto& element : m_children)
    {
      remove(element);
    }
    m_children.clear();
  }

  ~PolymorphicComponentHierarchyManager()
  {
    clear();
  }

private:
  const ChildComponentFactoryList_T& m_componentFactory;

  std::vector<ChildPair> m_children; // todo map ? multi_index with both index
                                     // of the component and of the process ?
};

template <typename Component>
using ComponentHierarchy
    = ComponentHierarchyManager<Component, typename Component::model_t, typename Component::component_t>;

template <typename Component>
using PolymorphicComponentHierarchy
    = PolymorphicComponentHierarchyManager<Component, typename Component::model_t, typename Component::component_t, typename Component::component_factory_list_t>;
}
