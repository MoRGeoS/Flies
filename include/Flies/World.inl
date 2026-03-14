#pragma once

#include "World.h"

namespace Flies
{
#pragma region World
	template<typename T>
	inline T* World::InsertComponent(Entity entity, const T& component)
	{
		if (!IsAlive(entity))
		{
			FLS_ASSERT(false, "Entity is not alive");
			return nullptr;
		}

		CreateStorage<T>();
		ComponentStorage<T>* storage = GetStorage<T>();
		storage->Insert(entity.id, component);
		return storage->Get(entity.id);
	}
	template<typename T>
	inline T* Flies::World::InsertComponent(Entity entity, T&& component)
	{
		if (!IsAlive(entity))
		{
			FLS_ASSERT(false, "Entity is not alive");
			return nullptr;
		}

		CreateStorage<T>();
		ComponentStorage<T>* storage = GetStorage<T>();
		storage->Insert(entity.id, std::move(component));
		return storage->Get(entity.id);
	}
	template<typename T, typename ...Args>
	inline T* World::EmplaceComponent(Entity entity, Args && ...args)
	{
		if (!IsAlive(entity))
		{
			FLS_ASSERT(false, "Entity is not alive");
			return nullptr;
		}

		CreateStorage<T>();
		ComponentStorage<T>* storage = GetStorage<T>();
		storage->Emplace(entity.id, std::forward<Args>(args)...);
		return storage->Get(entity.id);
	}
	template<typename ...Types>
	inline void World::RemoveComponents(Entity entity)
	{
		if (!IsAlive(entity)) return;

		auto remove = [&]<typename T>()
		{
			ComponentStorage<T>* storage = GetStorage<T>();
			if (storage)
			{
				storage->Remove(entity.id);
			}
		};

		(remove.template operator() < Types > (), ...);
	}
	template<typename ...Types>
	inline bool World::HasComponents(Entity entity)
	{
		if (!IsAlive(entity)) return false;

		auto has = [&]<typename T>(EntityID id) -> bool
		{
			ComponentStorage<T>* storage = GetStorage<T>();
			if (storage)
			{
				return storage->Contains(id);
			}
			return false;
		};

		return (has.template operator() < Types > (entity.id) && ...);
	}
	template<typename T>
	inline T* World::GetComponent(Entity entity)
	{
		if (!IsAlive(entity)) return nullptr;

		ComponentStorage<T>* storage = GetStorage<T>();
		if (!storage) return nullptr;

		return storage->Get(entity.id);
	}
	template<typename T>
	inline const T* World::GetComponent(Entity entity) const
	{
		if (!IsAlive(entity)) return nullptr;

		ComponentStorage<T>* storage = GetStorage<T>();
		if (!storage) return nullptr;

		return storage->Get(entity.id);
	}
	template<typename ...Types>
	inline View<Types...> World::CreateView()
	{
		return View<Types...>(*this);
	}
	template<typename ...Types>
	inline const View<Types...> World::CreateView() const
	{
		return View<Types...>(*this);
	}
	template<typename T>
	inline void World::CreateStorage()
	{
		if (HasStorage<T>()) return;

		size_type index = TypeID<T>().seq();

		if (m_Storages.size() <= index)
		{
			m_Storages.resize(index + 1);
		}

		ComponentStorage<T>* storage = new ComponentStorage<T>();
		StorageEntry& entry = m_Storages[index];
		entry.Storage = storage;
		entry.Remove = [storage](EntityID id) { storage->Remove(id); };
		entry.Contains = [storage](EntityID id) { return storage->Contains(id); };
		entry.Entities = [storage]() { return storage->Entities(); };
		entry.Size = [storage]() { return storage->Size(); };
	}
	template<typename T>
	inline void World::DestroyStorage()
	{
		if (!HasStorage<T>()) return;

		size_type index = TypeID<T>().seq();
		StorageEntry& entry = m_Storages[index];
		delete entry.Storage;
		entry.Storage = nullptr;
	}
	template<typename T>
	inline bool World::HasStorage() const
	{
		size_type index = TypeID<T>().seq();
		return (
			m_Storages.size() > index &&
			m_Storages[index].Storage != nullptr
			);
	}
	template<typename T>
	inline ComponentStorage<T>* World::GetStorage()
	{
		if (HasStorage<T>())
		{
			size_type index = TypeID<T>().seq();
			return reinterpret_cast<ComponentStorage<T>*>(m_Storages[index].Storage);
		}
		return nullptr;
	}
	template<typename T>
	inline const ComponentStorage<T>* World::GetStorage() const
	{
		if (HasStorage<T>())
		{
			size_type index = TypeID<T>().seq();
			return reinterpret_cast<ComponentStorage<T>*>(m_Storages[index].Storage);
		}
		return nullptr;
	}
#pragma endregion

#pragma region View
#pragma region View::Iterator
	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline View<Types...>::IteratorBase<ViewType, ValueTypes...>::IteratorBase(ViewType& view, size_type index)
		: m_View(&view)
		, m_Index(index)
	{
		SkipInvalid();
	}

	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline auto View<Types...>::template IteratorBase<ViewType, ValueTypes...>::operator*() const
	{
		EntityID id = m_View->m_SmallestStorage->Entities()[m_Index];
		return m_View->template GetComponents<ValueTypes...>(id);
	}

	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline View<Types...>::IteratorBase<ViewType, ValueTypes...>& View<Types...>::template IteratorBase<ViewType, ValueTypes...>::operator++()
	{
		++m_Index;
		SkipInvalid();
		return *this;
	}

	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline View<Types...>::IteratorBase<ViewType, ValueTypes...> View<Types...>::template IteratorBase<ViewType, ValueTypes...>::operator++(int)
	{
		IteratorBase tmp = *this;
		++(*this);
		return tmp;
	}

	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline void View<Types...>::template IteratorBase<ViewType, ValueTypes...>::SkipInvalid()
	{
		if (!m_View->m_SmallestStorage) return;

		auto entities = m_View->m_SmallestStorage->Entities();
		while (m_Index < entities.size())
		{
			if (m_View->HasAll(entities[m_Index])) break;
			++m_Index;
		}
	}
#pragma endregion

	template<typename... Types>
	inline View<Types...>::View(World& world)
		: m_World(&world)
	{
		auto getStorage = [&]<typename T>() -> ComponentStorage<T>*
		{
			return m_World->GetStorage<T>();
		};

		m_Storages = std::make_tuple(getStorage.template operator() < Types > ()...);

		auto consider = [&]<typename T>()
		{
			size_type index = TypeID<T>().seq();
			if (index >= m_World->m_Storages.size()) return;

			World::StorageEntry& entry = m_World->m_Storages[index];
			if (!entry.Storage) return;

			if (!m_SmallestStorage || entry.Size() < m_SmallestStorage->Size())
				m_SmallestStorage = &entry;
		};

		(consider.template operator() < Types > (), ...);
	}

	template<typename ...Types>
	template<typename Func>
	inline void View<Types...>::ForEach(Func&& func)
	{
		for (auto&& components : *this)
		{
			std::apply(std::forward<Func>(func), components);
		}
	}

	template<typename ...Types>
	inline size_type View<Types...>::EndIndex() const
	{
		if (!m_SmallestStorage) return 0;
		return static_cast<size_type>(m_SmallestStorage->Entities().size());
	}

	template<typename ...Types>
	inline bool View<Types...>::HasAll(EntityID id) const
	{
		auto contains = [&]<typename T>() -> bool
		{
			ComponentStorage<T>* storage = m_World->GetStorage<T>();
			return storage && storage->Contains(id);
		};

		return (contains.template operator() < Types > () && ...);
	}

	template<typename ...Types>
	template<typename ...ValueTypes>
	inline std::tuple<ValueTypes&...> View<Types...>::GetComponents(EntityID id) const
	{
		return std::apply([&](auto*... storage) -> std::tuple<ValueTypes&...>
			{
				return { *storage->Get(id)... };
			}, m_Storages);
	}
#pragma endregion
}

