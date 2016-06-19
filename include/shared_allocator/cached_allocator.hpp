/***************************************************************************************
* file        : cached_allocator.hpp
* data        : 2016/03/05
* author      : Victor Zarubkin
* contact     : v.s.zarubkin@gmail.com
* copyright   : Copyright (C) 2016 Victor Zarubkin
*             :
* description : This header contains definition of cached_allocator class
*             : which can be used to speed-up rapid allocations and
*             : deallocations of memory buffers of the same size.
*             : It is best to use this allocator to allocate and deallocate
*             : single instances of objects (memory buffers of size 1).
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

#ifndef SHARED___ALLOCATOR__SINGLE_CACHED_ALLOCATOR___HPP___
#define SHARED___ALLOCATOR__SINGLE_CACHED_ALLOCATOR___HPP___

#include "shared_allocator/shared_allocator.hpp"
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace salloc {

    /** \brief Allocator that caches memory for objects for further usage.

    It caches deallocated memory buffers and uses them later to provide fast allocation of memory buffers of the SAME SIZE (or lesser size).
    
    \note It is better to use this allocator to allocate memory buffers of the same size.

    \note BEST TO USE IF YOU WANT TO ALLOCATE AND DEALLOCATE BUFFERS OF SIZE 1 (SINGLE INSTANCES OF OBJECTS)
    (Best fit for std::list, std::set, std::map BUT NOT for std::vector).
    
    \ingroup salloc */
    template <class T, class TAlloc = shared_allocator<T> >
    class cached_allocator
    {
    protected:

        typedef cached_allocator<T, TAlloc> ThisType;
        typedef ::std::vector<T*, typename TAlloc::template rebind<T*>::other> MemoryCache;

        MemoryCache m_memoryCache; ///< Memory cache of objects
        TAlloc        m_allocator; ///< Allocator to allocate new instances of T

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

        /** \brief Auxiliary struct to convert this type to single_cached_allocator of other type. */
        template <class U>
        struct rebind
        {
            typedef cached_allocator<U, typename TAlloc::template rebind<U>::other> other;
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
        cached_allocator() throw()
        {
        }

        /** \brief Empty copy constructor.

        Does nothing. */
        cached_allocator(const ThisType&) throw()
        {
        }

        /** \brief Empty copy constructor.

        Does nothing. */
        template <class U>
        cached_allocator(const U&) throw()
        {
        }

        /** \brief Move constructor.

        Moves reserved memory. */
        cached_allocator(ThisType&& _rvalueAlloc) : m_memoryCache(::std::move(_rvalueAlloc.m_memoryCache))
        {
        }

        /** \brief Empty assignment operator.

        Does nothing and returns reference to this allocator. */
        template <class U>
        ThisType& operator=(const U&)
        {
            return *this;
        }

        ~cached_allocator()
        {
            private_clear();
        }

        /** \brief Clear memory cache.
        
        Also deallocates all cached memory. */
        void clear()
        {
            private_clear();
            m_memoryCache.clear();
        }

        /** \brief Swaps two allocators with their cache.

        \param _anotherAlloc Reference to another allocator. */
        void swap(ThisType& _anotherAlloc)
        {
            m_memoryCache.swap(_anotherAlloc.m_memoryCache);
        }

        /** \brief Reserve memory for certain number of elements.

        Reserves memory for number of buffers of specified size.
        
        \param _arraySize Required number of elements to reserve (size of one memory buffer).
        \param _reservationsNumber Required number of memory buffers. */
        void reserve(size_type _arraySize, size_type _reservationsNumber)
        {
            m_memoryCache.reserve(m_memoryCache.size() + _reservationsNumber);
            for (size_type i = 0; i < _reservationsNumber; ++i)
            {
                m_memoryCache.push_back(m_allocator.allocate(_arraySize));
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
            m_memoryCache.push_back(_memory);
        }

        /** \brief Truly deallocates memory.

        \param _memory Pointer to allocated memory. */
        inline void deallocate_force(pointer _memory, size_type = 0)
        {
            m_allocator.deallocate(_memory, 0);
        }

        /** \brief Allocate elements.
        
        \param _number Required number of elements. */
        pointer allocate(size_type _number = 1)
        {
            if (!m_memoryCache.empty())
            {
                pointer pMem = m_memoryCache.back();
                m_memoryCache.pop_back();
                return _number < 2 ? pMem : m_allocator.allocate(_number, pMem);
            }

            return m_allocator.allocate(_number);
        }

        /** \brief Allocate elements using hint.

        \param _number Required number of elements.
        \param _currentMemory Pointer to memory allocated earlier (it contains size which will be used as a hint). */
        pointer allocate(size_type _number, void* _currentMemory)
        {
            if (!m_memoryCache.empty())
            {
                pointer pMem = m_memoryCache.back();
                m_memoryCache.pop_back();
                return _number < 2 ? pMem : m_allocator.allocate(_number, pMem);
            }

            return m_allocator.allocate(_number, _currentMemory);
        }

        /** \brief Construct new object on preallocated memory using default constructor.

        \param _singleObject Pointer to preallocated memory. */
        inline void construct(T* _singleObject)
        {
            m_allocator.construct(_singleObject);
        }

        /** \brief Construct new object on preallocated memory using copy-constructor.

        \param _singleObject Pointer to preallocated memory.
        \param _value Const-reference to another object instance to be coped from.

        \note Declared as template function to make it possible to use this allocator with
        types without public copy-constructor. */
        template <class U>
        inline void construct(U* _singleObject, const U& _value)
        {
            m_allocator.construct(_singleObject, _value);
        }

        /** \brief Construct new object on preallocated memory using move-constructor.

        \param _singleObject Pointer to preallocated memory.
        \param _value Rvalue-reference to another object instance to be moved from.

        \note Declared as template function to make it possible to use this allocator with
        types without public move-constructor. */
        template <class U>
        inline void construct(U* _singleObject, U&& _value)
        {
            m_allocator.construct(_singleObject, ::std::forward<U&&>(_value));
        }

        /** \brief Construct new object on preallocated memory using arguments list.

        \param _singleObject Pointer to preallocated memory.
        \param _constructorArguments Variadic arguments list to be used by object constructor.

        \note Declared as template function to make it possible to use this allocator with
        types without specific constructor with arguments. */
        template <class U, class... TArgs>
        inline void construct(U* _singleObject, TArgs&&... _constructorArguments)
        {
            ::new (static_cast<void*>(_singleObject)) U(::std::forward<TArgs>(_constructorArguments)...);
        }

        /** \brief Destroy pointed object.

        Invokes object's destructor.

        \param _singleObject Pointer to object.

        \note Declared as template function to make it possible to use this allocator with
        types without public destructor. */
        template <class U>
        inline void destroy(U* _singleObject)
        {
            m_allocator.destroy(_singleObject);
        }

        /** \brief Estimate maximum array size. */
        size_t max_size() const throw()
        {
            return (size_t)(-1) / sizeof(T);
        }

    protected:

        /** \brief Deallocates all memory stored in memory cache. */
        inline void private_clear()
        {
            for (auto pMem : m_memoryCache)
            {
                m_allocator.deallocate(pMem, 0);
            }
        }

    }; // END class single_cached_allocator<T>.

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Using single_cached_allocator for itself is restricted.
    template <class T, class U>
    class cached_allocator<T, ::salloc::cached_allocator<U> >;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <class T>
    using local_cached_allocator = ::salloc::cached_allocator<T, ::std::allocator<T> >;

} // END namespace salloc.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Operator == for STL compatibility. */
template <class T, class U>
inline bool operator == (const ::salloc::cached_allocator<T, U>&, const ::salloc::cached_allocator<T, U>&) throw()
{
    return true;
}

template <class T, class U, class W, class Q>
inline bool operator == (const ::salloc::cached_allocator<T, U>&, const ::salloc::cached_allocator<W, Q>&) throw()
{
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace std {

    template <class T, class TAlloc>
    inline void swap(::salloc::cached_allocator<T, TAlloc>& _left, ::salloc::cached_allocator<T, TAlloc>& _right)
    {
        _left.swap(_right);
    }

} // END namespace std.

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SHARED___ALLOCATOR__SINGLE_CACHED_ALLOCATOR___HPP___
