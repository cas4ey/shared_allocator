/***************************************************************************************
* file        : shared_allocator.cpp
* data        : 2016/03/05
* author      : Victor Zarubkin
* contact     : v.s.zarubkin@gmail.com
* copyright   : Copyright (C) 2016  Victor Zarubkin
*             :
* description : This header contains implementation of shared_allocate, shared_deallocate and shared_size functions.
*             :
* references  : Original (and actual) version of source code can be found here <http://www.github.com/cas4ey/shared_allocator>.
*             :
* license     : This file is part of SharedAllocator.
*             :
*             : SharedAllocator is free software: you can redistribute it and/or modify
*             : it under the terms of the GNU General Public License as published by
*             : the Free Software Foundation, either version 3 of the License, or
*             : (at your option) any later version.
*             :
*             : SharedAllocator is distributed in the hope that it will be useful,
*             : but WITHOUT ANY WARRANTY; without even the implied warranty of
*             : MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*             : GNU General Public License for more details.
*             :
*             : You should have received a copy of the GNU General Public License
*             : along with SharedAllocator. If not, see <http://www.gnu.org/licenses/>.
*             :
*             : A copy of the GNU General Public License can be found in file LICENSE.
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
