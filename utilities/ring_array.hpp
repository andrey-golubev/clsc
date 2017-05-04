#ifndef _RING_ARRAY_HPP_
#define _RING_ARRAY_HPP_

#include <array>
#include <vector>
#include <stdexcept>

namespace clsc
{
    //TODO: m_head - m_tail > 0 - always, except init
    template<typename T, std::size_t N>
    class ring_array //TODO: think about enable_if'd comparable inheritance - add comparable version that is empty if operator() is undefined
    {
    public:
        ring_array() : m_head(0), m_tail(0), m_capacity(N) {}

        ring_array(const std::initializer_list<T>& list) : m_head(0), m_tail(0), m_capacity(N), m_array(list) {}

        size_t size() const
        {
            return static_cast<size_t>(std::abs(m_head - m_tail));
        }

        size_t capacity() const
        {
            return m_capacity;
        }

        bool empty() const
        {
            return m_array.empty();
        }

        const T* data() const
        {
            return m_array.data();
        }

        void push_back(const T& val)
        {
            if (m_tail == m_head && size() != 0)
            {
                m_head = (m_head + 1) % m_capacity;
            }
            m_array[m_tail] = val;
            m_tail = (m_tail + 1) % m_capacity;
        }

        const T& front()
        {
            return m_array[m_head];
        }

        const T& move_front()
        {
            // TODO: cannot out_of_range because of std::array?
            if (size() == 0 && m_array.size() == 0)
                throw std::out_of_range("array is empty");

            const auto& val = m_array[m_head];
            m_head = (m_head + 1) % m_capacity;
            m_tail = (m_tail + 1) % m_capacity;
            return val;
        }

        const T& back()
        {
            const auto& pos = (m_tail + m_capacity -1) % m_capacity; // TODO: check pos calculation
            return m_array[pos];
        }

        const T& move_back()
        {
            // TODO: cannot out_of_range because of std::array?
            if (size() == 0 && m_array.size() == 0)
                throw std::out_of_range("array is empty");

            const auto& pos = (m_tail + m_capacity -1) % m_capacity; // TODO: check pos calculation
            m_tail > 0 ? m_tail-- : m_tail = m_capacity - ((m_tail + 1) % m_capacity);
            m_head > 0 ? m_head-- : m_capacity - ((m_head + 1) % m_capacity);
            return m_array[pos];
        }

        T& operator[](int index)
        {
            if ((index >= size() && index >= m_array.size()) || index < 0)
                throw std::out_of_range("invalid index");
            return m_array[(m_head + index) % m_capacity];
        }

        T at(int index)
        {
            if ((index >= size() && index >= m_array.size()) || index < 0)
                throw std::out_of_range("invalid index");
            return m_array[(m_head + index) % m_capacity];
        }

    private:
        int32_t m_head, m_tail;
        const size_t m_capacity;
        std::array<T, N> m_array;
    };
}

#endif /* _RING_ARRAY_HPP_ */
