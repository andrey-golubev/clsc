#ifndef _RING_ARRAY_HPP_
#define _RING_ARRAY_HPP_

#include <array>
#include <stdexcept>

template<typename T, std::size_t N>
class ring_buffer
{
    ring_buffer() = delete;
public:
    ring_buffer(const size_t size) :
        m_capacity(size),
        m_size(0),
        m_head(0),
        m_tail(0)
    {
        if (size <= 1)
        {
            throw std::invalid_argument("illegal size");
        }
        m_array = new T[size];
    }

    ~ring_buffer()
    {
        delete[] m_array;
    }

//    const size_t size() const
//    {
//        return static_cast<size_t>(std::abs(static_cast<int>(m_head - m_tail)));
//    }

    const size_t capacity() const
    {
        return m_capacity;
    }

//    const bool empty() const
//    {
//        return (m_head == m_tail) ? false : true;
//    }

//    const T* data() const
//    {
//        return m_array;
//    }

    void push_back(const T& val)
    {
        m_tail = (m_tail + 1) % m_capacity;
        m_array[m_tail] = val;
        if (m_tail == m_head)
        {
            m_head = (m_head + 1) % m_capacity;
        }
    }

    const T front() const
    {
        auto val = m_array[m_head];
        m_tail = m_head;
        m_head = (m_head + 1) % m_capacity;
        return val;
    }

    const T back() const
    {
        auto val = m_array[m_tail];
        m_tail = (m_tail + 1) % m_capacity;
        if (m_tail == m_head)
        {
            m_head = (m_head + 1) % m_capacity;
        }
        return val;
    }

//    const T at(const int index) const
//    {
//        if (index >= m_size)
//        {
//            throw std::out_of_range("index out of range");
//        }
//        return m_array[index];
//    }

//    T& operator[](int index) const
//    {
//        if (index >= m_capacity)
//        {
//            throw std::out_of_range("index out of range");
//        }
//        return m_array[index];
//    }

private:
    T* m_array;
    const size_t m_capacity;
    size_t m_size;

    std::array<T, N> m_array1;

    uint32_t m_head, m_tail;
};

#endif /* _RING_ARRAY_HPP_ */
