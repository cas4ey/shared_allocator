/*
*
*/

#include <iostream>
#include <list>
#include <chrono>
#include "shared_allocator/shared_allocator.hpp"
#include "shared_allocator/cached_allocator.hpp"
#include "shared_allocator/size_cached_allocator.hpp"
#include <string.h>
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool test1()
{
    const salloc::shared_allocator<int> alloc;

    int* ints = alloc.allocate(100);
    if (ints == nullptr)
    {
        return false;
    }

    if (alloc.size(ints) != 100)
    {
        return false;
    }

    for (int i = 0; i < 100; ++i)
    {
        alloc.construct(ints + i, i + 1);
    }

    for (int i = 99; i > -1; --i)
    {
        if (ints[i] != i + 1)
        {
            return false;
        }
    }

    alloc.deallocate(ints);

    return true;
}

//////////////////////////////////////////////////////////////////////////

bool test2()
{
    salloc::cached_allocator<int> alloc;

    int* ints = alloc.allocate();
    if (ints == nullptr)
    {
        return false;
    }

    if (alloc.size(ints) != 1)
    {
        return false;
    }

    alloc.construct(ints, 333);
    if (*ints != 333)
    {
        return false;
    }

    alloc.deallocate(ints);

    int* int2 = alloc.allocate();
    if (int2 == nullptr)
    {
        return false;
    }

    if (ints != int2)
    {
        return false;
    }

    int* int3 = alloc.allocate();
    if (int3 == nullptr)
    {
        return false;
    }

    if (ints == int3 || int2 == int3)
    {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////

bool test3()
{
    static const int observations_number = 100000;
    static const int array_size = 1000;
    static const double observation_freq = 1.0 / double(observations_number);

    const salloc::shared_allocator<double> alloc1;
    std::allocator<double> alloc2;

    std::cout << std::endl;

    double shared_alloc_time = 0, shared_dealloc_time = 0;
    double std_alloc_time = 0, std_dealloc_time = 0;

    double** pointers = new double*[observations_number];
    memset(pointers, 0, observations_number * sizeof(double*));

    typedef decltype(std::chrono::high_resolution_clock::now()) Time;
    Time t1, t2;

    auto shared_alloc_dealloc = [&](int i)
    {
        t1 = std::chrono::high_resolution_clock::now();
        pointers[i] = alloc1.allocate(array_size);
        t2 = std::chrono::high_resolution_clock::now();

        shared_alloc_time += double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        t1 = std::chrono::high_resolution_clock::now();
        alloc1.deallocate(pointers[i], array_size);
        t2 = std::chrono::high_resolution_clock::now();

        shared_dealloc_time += double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    };

    auto std_alloc_dealloc = [&](int i)
    {
        t1 = std::chrono::high_resolution_clock::now();
        pointers[i] = alloc2.allocate(array_size);
        t2 = std::chrono::high_resolution_clock::now();

        std_alloc_time += double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        t1 = std::chrono::high_resolution_clock::now();
        alloc2.deallocate(pointers[i], array_size);
        t2 = std::chrono::high_resolution_clock::now();

        std_dealloc_time += double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    };

    int n = 0;
    for (int i = 0; i < observations_number; ++i)
    {
        shared_alloc_dealloc(i);
        std_alloc_dealloc(i);
        ++n;

        shared_alloc_dealloc(i);
        std_alloc_dealloc(i);
        ++n;
    }

    const double one_to_n = 1.0 / double(n);

    shared_alloc_time *= one_to_n;
    shared_dealloc_time *= one_to_n;

    std_alloc_time *= one_to_n;
    std_dealloc_time *= one_to_n;

    std::cout << "shared_allocator<double>  allocate(" << array_size << ") time = " << shared_alloc_time << " ns  ";
    std::cout << "deallocate(" << array_size << ") time = " << shared_dealloc_time << " ns\n";

    std::cout << "std::allocator<double>  allocate(" << array_size << ") time = " << std_alloc_time << " ns  ";
    std::cout << "deallocate(" << array_size << ") time = " << std_dealloc_time << " ns\n";

    delete[] pointers;

    return true;
}

//////////////////////////////////////////////////////////////////////////

bool test4()
{
    static const int observations_number = 100000;
    static const int array_size = 1000;
    static const double observation_freq = 1.0 / double(observations_number);

    salloc::cached_allocator<double> alloc1;
    std::allocator<double> alloc2;

    std::cout << std::endl;

    double shared_alloc_time = 0, shared_dealloc_time = 0;
    double std_alloc_time = 0, std_dealloc_time = 0;

    double** pointers = new double*[observations_number];
    memset(pointers, 0, observations_number * sizeof(double*));

    typedef decltype(std::chrono::high_resolution_clock::now()) Time;
    Time t1, t2;

    auto shared_alloc_dealloc = [&](int i)
    {
        t1 = std::chrono::high_resolution_clock::now();
        pointers[i] = alloc1.allocate(array_size);
        t2 = std::chrono::high_resolution_clock::now();

        shared_alloc_time += double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        t1 = std::chrono::high_resolution_clock::now();
        alloc1.deallocate(pointers[i], array_size);
        t2 = std::chrono::high_resolution_clock::now();

        shared_dealloc_time += double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    };

    auto std_alloc_dealloc = [&](int i)
    {
        t1 = std::chrono::high_resolution_clock::now();
        pointers[i] = alloc2.allocate(array_size);
        t2 = std::chrono::high_resolution_clock::now();

        std_alloc_time += double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());

        t1 = std::chrono::high_resolution_clock::now();
        alloc2.deallocate(pointers[i], array_size);
        t2 = std::chrono::high_resolution_clock::now();

        std_dealloc_time += double(std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count());
    };

    alloc1.reserve(array_size, 1);

    int n = 0;
    for (int i = 0; i < observations_number; ++i)
    {
        shared_alloc_dealloc(i);
        std_alloc_dealloc(i);
        ++n;

        shared_alloc_dealloc(i);
        std_alloc_dealloc(i);
        ++n;
    }

    const double one_to_n = 1.0 / double(n);

    shared_alloc_time *= one_to_n;
    shared_dealloc_time *= one_to_n;

    std_alloc_time *= one_to_n;
    std_dealloc_time *= one_to_n;

    std::cout << "cached_allocator<double>  allocate(" << array_size << ") time = " << shared_alloc_time << " ns  ";
    std::cout << "deallocate(" << array_size << ") time = " << shared_dealloc_time << " ns\n";

    std::cout << "std::allocator<double>  allocate(" << array_size << ") time = " << std_alloc_time << " ns  ";
    std::cout << "deallocate(" << array_size << ") time = " << std_dealloc_time << " ns\n";

    delete[] pointers;

    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

typedef bool(*pTest)();

const pTest tests[] = {
    test1,
    test2,
    test3,
    test4
};

//////////////////////////////////////////////////////////////////////////

int main()
{
    std::list<int> l1;
    std::list<int, salloc::shared_allocator<int> > l2;
    std::list<int, salloc::cached_allocator<int> > l3;



    std::cout << "Begin testing...\n";

    int result = 0;

    int i = 0;
    for (auto test : tests)
    {
        ++i;

        std::cout << "--- Test" << i << ": ";
        if (!test())
        {
            result = -1;
            std::cout << "FAILED!";
        }
        else
        {
            std::cout << "OK!";
        }

        std::cout << std::endl;
    }

    std::cout << "Testing complete.\nInput something to exit: ";

    char c = 0;
    std::cin >> c;

    return result;
}
