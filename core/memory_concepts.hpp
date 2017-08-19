#ifndef _MEMORY_CONCEPTS_HPP_
#define _MEMORY_CONCEPTS_HPP_

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cmath>

#include <vector>
#include <algorithm>

namespace clsc
{
    using size_type = std::size_t; // TODO: use 64 bit ints instead of size_t ?
    struct block
    {
        void* data;
        size_type size;
        bool operator==(const block& other) const
        {
            return this->size == other.size && this->data == other.data;
        }
    };

    class null_allocator
    {
    public:
        block allocate(size_type) { return {nullptr, 0}; }
        void deallocate(block& blk) { assert(blk.data == nullptr); }
        bool owns(const block& blk) { return blk.data == nullptr; }
    };

    class malloc_allocator
    {
    public:
        block allocate(size_type size)
        {
            block blk = {std::malloc(size), size};
            m_owned_data.push_back(blk);
            return blk;
        }
        void deallocate(block& blk)
        {
            free(blk.data);
            blk = {nullptr, 0};
        }
        // temporary to have owns here
        bool owns(const block& blk)
        {
            return std::find(m_owned_data.cbegin(), m_owned_data.cend(), blk) != m_owned_data.cend();
        }
    private:
        std::vector<block> m_owned_data;
    };

    template<class PrimaryAllocator, class FallbackAllocator>
    class fallback_allocator : private PrimaryAllocator, private FallbackAllocator
    {
    public:
        block allocate(size_type size)
        {
            block blk = PrimaryAllocator::allocate(size);
            if (blk.data == nullptr)
            {
                blk = FallbackAllocator::allocate(size);
            }
            return blk;
        }
        void deallocate(block& blk)
        {
            if (PrimaryAllocator::owns(blk)) // primary has to have ::owns() method
            {
                PrimaryAllocator::deallocate(blk);
            }
            else
            {
                FallbackAllocator::deallocate(blk);
            }
        }
        bool owns(const block& blk)
        {
            return PrimaryAllocator::owns(blk) || FallbackAllocator::owns(blk);
        }
    };

    namespace __private
    {
        constexpr static size_type __default_stack_alloc_size = 8192;
    }
    template<size_type n>
    class stack_allocator
    {
        using single_byte = char;
        single_byte m_storage[n];
        single_byte* m_pos;

        int align(size_type s)
        {
            // bad cast and +1 - to not have issues when log2(s) returns 0
            auto power = uint64_t(std::log2(s));
            return std::pow(2, power + 1);
        }
    public:
        stack_allocator() : m_pos(m_storage) {}
        block allocate(size_type s)
        {
            const auto s_after_aligned = align(s);
            if (s_after_aligned > (m_storage + n) - m_pos)
            {
                return {nullptr, 0};
            }
            block blk = {m_pos, s};
            m_pos += s_after_aligned;
            return blk;
        }
        void deallocate(block& blk)
        {
            // only deallocate from the last position
            if ((static_cast<single_byte*>(blk.data) + align(blk.size)) == m_pos)
            {
                m_pos = static_cast<single_byte*>(blk.data);
                blk = {nullptr, 0};
            }
        }
        bool owns(const block& blk)
        {
            return blk.data >= m_storage && blk.data < m_storage + n;
        }
        void deallocate_all()
        {
            m_pos = m_storage;
        }
    };


    // defines:
    using default_allocator = fallback_allocator<malloc_allocator, null_allocator>;
    using simple_allocator = fallback_allocator<stack_allocator<__private::__default_stack_alloc_size>, malloc_allocator>;
}

#endif
