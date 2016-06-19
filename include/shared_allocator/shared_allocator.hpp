/***************************************************************************************
* file        : shared_allocator.hpp
* data        : 2016/03/05
* author      : Victor Zarubkin
* contact     : v.s.zarubkin@gmail.com
* copyright   : Copyright (C) 2016 Victor Zarubkin
*             :
* description : This header contains declaration of shared_allocate, shared_deallocate
*             : and shared_size functions and definition of shared_allocator class
*             : which can be used to allocate memory over different shared libraries.
*             : It is possible to replace std::allocator by this allocator to use same
*             : instance of STL container in several shared libraries.
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

#ifndef SHARED___ALLOCATOR_____HPP___
#define SHARED___ALLOCATOR_____HPP___

#include <memory>
#include <stddef.h>
#include <type_traits>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)

# define SHARED_ALLOCATOR_DLLIMPORT __declspec(dllimport)

#else

# define SHARED_ALLOCATOR_DLLIMPORT

#endif // defined(WIN32) || defined(_WIN32) || defined(_WIN64)

extern "C"
{
    /** \brief Allocate new memory.
    
    \param _bytesNumber Required bytes number including already preallocated size of _currentMemory (if exist).
    \param _currentMemory Pointer to existing memory buffer (if exist). If it's size is greater than _bytesNumber then no new memory would be allocated. */
    SHARED_ALLOCATOR_DLLIMPORT void* shared_allocate(size_t _bytesNumber, void* _currentMemory = nullptr);

    /** \brief Deallocate memory.
    
    \param _currentMemory Pointer to existing memory buffer to be deallocated.
    
    \warning Pointer to _currentMemory after this operation will be invalid. */
    SHARED_ALLOCATOR_DLLIMPORT void shared_deallocate(void* _currentMemory);

    /** \brief Returns memory size in bytes.

    \param _currentMemory Pointer to existing memory buffer. */
    SHARED_ALLOCATOR_DLLIMPORT size_t shared_size(const void* _currentMemory);

}

#undef SHARED_ALLOCATOR_DLLIMPORT

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace salloc {

    namespace {

        template <class T>
        inline void shared_construct(T* _instance) {
            ::new (static_cast<void*>(_instance)) T();
        }

        template <class T>
        inline void shared_construct(T* _instance, const T& _value) {
            ::new (static_cast<void*>(_instance)) T(_value);
        }

        template <class T>
        inline void shared_construct(T* _instance, T&& _value) {
            ::new (static_cast<void*>(_instance)) T(::std::forward<T&&>(_value));
        }

        template <class T>
        inline void shared_construct(T** _instance) {
            *_instance = nullptr;
        }

        template <class T>
        inline void shared_construct(T** _instance, T* _value) {
            *_instance = _value;
        }

#define SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(TYPENAME, DEFAULT_VALUE) \
        inline void shared_construct(TYPENAME* _instance) { *_instance = DEFAULT_VALUE; } \
        inline void shared_construct(TYPENAME* _instance, TYPENAME _value) { *_instance = _value; }

        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(bool, false)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(char, 0)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(unsigned char, 0)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(short, 0)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(unsigned short, 0)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(int, 0)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(unsigned int, 0)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(long, 0)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(unsigned long, 0)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(long long, 0LL)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(unsigned long long, 0ULL)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(float, 0.f)
        SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR(double, 0.0)

#undef SHARED_ALLOCATOR_DECLARE_DEFAULT_CONSTRUCTOR

    } // END namespace <noname>.

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** \brief Allocator that can be used for objects shared across several shared libraries.
    
    It allocates and deallocates memory in one heap (instead of allocating memory in each shared library's heap)
    there for it is safe to use this allocator for shared objects (you can use STL containers and share them between
    several shared libraries safely).
    
    \ingroup salloc */
    template <class T>
    struct shared_allocator
    {
        typedef shared_allocator<T>            other;

        typedef T                         value_type;

        typedef value_type*                  pointer;
        typedef const value_type*      const_pointer;
        typedef void*                   void_pointer;
        typedef const void*       const_void_pointer;

        typedef value_type&                reference;
        typedef const value_type&    const_reference;

        typedef size_t                     size_type;
        typedef ptrdiff_t            difference_type;

        //typedef ::std::false_type propagate_on_container_copy_assignment;
        //typedef ::std::false_type propagate_on_container_move_assignment;
        //typedef ::std::false_type propagate_on_container_swap;

        /** \brief Returns this shared_allocator.

        \note Used since C++11 */
        other select_on_container_copy_construction() const
        {
            return *this;
        }

        /** \brief Auxiliary struct to convert this type to shared_allocator of other type. */
        template <class U>
        struct rebind
        {
            typedef shared_allocator<U> other;
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
        shared_allocator() throw()
        {
        }

        /** \brief Empty copy constructor.

        Does nothing. */
        template <class U>
        shared_allocator(const shared_allocator<U>&) throw()
        {
        }

        /** \brief Empty assignment operator.

        Does nothing and returns reference to this allocator. */
        template <class U>
        shared_allocator<T>& operator=(const shared_allocator<U>&) throw()
        {
            return *this;
        }

        /** \brief Returns number of elements in allocated memory.
        
        \param _memory Pointer to allocated memory.
        
        \warning Please, notice that the number of ELEMENTS will be returned (not number of BYTES). */
        size_type size(const_pointer _memory) const
        {
            return shared_size(_memory) / sizeof(T);
        }

        /** \brief Deallocate memory.

        \param _memory Pointer to allocated memory. */
        void deallocate(pointer _memory, size_type = 0) const
        {
            shared_deallocate(_memory);
        }

        /** \brief Allocate array of elements.

        \param _number Required number of elements in array. */
        pointer allocate(size_type _number) const
        {
            return static_cast<T*>(shared_allocate(_number * sizeof(T), nullptr));
        }

        /** \brief Allocate array of elements using hint.

        \param _number Required number of elements in array.
        \param _currentMemory Pointer to memory allocated earlier (it contains size which will be used as a hint). */
        pointer allocate(size_type _number, void* _currentMemory) const
        {
            return static_cast<T*>(shared_allocate(_number * sizeof(T), _currentMemory));
        }

        /** \brief Construct new object on preallocated memory using default constructor.

        \param _singleObject Pointer to preallocated memory. */
        void construct(T* _singleObject) const
        {
            shared_construct(_singleObject);
        }

        /** \brief Construct new object on preallocated memory using copy-constructor.

        \param _singleObject Pointer to preallocated memory.
        \param _value Const-reference to another object instance to be coped from.

        \note Declared as template function to make it possible to use this allocator with
        types without public copy-constructor. */
        template <class U>
        void construct(U* _singleObject, const U& _value) const
        {
            shared_construct(_singleObject, _value);
        }

        /** \brief Construct new object on preallocated memory using move-constructor.

        \param _singleObject Pointer to preallocated memory.
        \param _value Rvalue-reference to another object instance to be moved from.

        \note Declared as template function to make it possible to use this allocator with
        types without public move-constructor. */
        template <class U>
        void construct(U* _singleObject, U&& _value) const
        {
            shared_construct(_singleObject, ::std::forward<U&&>(_value));
        }

        /** \brief Construct new object on preallocated memory using arguments list.

        \param _singleObject Pointer to preallocated memory.
        \param _constructorArguments Variadic arguments list to be used by object constructor.

        \note Declared as template function to make it possible to use this allocator with
        types without specific constructor with arguments. */
        template <class U, class... TArgs>
        void construct(U* _singleObject, TArgs&&... _constructorArguments) const
        {
            ::new (static_cast<void*>(_singleObject)) U(::std::forward<TArgs>(_constructorArguments)...);
        }

        /** \brief Destroy pointed object.

        Invokes object's destructor.

        \param _singleObject Pointer to object.

        \note Declared as template function to make it possible to use this allocator with
        types without public destructor. */
        template <class U>
        void destroy(U* _singleObject) const
        {
            _singleObject->~U();
        }

        /** \brief Estimate maximum array size. */
        size_type max_size() const throw()
        {
            return (size_type)(-1) / sizeof(T);
        }

    }; // END struct shared_allocator<T>.

} // END namespace salloc.

/** Operator == for STL compatibility. */
template <class T, class U>
inline bool operator == (const ::salloc::shared_allocator<T>&, const ::salloc::shared_allocator<U>&) throw()
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SHARED___ALLOCATOR_____HPP___
