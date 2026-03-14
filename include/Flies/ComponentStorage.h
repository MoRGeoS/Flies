#pragma once

#include <memory>
#include <concepts>
#include <span>

#include <Flies/Common/Types.h>

#include <Flies/Entity.h>
#include <Flies/SparseSet.h>

namespace Flies
{
	template<typename T>
	requires std::movable<T>&& std::copyable<T>
	class ComponentStorage
	{
	public:
		ComponentStorage() = default;

		ComponentStorage(const ComponentStorage& other);
		ComponentStorage(ComponentStorage&& other) noexcept;

		ComponentStorage& operator=(const ComponentStorage& other);
		ComponentStorage& operator=(ComponentStorage&& other) noexcept;

		~ComponentStorage() { Cleanup(); }

		// METHODS

		T& Insert(EntityID id, const T& component);
		T& Insert(EntityID id, T&& component);

		template<typename... Args>
		requires std::constructible_from<T, Args...>
		T& Emplace(EntityID id, Args&&... args);

		void Remove(EntityID id);

		inline bool Contains(EntityID id) const;

		T* Get(EntityID id);
		const T* Get(EntityID id) const;

		std::span<const EntityID> Entities() const { return { m_Dense, m_Size }; }

		std::span<T> Components() { return { m_Storage, m_Size }; }
		std::span<const T> Components() const { return { m_Storage, m_Size }; }

		size_type Size() const { return m_Size; }

	private:
		using Dense = EntityID;
		using Storage = T;

		template<typename T>
		using Alloc = std::allocator<T>;

		template<typename T>
		using Traits = std::allocator_traits<Alloc<T>>;

		void Grow();
		void Cleanup();

	private:
		SparseSet m_SparseSet;

		Dense* m_Dense = nullptr;
		Storage* m_Storage = nullptr;
		size_type m_Capacity = 0;
		size_type m_Size = 0;

		Alloc<Dense> m_DenseAlloc;
		Alloc<Storage> m_StorageAlloc;
	};
}

#include <Flies/ComponentStorage.inl>
