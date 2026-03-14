#pragma once

#include <atomic>
#include <string_view>
#include <cstdint>

#include <Flies/Common/Types.h>

namespace Flies
{
    namespace
    {
        inline std::atomic<size_type> SeqCounter = 0;
        constexpr uint64_t FNV_OFFSET = 0x811C9DC5;
        constexpr uint64_t FNV_PRIME = 0x01000193;
    }

    template<typename T>
    struct TypeSeq
    {
        static size_type value() noexcept
        {
            static const size_type index = SeqCounter.fetch_add(1, std::memory_order_relaxed);
            return index;
        }
    };

    template<typename T>
    consteval std::string_view TypeName() noexcept
    {
#if defined(__clang__) || defined(__GNUC__)
        std::string_view pretty = __PRETTY_FUNCTION__;
        auto start = pretty.find("T = ") + 4;
        auto end = pretty.rfind(']');
#elif defined(_MSC_VER)
        std::string_view pretty = __FUNCSIG__;
        auto start = pretty.find("TypeName<") + 9;
        auto end = pretty.rfind('>');
#else
#   error "Unsupported compiler"
#endif
        return pretty.substr(start, end - start);
    }

    template<typename T>
    consteval size_type TypeHash() noexcept
    {
        constexpr std::string_view name = TypeName<T>();

        size_type hash = FNV_OFFSET;
        size_type prime = FNV_PRIME;

        for (char c : name)
            hash = (hash ^ static_cast<size_type>(c)) * prime;

        return hash;
    }

    class TypeInfo
    {
    public:
        constexpr TypeInfo(size_type seq, size_type hash, std::string_view name) noexcept
            : m_Seq(seq), m_Hash(hash), m_Name(name) {}

        size_type seq()  const noexcept { return m_Seq; }
        size_type hash() const noexcept { return m_Hash; }
        std::string_view name() const noexcept { return m_Name; }

        bool operator==(const TypeInfo& other) const noexcept { return m_Hash == other.m_Hash; }
        bool operator!=(const TypeInfo& other) const noexcept { return m_Hash != other.m_Hash; }

    private:
        size_type m_Seq;
        size_type m_Hash;
        std::string_view m_Name;
    };

    template<typename T>
    constexpr TypeInfo TypeID() noexcept
    {
        return TypeInfo
        {
            TypeSeq<T>::value(),
            TypeHash<T>(),
            TypeName<T>()
        };
    }
}