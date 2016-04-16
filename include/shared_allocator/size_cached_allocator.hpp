/***************************************************************************************
* file        : size_cached_allocator.hpp
* data        : 2016/03/05
* author      : Victor Zarubkin
* contact     : v.s.zarubkin@gmail.com
* copyright   : Copyright (C) 2016 Victor Zarubkin
*             :
* description : This header contains definition of size_cached_allocator class
*             : which can be used to speed-up rapid allocations and deallocations
*             : of memory buffers (arrays of objects) of the same size.
*             : It has individual cache for each size.
*             :
* references  : Original (and actual) version of source code can be found
*             : here <http://www.github.com/cas4ey/shared_allocator>.
*             :
* license     : This file is part of SharedAllocator.
*             : 
*             : The MIT License (MIT)
*             : Permission is hereby granted, free of charge, to any person
*             : obtaining a copy of this software and associated documentation
*             : files (the "Software"), to deal in the Software without
*             : restriction, including without limitation the rights to use,
*             : copy, modify, merge, publish, distribute, sublicense, and/or sell
*             : copies of the Software, and to permit persons to whom the Software
*             : is furnished to do so, subject to the following conditions:
*             : 
*             : The above copyright notice and this permission notice shall be
*             : included in all copies or substantial portions of the Software.
*             : 
*             : THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
*             : EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
*             : OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*             : IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
*             : ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*             : TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
*             : OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
****************************************************************************************/

#ifndef SHARED___ALLOCATOR__CACHED_ALLOCATOR___HPP___
#define SHARED___ALLOCATOR__CACHED_ALLOCATOR___HPP___

#include "shared_allocator/shared_allocator.hpp"
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace salloc {

    /** \brief Allocator that caches memory for further usage.
    
    It caches deallocated memory buffers of any size (it has individual cache for each size) and uses them later
    to provide fast allocation.
    
    \note You will benefit from using this allocator only if you allocate and deallocate arrays of the same size frequently,
    but if you will use it with growing containers (for example, std::vector) you will just waste memory for caches of smaller
    size that will not be used any more.
    
    \ingroup salloc */
    template <class T, class TAlloc = shared_allocator<T> >
    class size_cached_allocator
    {
    protected:

        template <class U>
        using TSameAlloc = typename TAlloc::template rebind<U>::other;

        typedef ::std::vector<T*, TSameAlloc<T*> >                          MemoryCache;
        typedef ::std::vector<MemoryCache*, TSameAlloc<MemoryCache*> >        SizeCache;
        typedef TSameAlloc<MemoryCache>                                MemoryCacheAlloc;

        SizeCache    m_memoryCache; ///< Memory cache of objects (including arrays)
        const TAlloc   m_allocator; ///< Allocator to allocate new instances of T

    public:

        typedef T                         value_type;

        typedef value_type*                  pointer;
        typedef const value_type*      const_pointer;
        typedef void*                   void_pointer;
        typedef const void*       const_void_pointer;

        typedef value_type&                reference;
        typedef const value_type&    const_reference;

        typedef size_t                     size_type;
        typedef ptrdiff_t            difference_type;

        /** \brief Auxiliary struct to convert this type to cached_allocator of other type. */
        template <class U>
        struct rebind
        {
            typedef size_cached_allocator<U> other;
        };

        /** \brief Returns an actual adress of value.

        \note Uses std::addressof */
        pointer address(reference _value) const throw()
        {
            return ::std::addressof(_value);
        }

        /** \brief Returns an actual adress of const value.

        \note Uses std::addressof */
        const_pointer address(const_reference _value) const throw()
        {
            return ::std::addressof(_value);
        }

        /** \brief Default constructor.

        Does nothing. */
        size_cached_allocator() throw()
        {
        }

        /** \brief Empty copy constructor.

        Does nothing. */
        template <class U>
        size_cached_allocator(const size_cached_allocator<U>&) throw()
        {
        }

        /** \brief Move constructor.

        Moves reserved memory. */
        size_cached_allocator(size_cached_allocator<T>&& _rvalueAlloc) : m_memoryCache(::std::move(_rvalueAlloc.m_memoryCache))
        {
        }

        /** \brief Empty assignment operator.

        Does nothing and returns reference to this allocator. */
        template <class U>
        size_cached_allocator<T>& operator=(const size_cached_allocator<U>&)
        {
            return *this;
        }

        ~size_cached_allocator()
        {
            for (auto pCache : m_memoryCache)
            {
                if (pCache != nullptr)
                {
                    for (auto pMem : *pCache)
                    {
                        m_allocator.deallocate(pMem);
                    }

                    static const MemoryCacheAlloc memAlloc;
                    memAlloc.destroy(pCache);
                    memAlloc.deallocate(pCache);
                }
            }
        }

        /** \brief Swaps two allocators with their cahce.

        \param _anotherAlloc Reference to another allocator. */
        void swap(size_cached_allocator<T>& _anotherAlloc)
        {
            m_memoryCache.swap(_anotherAlloc.m_memoryCache);
        }

        void reserve_size(size_type _maxSize)
        {
            if (_maxSize >= m_memoryCache.size())
            {
                m_memoryCache.resize(_maxSize + 1, nullptr);
            }
        }

        void reserve(size_type _arraySize, size_type _reservationsNumber)
        {
            reserve_size(_arraySize);

            auto& pCache = m_memoryCache[_arraySize];
            if (pCache == nullptr)
            {
                static const MemoryCacheAlloc memAlloc;
                auto newCache = memAlloc.allocate(1);
                memAlloc.construct(newCache);
                newCache->reserve(4);
                pCache = newCache;
            }

            pCache->reserve(pCache->size() + _reservationsNumber);
            for (size_type i = 0; i < _reservationsNumber; ++i)
            {
                pCache->push_back(m_allocator.allocate(_arraySize));
            }
        }

        /** \brief Returns number of elements in allocated memory.

        \param _memory Pointer to allocated memory.

        \warning Please, notice that the number of ELEMENTS will be returned (not number of BYTES).

        \note This function made template to avoid compiler errors for allocators that do not have size() function. */
        template <class U>
        inline size_type size(const U* _memory) const
        {
            return m_allocator.size(_memory);
        }

        /** \brief Stores pointer to memory in cache to be used later.

        \param _memory Pointer to allocated memory. */
        void deallocate(pointer _memory, size_type = 0)
        {
            const size_type elementsNumber = m_allocator.size(_memory);

            reserve_size(elementsNumber);

            auto& pCache = m_memoryCache[elementsNumber];
            if (pCache == nullptr)
            {
                static const MemoryCacheAlloc memAlloc;
                auto newCache = memAlloc.allocate(1);
                memAlloc.construct(newCache);
                newCache->reserve(4);
                pCache = newCache;
            }

            pCache->push_back(_memory);
        }

        /** \brief Truly deallocates memory.

        \param _memory Pointer to allocated memory. */
        inline void deallocate_force(pointer _memory, size_type = 0) const
        {
            m_allocator.deallocate(_memory);
        }

        /** \brief Allocate array of elements.

        \param _number Required number of elements in array. */
        pointer allocate(size_type _number) const
        {
            if (_number >= m_memoryCache.size())
            {
                return m_allocator.allocate(_number);
            }

            auto memCache = m_memoryCache[_number];
            if (memCache == nullptr || memCache->empty())
            {
                return m_allocator.allocate(_number);
            }

            pointer pMem = memCache->back();
            memCache->pop_back();
            return pMem;
        }

        /** \brief Allocate array of elements ignoring hint.

        \param _number Required number of elements in array. */
        inline pointer allocate(size_type _number, void*) const
        {
            return allocate(_number);
        }

        /** \brief Allocate actually one element. */
        inline pointer allocate()
        {
            return allocate(1);
        }

        /** \brief Construct new object on preallocated memory using default constructor.

        \param _singleObject Pointer to preallocated memory. */
        inline void construct(T* _singleObject) const
        {
            shared_construct(_singleObject);
        }

        /** \brief Construct new object on preallocated memory using copy-constructor.

        \param _singleObject Pointer to preallocated memory.
        \param _value Const-reference to another object instance to be coped from.

        \note Declared as template function to make it possible to use this allocator with
        types without public copy-constructor. */
        template <class U>
        inline void construct(U* _singleObject, const U& _value) const
        {
            shared_construct(_singleObject, _value);
        }

        /** \brief Construct new object on preallocated memory using move-constructor.

        \param _singleObject Pointer to preallocated memory.
        \param _value Rvalue-reference to another object instance to be moved from.

        \note Declared as template function to make it possible to use this allocator with
        types without public move-constructor. */
        template <class U>
        inline void construct(U* _singleObject, U&& _value) const
        {
            shared_construct(_singleObject, _value);
        }

        /** \brief Construct new object on preallocated memory using arguments list.

        \param _singleObject Pointer to preallocated memory.
        \param _constructorArguments Variadic arguments list to be used by object constructor.

        \note Declared as template function to make it possible to use this allocator with
        types without specific constructor with arguments. */
        template <class U, class... TArgs>
        inline void construct(U* _singleObject, TArgs&&... _constructorArguments) const
        {
            ::new (static_cast<void*>(_singleObject)) U(::std::forward<TArgs>(_constructorArguments)...);
        }

        /** \brief Destroy pointed object.

        Invokes object's destructor.

        \param _singleObject Pointer to object.

        \note Declared as template function to make it possible to use this allocator with
        types without public destructor. */
        template <class U>
        inline void destroy(U* _singleObject) const
        {
            _singleObject->~U();
        }

        /** \brief Estimate maximum array size. */
        size_type max_size() const throw()
        {
            return (size_type)(-1) / sizeof(T);
        }

    }; // END class cached_allocator<T>.

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Using cached_allocator for itself is restricted.
    template <class T, class U>
    class size_cached_allocator<T, size_cached_allocator<U> >;

} // END namespace salloc.

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SHARED___ALLOCATOR__CACHED_ALLOCATOR___HPP___
