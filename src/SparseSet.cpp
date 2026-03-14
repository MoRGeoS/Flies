#pragma once

#include <memory>

#include <Flies/SparseSet.h>
#include <Flies/Common/Assert.h>

namespace Flies
{
	SparseSet::SparseSet(const SparseSet& other)
	{
		if (other.m_Capacity > 0)
		{
			m_Sparse = Traits::allocate(m_Alloc, other.m_Capacity);
			FLS_ASSERT(m_Sparse, "Failed to allocate memory");

			std::uninitialized_copy_n(other.m_Sparse, other.m_Capacity, m_Sparse);
			m_Capacity = other.m_Capacity;
		}
	}

	SparseSet::SparseSet(SparseSet&& other) noexcept
	{
		if (other.m_Sparse)
		{
			// Copy values
			m_Sparse = other.m_Sparse;
			m_Capacity = other.m_Capacity;

			// Assign default values
			other.m_Sparse = nullptr;
			other.m_Capacity = 0;
		}
	}

	SparseSet& SparseSet::operator=(const SparseSet& other)
	{
		if (this != &other)
		{
			Cleanup();

			// Copy
			if (other.m_Capacity > 0)
			{
				// Allocate Sparse
				m_Sparse = Traits::allocate(m_Alloc, other.m_Capacity);
				FLS_ASSERT(m_Sparse, "Failed to allocate memory");

				// Copy Sparse
				std::uninitialized_copy_n(other.m_Sparse, other.m_Capacity, m_Sparse);
				m_Capacity = other.m_Capacity;
			}
		}
		return *this;
	}
	
	SparseSet& SparseSet::operator=(SparseSet&& other) noexcept
	{
		if (this != &other)
		{
			Cleanup();

			// Move
			if (other.m_Capacity > 0)
			{
				// Copy values
				m_Sparse = other.m_Sparse;
				m_Capacity = other.m_Capacity;

				// Assign default values
				other.m_Sparse = nullptr;
				other.m_Capacity = 0;
			}
		}
		return *this;
	}

	SparseSet::~SparseSet()
	{
		Cleanup();
	}

	void SparseSet::Insert(ID id, Index index)
	{
		if (m_Capacity <= id)
		{
			Grow(id + 1);
		}
		m_Sparse[id] = index;
	}

	void SparseSet::Remove(ID id)
	{
		if (m_Capacity > id)
		{
			m_Sparse[id] = NULL_INDEX;
		}
	}

	bool SparseSet::Contains(ID id) const
	{
		return (
			m_Capacity > id &&
			m_Sparse[id] != NULL_INDEX
			);
	}

	SparseSet::Index SparseSet::Get(ID id) const
	{
		FLS_ASSERT(m_Capacity > id, "Index out of range");
		if (Contains(id))
		{
			return m_Sparse[id];
		}
		return NULL_INDEX;
	}

	SparseSet::Index SparseSet::operator[](ID id) const
	{
		FLS_ASSERT(m_Capacity > id, "Index out of range");
		if (Contains(id))
		{
			return m_Sparse[id];
		}
		return NULL_INDEX;
	}

	void SparseSet::Grow(size_type desiredCapacity)
	{
		size_type newCapacity = std::max((size_type)20, std::max(m_Capacity * (size_type)2, desiredCapacity));

		Sparse* newSparse = Traits::allocate(m_Alloc, newCapacity);
		FLS_ASSERT(newSparse, "Failed to allocate memory");

		if (m_Sparse)
		{
			std::uninitialized_copy_n(m_Sparse, m_Capacity, newSparse);
			Traits::deallocate(m_Alloc, m_Sparse, m_Capacity);
		}

		std::uninitialized_fill_n(newSparse + m_Capacity, newCapacity - m_Capacity, NULL_INDEX);

		m_Sparse = newSparse;
		m_Capacity = newCapacity;
	}

	void SparseSet::Cleanup()
	{
		if (m_Sparse)
		{
			Traits::deallocate(m_Alloc, m_Sparse, m_Capacity);
			m_Sparse = nullptr;
			m_Capacity = 0;
		}
	}
}
