#pragma once

#include <limits>
#include <memory>

#include <Flies/Common/Types.h>
#include <Flies/Entity.h>
#include <Flies/Span.h>

namespace Flies
{
	class Sparse
	{
	public:
		static constexpr size_type NULL_INDEX = ::Flies::MAX_SIZE;

	public:
		Sparse() = default;

		Sparse(const Sparse& other);
		Sparse(Sparse&& other) noexcept;

		Sparse& operator=(const Sparse& other);
		Sparse& operator=(Sparse&& other) noexcept;

		~Sparse();

		void Insert(EntityID id, size_type index);
		void Remove(EntityID id);

		bool Contains(EntityID id) const;

		size_type Get(EntityID id) const;
		size_type operator[](EntityID id) const;

		void Cleanup();
		
		
		EntityID* begin() { return m_Data; }
		const EntityID* begin() const { return m_Data; }
		const EntityID* cbegin() const { return m_Data; }
		
		EntityID* end() { return m_Data + m_Size; }
		const EntityID* end() const { return m_Data + m_Size; }
		const EntityID* cend() const { return m_Data + m_Size; }
		
		EntityID* Data() { return m_Data; }
		const EntityID* Data() const { return m_Data; }

		size_type Size() const { return m_Size; }

		Flies::Span<EntityID> Span() { return Flies::Span<EntityID>(m_Data, m_Size); }
		Flies::Span<const EntityID> Span() const { return Flies::Span<const EntityID>(m_Data, m_Size); }

	private:
		using Alloc = std::allocator<size_type>;
		using Traits = std::allocator_traits<Alloc>;

		void Grow(size_type desiredCapacity);

	private:
		size_type* m_Data = nullptr;
		size_type m_Size = 0;

		Alloc m_Alloc;
	};
}
