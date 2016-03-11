### shared_allocator
SharedAllocator is simple allocator that allocates memory inside one shared library and can be used to make it safe
to share STL containers between several shared libraries.
SharedAllocator interface is similar to `std::allocator` interface
so `std::allocator` can be easily replaced by `salloc::shared_allocator` (or by other `salloc::` allocators).

### adding shared_allocator to your project with CMake
1. Copy files include/shared_allocator/`*.*` to your include directory 
    (for example, include/third_party/shared_allocator or just include/shared_allocator).
2. Copy files source/`*.*` to your sources directory (for example, source/shared_allocator).
3. Include source/shared_allocator/CMakeLists.txt into your main CMakeLists.txt: `add_subdirectory(source/shared_allocator)`
4. Add path to shared_allocator header files into additional include directories: `include_directories(include/third_party)`
    to make it possible for your compiler to find includes like `#include "shared_allocator/shared_allocator.hpp"`.
5. Add link to shared_allocator library into every project's CMakeLists.txt that would be using shared_allocator<T>
    (or shared_allocate function and so on): `target_link_libraries(${PROJECT_NAME} shared_allocator)`

### example code
```
#include "shared_allocator/shared_allocator.hpp"
#include <vector>

int main()
{
    // This vector can be safely shared among several shared libraries.
    std::vector<int, salloc::shared_allocator<int> > shared_vector;
    
    shared_vector.reserve(100);
    for (int i = 0; i < 100; ++i)
    {
        shared_vector.push_back(i);
    }
    
    return 0;
}
```

### allocators
- **salloc::shared_allocator** is simple allocator that allocates and deallocates memory using imported functions `shared_allocate`
    and `shared_deallocate`. All instances of this allocator will allocate and deallocate memory inside one heap.
- **salloc::cached_allocator** is allocator that caches deallocated memory (stores pointers to previously allocated memory for further usage).
    So if you allocating and deallocating memory buffers **of the same size** rapidly you will have a very small number
    of real memory allocations, because once allocated, this memory buffer will be reused for many times.
    It is best suited for containers that allocating and deallocating memory buffers of the same size
    (for example, `std::list`, `std::set`, `std::map`). It is not so good for `std::vector` because it needs memory allocations of
    different size, but it should not impact performance if you will use `salloc::cached_allocator` instead of `salloc::shared_allocator`
    for `std::vector`.
- **salloc::size_cached_allocator** works like `salloc::cached_allocator` but it have an individual cache for each size of memory buffer
    (pointers for deallocated memory buffers of each size are stored in individual lists. For example, 3 pointers of size 100 will be
    stored in one list and 5 pointers of size 60 will be stored in another list).
    It is not recommended to use this allocator with individual containers, it is better to share this allocator between several
    containers at once.

# LICENSE
SharedAllocator is licensed under terms of GNU GPL v3 (see file LICENSE), but I plan to add more permissive licenses in the near future (LGPL or MIT licenses maybe). Send me a message if you are interested.
