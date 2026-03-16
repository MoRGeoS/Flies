#pragma once

#include <cstdint>
#include <limits>
#include <memory>

#include <Flies/Common/Types.h>

namespace Flies
{
	class SparseSet
	{
	public:
		using ID = uint32_t;
		using Index = uint32_t;

		static constexpr Index NULL_INDEX = std::numeric_limits<Index>::max();

	public:
		SparseSet() = default;

		SparseSet(const SparseSet& other);
		SparseSet(SparseSet&& other) noexcept;

		SparseSet& operator=(const SparseSet& other);
		SparseSet& operator=(SparseSet&& other) noexcept;

		~SparseSet();

		// METHODS

		void Insert(ID id, Index index);
		void Remove(ID id);

		inline bool Contains(ID id) const;

		Index Get(ID id) const;
		Index operator[](ID id) const;

		ID* begin() { return m_Sparse; }
		const ID* begin() const { return m_Sparse; }
		const ID* cbegin() const { return m_Sparse; }

		ID* end() { return m_Sparse + m_Capacity; }
		const ID* end() const { return m_Sparse + m_Capacity; }
		const ID* cend() const { return m_Sparse + m_Capacity; }

		size_type Size() const { return m_Capacity; }
		void Cleanup();

	private:
		using Sparse = Index;

		using Alloc = std::allocator<Index>;
		using Traits = std::allocator_traits<Alloc>;

		void Grow(size_type desiredCapacity);

	private:
		Sparse* m_Sparse = nullptr;
		size_type m_Capacity = 0;

		Alloc m_Alloc;
	};
}
