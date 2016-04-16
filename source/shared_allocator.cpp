/***************************************************************************************
* file        : shared_allocator.cpp
* data        : 2016/03/05
* author      : Victor Zarubkin
* contact     : v.s.zarubkin@gmail.com
* copyright   : Copyright (C) 2016 Victor Zarubkin
*             :
* description : This header contains implementation of shared_allocate,
*             : shared_deallocate and shared_size functions.
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

#include <xstddef>
#include <type_traits>
#include <malloc.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)

# define SHARED_ALLOCATOR_DLLEXPORT __declspec(dllexport)

#else

# define SHARED_ALLOCATOR_DLLEXPORT

#endif // defined(WIN32) || defined(_WIN32) || defined(_WIN64)

extern "C"
{
    /** \brief Allocate new memory.
    
    \param _bytesNumber Required bytes number including already preallocated size of _currentMemory (if exist).
    \param _currentMemory Pointer to existing memory buffer (if exist). If it's size is greater than _bytesNumber then no new memory would be allocated. */
    SHARED_ALLOCATOR_DLLEXPORT void* shared_allocate(size_t _bytesNumber, void* _currentMemory = nullptr)
    {
        if (_currentMemory == nullptr)
        {
            size_t* newMemory = reinterpret_cast<size_t*>(malloc(_bytesNumber + sizeof(size_t)));
            *newMemory = _bytesNumber;
            return static_cast<void*>(newMemory + 1);
        }

        size_t* pMemory = reinterpret_cast<size_t*>(_currentMemory) - 1;
        if (*pMemory >= _bytesNumber)
        {
            return _currentMemory;
        }

        size_t* newMemory = reinterpret_cast<size_t*>(realloc(pMemory, _bytesNumber + sizeof(size_t)));
        *newMemory = _bytesNumber;
        return static_cast<void*>(newMemory + 1);
    }

    /** \brief Deallocate memory.
    
    \param _currentMemory Pointer to existing memory buffer to be deallocated.
    
    \warning Pointer to _currentMemory after this operation will be invalid. */
    SHARED_ALLOCATOR_DLLEXPORT void shared_deallocate(void* _currentMemory)
    {
        if (_currentMemory != nullptr)
        {
            size_t* pMemory = reinterpret_cast<size_t*>(_currentMemory) - 1;
            free(pMemory);
        }
    }

    /** \brief Returns memory size in bytes.

    \param _currentMemory Pointer to existing memory buffer. */
    SHARED_ALLOCATOR_DLLEXPORT size_t shared_size(const void* _currentMemory)
    {
        if (_currentMemory != nullptr)
        {
            return *(reinterpret_cast<const size_t*>(_currentMemory) - 1);
        }

        return 0;
    }

}

#undef SHARED_ALLOCATOR_DLLEXPORT

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
