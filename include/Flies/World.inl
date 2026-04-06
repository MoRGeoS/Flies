#pragma once

#include "World.h"
#include "Common/Assert.h"

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

		// Insert
		CreateStorage<T>();
		ComponentStorage<T>* storage = GetStorage<T>();
		storage->Insert(entity.id, component);

		// Call OnInsert callback
		size_type index = TypeID<T>().seq();
		for (auto& cb : m_Storages[index].OnInsert)
		{
			cb(*this, entity);
		}

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

		// Insert
		CreateStorage<T>();
		ComponentStorage<T>* storage = GetStorage<T>();
		storage->Insert(entity.id, std::move(component));

		// Call OnInsert callback
		size_type index = TypeID<T>().seq();
		for (auto& cb : m_Storages[index].OnInsert)
		{
			cb(*this, entity);
		}

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

		// Emplace
		CreateStorage<T>();
		ComponentStorage<T>* storage = GetStorage<T>();
		storage->Emplace(entity.id, std::forward<Args>(args)...);

		// Call OnInsert callback
		size_type index = TypeID<T>().seq();
		for (auto& cb : m_Storages[index].OnInsert)
		{
			cb(*this, entity);
		}

		return storage->Get(entity.id);
	}

	template<typename... Types>
	inline void World::RemoveComponents(Entity entity)
	{
		if (!IsAlive(entity)) return;

		auto remove = [&]<typename T>()
		{
			size_type index = TypeID<T>().seq();
			if (m_Storages.size() <= index) return;

			// Call OnRemove callback
			for (auto& cb : m_Storages[index].OnRemove)
			{
				cb(*this, entity);
			}

			// Remove
			if (m_Storages[index].Storage)
			{
				m_Storages[index].Remove(entity.id);
			}
		};

		(remove.template operator()<Types>(), ...);
	}

	template<typename... Types>
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

		return (has.template operator()<Types>(entity.id) && ...);
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
	inline View<Types...> World::GetView()
	{
		return View<Types...>(*this);
	}

	template<typename ...Types>
	inline const View<Types...> World::GetView() const
	{
		return View<Types...>(*this);
	}

    template <typename... Types>
    inline Flies::ForEachView<Types...> World::GetForEachView()
    {
        return Flies::ForEachView<Types...>(*this);
    }

    template <typename... Types>
    inline const Flies::ForEachView<Types...> World::GetForEachView() const
    {
        return Flies::ForEachView<Types...>(*this);
    }

    template<typename T>
	inline void World::OnInsert(std::function<void(World&, Entity)> fn)
	{
		size_type index = TypeID<T>().seq();
		if (m_Storages.size() <= index)
		{
			m_Storages.resize(index + 1);
		}

		m_Storages[index].OnInsert.push_back(fn);
	}

	template<typename T>
	inline void World::OnRemove(std::function<void(World&, Entity)> fn)
	{
		size_type index = TypeID<T>().seq();
		if (m_Storages.size() <= index)
		{
			m_Storages.resize(index + 1);
		}

		m_Storages[index].OnRemove.push_back(fn);
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
		entry.Deleter = [this, index]()
			{
				if (m_Storages[index].Storage) 
				{
					delete reinterpret_cast<ComponentStorage<T>*>(m_Storages[index].Storage);
					m_Storages[index].Storage = nullptr;
				}
			};

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
		entry.Deleter();
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
#	pragma region View::Iterator
	template<typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	inline View<Types...>::Iterator::Iterator(View& view, size_type index)
		: m_View(view)
		, m_Index(index)
	{
		SkipInvalid();
	}

	template<typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	inline EntityID View<Types...>::Iterator::operator*() const
	{
		return m_View.m_SmallestStorage->Entities()[m_Index];
	}

	template<typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	inline View<Types...>::Iterator& View<Types...>::Iterator::operator++()
	{
		++m_Index;
		SkipInvalid();
		return *this;
	}

	template<typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	inline View<Types...>::Iterator View<Types...>::Iterator::operator++(int)
	{
		Iterator tmp = *this;
		++(*this);
		return tmp;
	}

	template<typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	inline bool View<Types...>::Iterator::HasAll(EntityID id) const
	{
		return std::apply([&](auto*... storage) -> bool
			{
				return (... && (storage && storage->Contains(id)));
			}, m_Storages);
	}

	template<typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	inline void View<Types...>::Iterator::SkipInvalid()
	{
		if (!m_View->m_SmallestStorage) return;

		std::span<const EntityID> entities = m_View->m_SmallestStorage->Entities();
		while (m_Index < entities.size())
		{
			if (HasAll(entities[m_Index]))
			{
				break;
			}
			m_Index++;
		}
	}
#	pragma endregion

	template<typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	inline View<Types...>::View(World& world)
		: m_World(&world)
	{
		auto getStorage = [&]<typename T>() -> ComponentStorage<std::remove_cvref_t<T>>*
		{
			return m_World->GetStorage<std::remove_cvref_t<T>>();
		};
		m_Storages = std::make_tuple(getStorage.template operator()<Types>()...);

		auto consider = [&]<typename T>()
		{
			size_type index = TypeID<T>().seq();
			if (index >= m_World->m_Storages.size()) return;

			World::StorageEntry& entry = m_World->m_Storages[index];
			if (!entry.Storage) return;

			if (!m_SmallestStorage || entry.Size() < m_SmallestStorage->Size())
				m_SmallestStorage = &entry;
		};
		(consider.template operator()<Types>(), ...);
	}

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline void View<Types...>::Each(std::function<void(EntityID)> func) const
    {
		for (auto it = begin(); it != end(); it++)
		{
			func(*it);
		}
    }

    template<typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	inline size_type View<Types...>::EndIndex() const
	{
		if (!m_SmallestStorage) return 0;
		return static_cast<size_type>(m_SmallestStorage->Entities().size());
	}
#pragma endregion

#pragma region ForEachView
#	pragma region ForEachView::IteratorBase
    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline ForEachView<Types...>::Iterator::Iterator(View<Types...>& view, size_type index)
		: m_Iterator(view, index)
    {
    }

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline ForEachView<Types...>::Iterator::Iterator(const View<Types...>::Iterator& iterator)
		: m_Iterator(iterator)
    {
    }

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline std::tuple<EntityID, Types&...> ForEachView<Types...>::Iterator::operator*() const
    {
		EntityID id = *m_Iterator;

    	return std::apply(
    	    [id](auto*... storages) -> std::tuple<EntityID, Types&...>
    	    {
    	        return { id, *storages->GetUnsafe(id)... };
    	    },
    	    m_Iterator.m_View.m_Storages
    	);
    }

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline ForEachView<Types...>::Iterator& ForEachView<Types...>::Iterator::operator++()
    {
		return ++m_Iterator;
    }

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline ForEachView<Types...>::Iterator ForEachView<Types...>::Iterator::operator++(int i)
    {
        return m_Iterator++(i);
    }
#pragma endregion

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline ForEachView<Types...>::ForEachView(World& world)
		: m_View(world.GetView<Types...>())
    {
    }

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline ForEachView<Types...>::ForEachView(const View<Types...>& view)
		: m_View(view) 
    {
    }

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline void ForEachView<Types...>::Each(std::function<void(EntityID id, Types&...)> func)
    {
		for (auto it = begin(); it != end(); it++)
		{
			func(*it);
		}
    }

    template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
    inline void ForEachView<Types...>::Each(std::function<void(EntityID id, const Types&...)> func) const
    {
		for (auto it = begin(); it != end(); it++)
		{
			func(*it);
		}
    }
#pragma endregion
}
