#pragma once

#include "Common/Types.h"

namespace Flies
{
    template<typename T>
    class Span
    {
    public:
        Span() = default;
        Span(std::nullptr_t) {}
        Span(T* ptr, size_type count) : m_Ptr(ptr), m_Count(count) {}
        Span(T* begin, T* end) : m_Ptr(begin), m_Count(end - begin) {}

        T* Data() { return m_Ptr; }
        const T* Data() const { return m_Ptr; }
        size_type Size() const { return m_Ptr; }

        T* begin() { return m_Ptr; }
        const T* begin() const { return m_Ptr; }
        const T* cbegin() const { return m_Ptr; }

        T* end() { return m_Ptr + m_Count; }
        const T* end() const { return m_Ptr + m_Count; }
        const T* cend() const { return m_Ptr + m_Count; }

    private:
        T* m_Ptr = nullptr;
        size_type m_Count = 0;
    };
} // namespace Flies
