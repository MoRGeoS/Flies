#pragma once

#include "Flies/Common/Assert.h"
#include "Flies/Sparse.h"

#include <memory>

namespace Flies
{
	Sparse::Sparse(const Sparse& other)
	{
		if (other.m_Size > 0)
		{
			m_Data = Traits::allocate(m_Alloc, other.m_Size);
			FLS_ASSERT(m_Data, "Failed to allocate memory");

			std::uninitialized_copy_n(other.m_Data, other.m_Size, m_Data);
			m_Size = other.m_Size;
		}
	}

	Sparse::Sparse(Sparse&& other) noexcept
	{
		if (other.m_Data)
		{
			// Copy values
			m_Data = other.m_Data;
			m_Size = other.m_Size;

			// Assign default values
			other.m_Data = nullptr;
			other.m_Size = 0;
		}
	}

	Sparse& Sparse::operator=(const Sparse& other)
	{
		if (this != &other)
		{
			Cleanup();

			// Copy
			if (other.m_Size > 0)
			{
				// Allocate Sparse
				m_Data = Traits::allocate(m_Alloc, other.m_Size);
				FLS_ASSERT(m_Data, "Failed to allocate memory");

				// Copy Sparse
				std::uninitialized_copy_n(other.m_Data, other.m_Size, m_Data);
				m_Size = other.m_Size;
			}
		}
		return *this;
	}
	
	Sparse& Sparse::operator=(Sparse&& other) noexcept
	{
		if (this != &other)
		{
			Cleanup();

			// Move
			if (other.m_Size > 0)
			{
				// Copy values
				m_Data = other.m_Data;
				m_Size = other.m_Size;

				// Assign default values
				other.m_Data = nullptr;
				other.m_Size = 0;
			}
		}
		return *this;
	}

	Sparse::~Sparse()
	{
		Cleanup();
	}

	void Sparse::Insert(EntityID id, size_type index)
	{
		if (m_Size <= id)
		{
			Grow(id + 1);
		}
		m_Data[id] = index;
	}

	void Sparse::Remove(EntityID id)
	{
		if (m_Size > id)
		{
			m_Data[id] = NULL_INDEX;
		}
	}

	bool Sparse::Contains(EntityID id) const
	{
		return (
			m_Size > id &&
			m_Data[id] != NULL_INDEX
			);
	}

	size_type Sparse::Get(EntityID id) const
	{
		FLS_ASSERT(m_Size > id, "size_type out of range");
		if (Contains(id))
		{
			return m_Data[id];
		}
		return NULL_INDEX;
	}

	size_type Sparse::operator[](EntityID id) const
	{
		FLS_ASSERT(m_Size > id, "size_type out of range");
		if (Contains(id))
		{
			return m_Data[id];
		}
		return NULL_INDEX;
	}

	void Sparse::Grow(size_type desiredCapacity)
	{
		size_type newCapacity = std::max((size_type)20, std::max(m_Size * (size_type)2, desiredCapacity));

		size_type* newSparse = Traits::allocate(m_Alloc, newCapacity);
		FLS_ASSERT(newSparse, "Failed to allocate memory");

		if (m_Data)
		{
			std::uninitialized_copy_n(m_Data, m_Size, newSparse);
			Traits::deallocate(m_Alloc, m_Data, m_Size);
		}

		std::uninitialized_fill_n(newSparse + m_Size, newCapacity - m_Size, NULL_INDEX);

		m_Data = newSparse;
		m_Size = newCapacity;
	}

	void Sparse::Cleanup()
	{
		if (m_Data)
		{
			Traits::deallocate(m_Alloc, m_Data, m_Size);
			m_Data = nullptr;
			m_Size = 0;
		}
	}
}
