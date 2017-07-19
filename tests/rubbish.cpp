#include <map>
#include <array>
#include <iostream>
#include <stdexcept>

#include <gtest/gtest.h>


TEST(algorithms, counting_sort)
{
    const auto counting_sort = [](int* array, const std::size_t size)
    {
        std::map<int, size_t> sorter;

        //        for (std::size_t i = 0; i < size; i++)
        //        {
        //            sorter[array[i]] = 0;
        //        }

        for (std::size_t i = 0; i < size; i++)
        {
            sorter[array[i]] += 1;
        }

        std::size_t index = 0;
        for (const auto& pair : sorter)
            for (std::size_t i = 0; i < pair.second; i++)
            {
                array[index] = pair.first;
                index++;
            }
    };

    const std::size_t N = 15;
    std::array<int, N> array_to_sort =
    { 0, 1, 10, 0, 1, 1, 3, 10, 0, 1, 3, 1, 0, 3, 10 };
    counting_sort(array_to_sort.data(), N);
    for (const auto& value : array_to_sort)
        std::cout << value << " ";
    std::cout << std::endl;
}

template<typename T, std::size_t max_size>
struct stack
{
private:
    std::array<T, max_size> items{};
    std::int64_t current_size = 0;

public:
    void push(T data)
    {
        current_size++;
        if (current_size > max_size)
            throw std::runtime_error("Stack overflow");
        items[current_size - 1] = data;
    }

    T pop()
    {
        current_size--;
        if (current_size < 0)
            throw std::runtime_error("Stack underflow");
        return items[current_size];
    }

    // for compatibility with range-based for-loops
    T* begin()
    {
        return &items[0];
    }

    T* end()
    {
        return &items[current_size];
    }

    T operator++()
    {
        static std::int64_t index = 0;
        return items[index++];
    }

    std::size_t size()
    {
        return static_cast<std::size_t>(current_size);
    }
};


TEST(algorithms, stack)
{
    stack<int, 70> stack;
    for (int i = 0; i < 50; i++)
        stack.push(i);

    for (int i = 0; i < 20; i++)
        stack.pop();

    for (const auto& elem : stack)
        std::cout << elem << " ";
    std::cout << std::endl;
}


template<typename T, std::size_t max_size>
struct queue
{
private:
    std::array<T, max_size> items{};
    std::int64_t head = max_size, tail = head;
    std::int64_t current_size = 0;

public:
    void enqueue(T data)
    {
        tail = tail - 1 < 0 ? max_size - 1 : tail - 1;
        current_size++;
        if (current_size > max_size)
            throw std::runtime_error("Queue is full");
        items[tail] = data;
    }

    std::size_t size()
    {
        return static_cast<std::size_t>(current_size);
    }

    T dequeue()
    {
        head = head - 1 < 0 ? max_size - 1 : head - 1;
        current_size--;
        if (current_size < 0)
            throw std::runtime_error("Queue has no elements to dequeue");
        return items[head];
    }

    // for compatibility with range-based for-loops
//    T* begin()
//    {
//        return &items[head];
//    }

//    T* end()
//    {
//        return &items[tail];
//    }

//    T operator++()
//    {
//        static std::int64_t index = head + 1;
//        index = index - 1 < tail ? tail : index - 1;
//        return items[index];
//    }
};

TEST(algorithms, queue)
{
    queue<int, 70> queue;
    for (int i = 0; i < 50; i++)
        queue.enqueue(i);

    for (int i = 0; i < 20; i++)
        std::cout << queue.dequeue() << " ";

//    for (const auto& elem : queue)
//        std::cout << elem << " ";
    std::cout << std::endl;
}

TEST(algorithms, list)
{
}
