#pragma once

#include <vector>
#include <span>
#include <tuple>
#include <functional>
#include <type_traits>

#include "Common/Types.h"

#include "EntityPool.h"
#include "ComponentStorage.h"
#include "TypeInfo.h"

namespace Flies
{
	template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	class View;

	template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	class ForEachView;

	class World
	{
	public:
		World() = default;
		~World();

		void Cleanup();

		Entity CreateEntity();
		void DestroyEntity(Entity entity);

		bool IsAlive(Entity entity) const;

		template <typename T>
		T* InsertComponent(Entity entity, const T& component);

		template <typename T>
		T* InsertComponent(Entity entity, T&& component);

		template <typename T, typename... Args>
		T* EmplaceComponent(Entity entity, Args&&... args);

		template <typename... Types>
		void RemoveComponents(Entity entity);

		template <typename... Types>
		bool HasComponents(Entity entity);

		template <typename T>
		T* GetComponent(Entity entity);

		template <typename T>
		const T* GetComponent(Entity entity) const;

		template <typename... Types>
		Flies::View<Types...> GetView();

		template <typename... Types>
		const Flies::View<Types...> GetView() const;

		template <typename... Types>
		Flies::ForEachView<Types...> GetForEachView();

		template <typename... Types>
		const Flies::ForEachView<Types...> GetForEachView() const;

		template <typename T>
		void OnInsert(std::function<void(World&, Entity)> fn);

		template <typename T>
		void OnRemove(std::function<void(World&, Entity)> fn);

	private:
		template <typename T>
		void CreateStorage();

		template <typename T>
		void DestroyStorage();

		template <typename T>
		bool HasStorage() const;

		template <typename T>
		ComponentStorage<T>* GetStorage();

		template <typename T>
		const ComponentStorage<T>* GetStorage() const;

	private:
		EntityPool m_EntityPool;

		struct StorageEntry
		{
			void* Storage = nullptr;

			std::function<void()> Deleter;
			std::function<void(EntityID)> Remove;
			std::function<bool(EntityID)> Contains;
			std::function<Flies::Span<const EntityID>()> Entities;
			std::function<size_type()> Size;

			std::vector<std::function<void(World&, Entity)>> OnInsert;
			std::vector<std::function<void(World&, Entity)>> OnRemove;
		};
		std::vector<StorageEntry> m_Storages;

		template <typename... Types>
		requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
		friend class View;
	};

	template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	class View
	{
	public:
		class Iterator
		{
		public:
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			Iterator(View& view, size_type index);

			EntityID operator*() const;
			Iterator& operator++();
			Iterator operator++(int);

			bool operator==(const Iterator& other) const { return m_Index == other.m_Index; }
			bool operator!=(const Iterator& other) const { return !(*this == other); }

		private:
			bool HasAll(EntityID id) const;
			void SkipInvalid();

		private:
			View& m_View;
			size_type m_Index = 0;

			template<typename... UTypes>
			requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
			friend class ForEachView;
		};

	public:
		View(World& world);
		~View() = default;

		void Each(std::function<void(EntityID)> func) const;

		Iterator begin() { return Iterator(*this, 0); }
		const Iterator begin() const { return Iterator(*this, 0); }
		const Iterator cbegin() const { return begin(); }

		Iterator end() { return Iterator(*this, EndIndex()); }
		const Iterator end() const { return Iterator(*this, EndIndex()); }
		const Iterator cend() const { return end(); }

	private:
		size_type EndIndex() const;

	private:
		World& m_World;
		std::tuple<ComponentStorage<std::remove_cvref_t<Types>>*...> m_Storages = {};
		World::StorageEntry* m_SmallestStorage = nullptr;

		template<typename... UTypes>
		requires ((!std::is_reference_v<UTypes> && !std::is_volatile_v<UTypes>) && ...)
		friend class ForEachView;
	};

	template <typename... Types>
	requires ((!std::is_reference_v<Types> && !std::is_volatile_v<Types>) && ...)
	class ForEachView
	{
	public:
		class Iterator
		{
		public:
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			Iterator(View<Types...>& view, size_type index);
			Iterator(const View<Types...>::Iterator& iterator);

			std::tuple<EntityID, Types&...> operator*() const;

			Iterator& operator++();
			Iterator operator++(int);

			bool operator==(const Iterator& other) const { return m_Iterator == other.m_Iterator; }
			bool operator!=(const Iterator& other) const { return !(*this == other); }

		private:
			View<Types...>::Iterator m_Iterator;
		};

	public:
		ForEachView(World& world);
		ForEachView(const View<Types...>& view);

		void Each(std::function<void(EntityID id, Types&...)> func);
		void Each(std::function<void(EntityID id, const Types&...)> func) const;

		Iterator begin() { return Iterator(m_View.begin()); }
		Iterator begin() const { return Iterator(m_View.begin()); }
		Iterator cbegin() const { return begin(); }

		Iterator end() { return Iterator(m_View.end()); }
		Iterator end() const { return Iterator(m_View.end()); }
		Iterator cend() const { return end(); }

	private:
		View<Types...> m_View;
	};
}

#include "World.inl"
