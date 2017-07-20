#include <map>
#include <array>
#include <iostream>
#include <stdexcept>

#include <gtest/gtest.h>

template<typename Callable>
void cout_printer(int times, Callable function_object)
{
    for (int i = 0; i < times; i++)
    {
        std::cout << function_object(i) << " ";
    }
    std::cout << std::endl;
}

template<typename Callable, typename ...Args>
void cout_printer(Callable function_object, Args... params)
{
    std::cout << function_object(params...) << std::endl;
}



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

int64_t fibonacci_rec(int64_t n)
{
    if (n == 0) return 0;
    if (n <= 2) return 1;
    return fibonacci_rec(n - 1) + fibonacci_rec(n - 2);
}

TEST(algorithms, fibonacci)
{
    constexpr auto fibonacci = [] (int64_t n) -> int64_t
    {
        if (n == 0) return 0;
        int64_t a = 0, b = 1;
        for (int i = 1; i < n; i++)
        {
            b += a;
            a = b - a;
        }
        return b;
    };

    cout_printer(20, fibonacci);
    cout_printer(20, fibonacci_rec);
}

TEST(algorithms, combinations)
{
    constexpr auto factorial = [] (uint64_t n)
    {
        uint64_t ret = 1;
        for (int i = 2; i <= n; i++)
        {
            ret *= i;
        }
        return ret;
    };

    const auto combinations = [factorial] (uint64_t n, uint64_t k)
    {
        return factorial(n) / (factorial(k) * factorial(n - k));
    };

    cout_printer(factorial, 5);
    cout_printer(combinations, 5, 3);
}

TEST(algorithms, power_set)
{
    constexpr int64_t set[] = { 1, 2, 3 };
    constexpr size_t size = sizeof(set) / sizeof(int64_t);
    std::vector<std::vector<int64_t>> power_set;

    for (int i = 0; i < size; i++)
        for (int j = i; j < size; j++)
        {
            std::vector<int64_t> subset;
            for (int k = i; k <= j; k++)
                subset.push_back(set[k]);
            power_set.push_back(subset);
        }
    power_set.push_back(std::vector<int64_t>());
    power_set.push_back(std::vector<int64_t>(set, set + size));
    for (const auto& subset : power_set)
    {
        for (const auto& value : subset)
            std::cout << value << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

TEST(algorithms, testing_something)
{
    for (int i = 1; i < 100; i++)
    {
        std::cout << "    n^3: " << i * i * i << std::endl;
        std::cout << "n * 2^n: " << i * std::pow(2, i) << std::endl;
    }
    std::cout << std::endl;
}
