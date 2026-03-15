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

		(remove.template operator()<Types>(), ...);
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
		entry.Deleter = [storage]() { delete reinterpret_cast<ComponentStorage<T>>(storage); storage = nullptr; };
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
	inline View<Types...>::Iterator::Iterator(View& view, size_type index)
		: m_View(&view)
		, m_Index(index)
	{
		SkipInvalid();
	}

	template<typename... Types>
	inline EntityID View<Types...>::Iterator::operator*() const
	{
		return m_View->m_SmallestStorage->Entities()[m_Index];
	}

	template<typename... Types>
	inline View<Types...>::Iterator& View<Types...>::Iterator::operator++()
	{
		++m_Index;
		SkipInvalid();
		return *this;
	}

	template<typename... Types>
	inline View<Types...>::Iterator View<Types...>::Iterator::operator++(int)
	{
		Iterator tmp = *this;
		++(*this);
		return tmp;
	}

	template<typename... Types>
	inline void View<Types...>::Iterator::SkipInvalid()
	{
		if (!m_View->m_SmallestStorage) return;

		std::span<const EntityID> entities = m_View->m_SmallestStorage->Entities();
		while (m_Index < entities.size())
		{
			if (m_View->HasAll(entities[m_Index]))
			{
				break;
			}
			m_Index++;
		}
	}
#	pragma endregion

#	pragma region View::ForEachIteratorBase
	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline View<Types...>::ForEachIteratorBase<ViewType, ValueTypes...>::ForEachIteratorBase(ViewType& view, size_type index)
		: m_View(&view)
		, m_Index(index)
	{
		SkipInvalid();
	}

	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline View<Types...>::ForEachIteratorBase<ViewType, ValueTypes...>::value_type View<Types...>::ForEachIteratorBase<ViewType, ValueTypes...>::operator*() const
	{
		EntityID id = m_View->m_SmallestStorage->Entities()[m_Index];
		return std::tuple_cat(std::make_tuple(id), m_View->GetComponents(id));
	}

	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline View<Types...>::ForEachIteratorBase<ViewType, ValueTypes...>& View<Types...>::ForEachIteratorBase<ViewType, ValueTypes...>::operator++()
	{
		++m_Index;
		SkipInvalid();
		return *this;
	}

	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline View<Types...>::ForEachIteratorBase<ViewType, ValueTypes...> View<Types...>::ForEachIteratorBase<ViewType, ValueTypes...>::operator++(int)
	{
		ForEachIteratorBase tmp = *this;
		++(*this);
		return tmp;
	}

	template<typename... Types>
	template<typename ViewType, typename... ValueTypes>
	inline void View<Types...>::ForEachIteratorBase<ViewType, ValueTypes...>::SkipInvalid()
	{
		if (!m_View->m_SmallestStorage) return;

		std::span<const EntityID> entities = m_View->m_SmallestStorage->Entities();
		while (m_Index < entities.size())
		{
			if (m_View->HasAll(entities[m_Index]))
			{
				break;
			}
			m_Index++;
		}
	}
#	pragma endregion
	template<typename... Types>
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

	template<typename... Types>
	template<typename Func>
	inline void View<Types...>::ForEach(Func&& func)
	{
		ForEachView view = ForEach();
		for (auto&& components : view)
		{
			std::apply(std::forward<Func>(func), components);
		}
	}

	template<typename... Types>
	template<typename Func>
	inline void View<Types...>::ForEach(Func&& func) const
	{
		const ForEachView view = ForEach();
		for (auto&& components : view)
		{
			std::apply(std::forward<Func>(func), components);
		}
	}

	template<typename... Types>
	inline size_type View<Types...>::EndIndex() const
	{
		if (!m_SmallestStorage) return 0;
		return static_cast<size_type>(m_SmallestStorage->Entities().size());
	}

	template<typename... Types>
	inline bool View<Types...>::HasAll(EntityID id) const
	{
		auto contains = [&]<typename T>() -> bool
		{
			ComponentStorage<std::remove_cvref_t<T>>* storage = m_World->GetStorage<std::remove_cvref_t<T>>();
			return storage && storage->Contains(id);
		};

		return (contains.template operator()<Types>() && ...);
	}

	template<typename... Types>
	inline std::tuple<Types&...> View<Types...>::GetComponents(EntityID id) const
	{
		return std::apply([&](auto*... storage) -> std::tuple<Types&...>
			{
				return { *storage->Get(id)... };
			}, m_Storages);
	}
#pragma endregion
}

