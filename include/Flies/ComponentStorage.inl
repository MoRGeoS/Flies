#pragma once

#include <Flies/Common/Assert.h>
#include "ComponentStorage.h"

namespace Flies
{
	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline ComponentStorage<T>::ComponentStorage(const ComponentStorage& other)
	{
		m_SparseSet = other.m_SparseSet;

		if (other.m_Size > 0)
		{
			// Allocate Dense
			m_Dense = Traits<Dense>::allocate(m_DenseAlloc, other.m_Capacity);
			FLS_ASSERT(m_Dense, "Failed to allocate memory");

			// Allocate Storage
			m_Storage = Traits<Storage>::allocate(m_StorageAlloc, other.m_Capacity);
			FLS_ASSERT(m_Storage, "Failed to allocate memory");

			// Copy Dense
			std::uninitialized_copy_n(other.m_Dense, other.m_Size, m_Dense);

			// Copy Storage
			for (size_type i = 0; i != other.m_Size; i++)
			{
				m_Storage[i] = other.m_Storage[i];
			}

			// Assign sizes
			m_Capacity = other.m_Capacity;
			m_Size = other.m_Size;
		}
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline Flies::ComponentStorage<T>::ComponentStorage(ComponentStorage&& other) noexcept
	{
		m_SparseSet = std::move(other.m_SparseSet);

		if (other.m_Size > 0)
		{
			// Copy values
			m_Dense = other.m_Dense;
			m_Storage = other.m_Storage;
			m_Capacity = other.m_Capacity;
			m_Size = other.m_Size;

			// Assign default values
			other.m_Dense = nullptr;
			other.m_Storage = nullptr;
			other.m_Capacity = 0;
			other.m_Size = 0;
		}
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline ComponentStorage<T>& ComponentStorage<T>::operator=(const ComponentStorage& other)
	{
		if (this != &other)
		{
			m_SparseSet = other.m_SparseSet;

			Cleanup();

			if (other.m_Size > 0)
			{
				// Allocate Dense
				m_Dense = Traits<Dense>::allocate(m_DenseAlloc, other.m_Capacity);
				FLS_ASSERT(m_Dense, "Failed to allocate memory");

				// Allocate Storage
				m_Storage = Traits<Storage>::allocate(m_StorageAlloc, other.m_Capacity);
				FLS_ASSERT(m_Storage, "Failed to allocate memory");

				// Copy Dense
				std::uninitialized_copy_n(other.m_Dense, other.m_Size, m_Dense);

				// Copy Storage
				for (size_type i = 0; i != other.m_Size; i++)
				{
					m_Storage[i] = other.m_Storage[i];
				}

				// Assign sizes
				m_Capacity = other.m_Capacity;
				m_Size = other.m_Size;
			}
		}
		return *this;
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline ComponentStorage<T>& ComponentStorage<T>::operator=(ComponentStorage&& other) noexcept
	{
		if (this != &other)
		{
			m_SparseSet = std::move(other.m_SparseSet);

			Cleanup();

			if (other.m_Size > 0)
			{
				// Copy values
				m_Dense = other.m_Dense;
				m_Storage = other.m_Storage;
				m_Capacity = other.m_Capacity;
				m_Size = other.m_Size;

				// Assign default values
				other.m_Dense = nullptr;
				other.m_Storage = nullptr;
				other.m_Capacity = 0;
				other.m_Size = 0;
			}
		}
		return *this;
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline T& ComponentStorage<T>::Insert(EntityID id, const T& component)
	{
		if (Contains(id))
		{
			size_type di = m_SparseSet[id];
			m_Storage[di] = component;
			return m_Storage[di];
		}

		if (m_Size >= m_Capacity)
		{
			Grow();
		}

		Traits<Dense>::construct(m_DenseAlloc, m_Dense + m_Size, id);
		Traits<Storage>::construct(m_StorageAlloc, m_Storage + m_Size, component);

		m_SparseSet.Insert(id, m_Size);
		return m_Storage[m_Size++];
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline T& ComponentStorage<T>::Insert(EntityID id, T&& component)
	{
		if (Contains(id))
		{
			size_type di = m_SparseSet[id];
			m_Storage[di] = std::move(component);
			return m_Storage[di];
		}

		if (m_Size >= m_Capacity)
		{
			Grow();
		}

		Traits<Dense>::construct(m_DenseAlloc, m_Dense + m_Size, id);
		Traits<Storage>::construct(m_StorageAlloc, m_Storage + m_Size, std::move(component));

		m_SparseSet.Insert(id, m_Size);
		return m_Storage[m_Size++];
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	template<typename... Args>
		requires std::constructible_from<T, Args...>
	inline T& ComponentStorage<T>::Emplace(EntityID id, Args&&... args)
	{
		if (Contains(id))
		{
			size_type di = m_SparseSet[id];
			Traits<Storage>::destroy(m_StorageAlloc, m_Storage + di);
			Traits<Storage>::construct(m_StorageAlloc, m_Storage + di, std::forward<Args>(args)...);
			return m_Storage[di];
		}

		if (m_Size >= m_Capacity)
		{
			Grow();
		}

		Traits<Dense>::construct(m_DenseAlloc, m_Dense + m_Size, id);
		Traits<Storage>::construct(m_StorageAlloc, m_Storage + m_Size, std::forward<Args>(args)...);

		m_SparseSet.Insert(id, m_Size);
		return m_Storage[m_Size++];
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline void ComponentStorage<T>::Remove(EntityID id)
	{
		if (Contains(id))
		{
			size_type di = m_SparseSet[id];
			size_type last = m_Size - 1;

			// Swap
			if (di != last)
			{
				m_SparseSet.Insert(m_Dense[last], di);
				m_Dense[di] = m_Dense[last];
				m_Storage[di] = std::move(m_Storage[last]);
			}

			// Pop back
			Traits<Dense>::destroy(m_DenseAlloc, m_Dense + last);
			Traits<Storage>::destroy(m_StorageAlloc, m_Storage + last);
			m_SparseSet.Remove(id);
			m_Size--;
		}
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline bool ComponentStorage<T>::Contains(EntityID id) const
	{
		return m_SparseSet.Contains(id);
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline T* ComponentStorage<T>::Get(EntityID id)
	{
		if (Contains(id))
		{
			return m_Storage + m_SparseSet[id];
		}
		return nullptr;
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline const T* ComponentStorage<T>::Get(EntityID id) const
	{
		if (Contains(id))
		{
			return m_Storage + m_SparseSet[id];
		}
		return nullptr;
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline void ComponentStorage<T>::Grow()
	{
		size_type newCapacity = m_Capacity ? m_Capacity * (size_type)2 : (size_type)20;

		Dense* newDense = Traits<Dense>::allocate(m_DenseAlloc, newCapacity);
		FLS_ASSERT(newDense, "Failed to allocate memory");

		Storage* newStorage = Traits<Storage>::allocate(m_StorageAlloc, newCapacity);
		FLS_ASSERT(newStorage, "Failed to allocate memory");

		if (m_Dense && m_Storage)
		{
			std::uninitialized_copy_n(m_Dense, m_Size, newDense);
			Traits<Dense>::deallocate(m_DenseAlloc, m_Dense, m_Capacity);

			std::uninitialized_move_n(m_Storage, m_Size, newStorage);
			Traits<Storage>::deallocate(m_StorageAlloc, m_Storage, m_Capacity);
		}

		m_Dense = newDense;
		m_Storage = newStorage;
		m_Capacity = newCapacity;
	}

	template<typename T>
		requires std::movable<T>&& std::copyable<T>
	inline void ComponentStorage<T>::Cleanup()
	{
		if (m_Capacity > 0)
		{
			FLS_ASSERT(m_Storage && m_Dense)
			Traits<Dense>::deallocate(m_DenseAlloc, m_Dense, m_Capacity);

			for (auto it = m_Storage; it != m_Storage + m_Size; it++)
			{
				Traits<Storage>::destroy(m_StorageAlloc, it);
			}
			Traits<Storage>::deallocate(m_StorageAlloc, m_Storage, m_Capacity);

			m_Dense = nullptr;
			m_Storage = nullptr;
			m_Capacity = 0;
			m_Size = 0;
		}
	}
}
