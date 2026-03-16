#pragma once

#include <vector>
#include <span>
#include <tuple>
#include <functional>
#include <type_traits>

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

		void Cleanup();

	private:
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

			std::function<void()> Deleter;
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
			void SkipInvalid();

		private:
			View* m_View = nullptr;
			size_type m_Index = 0;
		};

		template<typename ViewType, typename... ValueTypes>
		class ForEachIteratorBase
		{
		public:
			using value_type = std::tuple<EntityID, ValueTypes&...>;
			using difference_type = std::ptrdiff_t;
			using iterator_category = std::forward_iterator_tag;

			ForEachIteratorBase(ViewType& view, size_type index = 0);

			value_type operator*() const;
			ForEachIteratorBase& operator++();
			ForEachIteratorBase operator++(int);

			bool operator==(const ForEachIteratorBase& other) const { return m_Index == other.m_Index; }
			bool operator!=(const ForEachIteratorBase& other) const { return !(*this == other); }

		private:
			void SkipInvalid();

		private:
			ViewType* m_View = nullptr;
			size_type m_Index = 0;
		};

		struct ForEachView
		{
			View& view;

			ForEachIteratorBase<View, Types...> begin() { return ForEachIteratorBase<View, Types...>{ view }; }
			ForEachIteratorBase<const View, const Types...> begin() const { return ForEachIteratorBase<View, Types...>{ view }; }
			ForEachIteratorBase<const View, const Types...> cbegin() const { return begin(); }

			ForEachIteratorBase<View, Types...> end() { return ForEachIteratorBase<View, Types...>{ view, view.EndIndex() }; }
			ForEachIteratorBase<const View, const Types...> end() const { return ForEachIteratorBase<View, Types...>{ view, view.EndIndex() }; }
			ForEachIteratorBase<const View, const Types...> cend() const { return end(); }
		};

	public:
		View(World& world);
		~View() = default;
		
		ForEachView ForEach() { return ForEachView{ *this }; }
		const ForEachView ForEach() const { return ForEachView{ *this }; }

		template<typename Func>
		void ForEach(Func&& func);

		template<typename Func>
		void ForEach(Func&& func) const;

		Iterator begin() { return Iterator(*this, 0); }
		Iterator begin() const { return Iterator(*this, 0); }
		Iterator cbegin() const { return begin(); }

		Iterator end() { return Iterator(*this, EndIndex()); }
		Iterator end() const { return Iterator(*this, EndIndex()); }
		Iterator cend() const { return end(); }

	private:
		size_type EndIndex() const;
		bool HasAll(EntityID id) const;

		std::tuple<Types&...> GetComponents(EntityID id) const;

	private:
		World* m_World = nullptr;
		std::tuple<ComponentStorage<std::remove_cvref_t<Types>>*...> m_Storages = {};
		World::StorageEntry* m_SmallestStorage = nullptr;
	};
}

#include <Flies/World.inl>
