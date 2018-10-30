// Copyright 2018 Andrey Golubev
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// ANDANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
// IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    using allocator_size_type = std::size_t; // TODO: use 64 bit ints instead of size_t ?
    struct block
    {
        void* data;
        allocator_size_type size;
        bool operator==(const block& other) const
        {
            return this->size == other.size && this->data == other.data;
        }
    };

    class null_allocator
    {
    public:
        block allocate(allocator_size_type) { return {nullptr, 0}; }
        void deallocate(block& blk) { assert(blk.data == nullptr); }
        bool owns(const block& blk) { return blk.data == nullptr; }
    };

    class malloc_allocator
    {
    public:
        block allocate(allocator_size_type size)
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
        block allocate(allocator_size_type size)
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
            if (PrimaryAllocator::owns(blk))
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

    namespace __internal
    {
        constexpr static allocator_size_type __default_stack_alloc_size = 8192;
    }
    template<allocator_size_type container_size>
    class stack_allocator
    {
        using single_byte = char;
        single_byte m_storage[container_size];
        single_byte* m_pos;

        int align(allocator_size_type s)
        {
            auto power = std::ceil(std::log2(s));
            power = power > 0 ? power : 1;
            return std::pow(2, power);
        }
    public:
        stack_allocator() : m_pos(m_storage) {}
        block allocate(allocator_size_type size)
        {
            const auto aligned_size = align(size);
            if (aligned_size > (m_storage + container_size) - m_pos)
            {
                return {nullptr, 0};
            }
            block blk = {m_pos, size};
            m_pos += aligned_size;
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
            return blk.data >= m_storage && blk.data < m_storage + container_size;
        }
    };


    // defines:
    using default_allocator = fallback_allocator<malloc_allocator, null_allocator>;
    using simple_allocator = fallback_allocator<stack_allocator<__internal::__default_stack_alloc_size>, malloc_allocator>;
}

#endif  // _MEMORY_CONCEPTS_HPP_
