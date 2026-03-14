#pragma once

#include <vector>
#include <span>
#include <tuple>
#include <functional>

#include <Flies/EntityPool.h>
#include <Flies/ComponentStorage.h>
#include <Flies/TypeInfo.h>

#include <Flies/Common/Types.h>
#include <Flies/Common/Assert.h>

namespace Flies
{
	class World;

	template<typename... Types>
	class View;

	class World
	{
	public:
		World() = default;
		~World();

		Entity CreateEntity();
		void DestroyEntity(Entity entity);

		bool IsAlive(Entity entity) const;

		template<typename T>
		T* InsertComponent(Entity entity, const T& component);

		template<typename T>
		T* InsertComponent(Entity entity, T&& component);

		template<typename T, typename... Args>
		T* EmplaceComponent(Entity entity, Args&&... args);

		template<typename... Types>
		void RemoveComponents(Entity entity);

		template<typename... Types>
		bool HasComponents(Entity entity);

		template<typename T>
		T* GetComponent(Entity entity);

		template<typename T>
		const T* GetComponent(Entity entity) const;

		template<typename... Types>
		View<Types...> CreateView();

		template<typename... Types>
		const View<Types...> CreateView() const;

	private:
		void Cleanup();

		template<typename T>
		void CreateStorage();

		template<typename T>
		void DestroyStorage();

		template<typename T>
		bool HasStorage() const;

		template<typename T>
		ComponentStorage<T>* GetStorage();

		template<typename T>
		const ComponentStorage<T>* GetStorage() const;

	private:
		EntityPool m_EntityPool;

		struct StorageEntry
		{
			void* Storage = nullptr;

			std::function<void(EntityID)> Remove;
			std::function<bool(EntityID)> Contains;
			std::function<std::span<const EntityID>()> Entities;
			std::function<size_type()> Size;
		};
		std::vector<StorageEntry> m_Storages;

		template<typename... Types>
		friend class View;
	};

	template<typename... Types>
	class View
	{
	public:
		template<typename ViewType, typename... ValueTypes>
		class IteratorBase
		{
		public:
			using value_type = std::tuple<ValueTypes&...>;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			IteratorBase(ViewType& view, size_type index);

			auto operator*() const;
			IteratorBase& operator++();
			IteratorBase operator++(int);

			bool operator==(const IteratorBase& other) const { return m_Index == other.m_Index; }
			bool operator!=(const IteratorBase& other) const { return !(*this == other); }

		private:
			void SkipInvalid();

			ViewType* m_View = nullptr;
			size_type   m_Index = 0;
		};

		using Iterator = IteratorBase<View, Types...>;
		using ConstIterator = IteratorBase<const View, const Types...>;

	public:
		View(World& world);
		~View() = default;
		
		template<typename Func>
		void ForEach(Func&& func);

		Iterator begin() { return Iterator(*this, 0); }
		ConstIterator begin()  const { return ConstIterator(*this, 0); }
		ConstIterator cbegin() const { return begin(); }

		Iterator end() { return Iterator(*this, EndIndex()); }
		ConstIterator end()    const { return ConstIterator(*this, EndIndex()); }
		ConstIterator cend()   const { return end(); }

	private:
		size_type EndIndex() const;
		bool HasAll(EntityID id) const;

		template<typename... ValueTypes>
		std::tuple<ValueTypes&...> GetComponents(EntityID id) const;

	private:
		World* m_World = nullptr;
		std::tuple<ComponentStorage<Types>*...> m_Storages = {};
		World::StorageEntry* m_SmallestStorage = nullptr;
	};
}

#include <Flies/World.inl>
