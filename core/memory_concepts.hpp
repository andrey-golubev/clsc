#ifndef _MEMORY_CONCEPTS_HPP_
#define _MEMORY_CONCEPTS_HPP_

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <vector>
#include <algorithm>

namespace clsc
{
    using block = void*;

    class null_allocator
    {
    public:
        block allocate(std::size_t) { return nullptr; }
        void deallocate(block blk) { assert(blk == block(nullptr)); }
        bool owns(block blk) { return blk == block(nullptr); }
    };

    // TODO: support basic malloc allocator
    class malloc_allocator
    {
    public:
        block allocate(std::size_t size)
        {
            block blk = block(std::malloc(size));
            m_owned_data.push_back(blk);
            return blk;
        }
        void deallocate(block blk)
        {
            free(blk);
            blk = block(nullptr);
        }
        // temporary to have owns here
        bool owns(block blk)
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
        block allocate(std::size_t size)
        {
            block blk = PrimaryAllocator::allocate(size);
            if (!blk) // !blk.ptr
            {
                blk = FallbackAllocator::allocate(size);
            }
            return blk;
        }
        void deallocate(block blk)
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
        bool owns(block blk)
        {
            return PrimaryAllocator::owns(blk) || FallbackAllocator::owns(blk);
        }
    };
}

#endif
