#ifndef __ALIGNED_MEMORY_H__
#define __ALIGNED_MEMORY_H__

// Copyright (c) 2025  Constantine Papakonstantinou
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

/**
 * \file aligned_memory.h header only support for smart memory aligned pointers
 * \author cpapakonstantinou
 * \date 2021
 **/
#include <memory>
#include <stdlib.h>

/**
 * \brief   allocated a memory aligned array
 * \param   align   the byte alignment
 * \param   size    the memory size
 * \returns the memory aligned pointer
 */
template<class T>
static T* palign(size_t align, size_t size)
{

    T* ptr = static_cast<T*>(aligned_alloc(align, size*sizeof(T)));
    if(ptr==nullptr)
         throw std::bad_alloc();

	// T* ptr;
	// if(posix_memalign(reinterpret_cast<void**>(&ptr),align,size))
	// 	throw std::bad_alloc(); 

	return ptr;
}

/**
 * \brief   frees a memory aligned array allocated with palign
 * \param   ptr   the pointer to free
 */
static inline void pdelete(void * ptr) 
{ 
    free(ptr); 
}

/** 
 *  type alias unique_ptrs since the deleter is part of the type
 *  note for shared_ptrs the deleter is only part of the constructor, not the type 
 */
  
///alias for memory aligned unique pointer types
template<class T> using unique_ptr_aligned = std::unique_ptr<T, decltype(&pdelete)>;
///alias for memory aligned unique pointer array types
template<class T> using unique_ptr_aaray = unique_ptr_aligned<T[]>;

/**
 * \brief 	declares a memory aligned unique_ptr
 * \param 	align 	the byte alignment
 * \param 	size 	the memory size
 * \returns a unique_ptr pointing at a memory aligned address
 */
template<class T>
unique_ptr_aligned<T> make_unique_aligned(size_t align, size_t size)
{
	return unique_ptr_aligned<T>(palign<T>(align, size), &pdelete);
}

/**
 * \brief 	declares a memory aligned unique_ptr array
 * \param 	align 	the byte alignment
 * \param 	size 	the memory size
 * \returns a unique_ptr pointing at a memory aligned address
 */
template<class T>
unique_ptr_aaray<T> make_unique_aarray(size_t align, size_t size)
{
	return unique_ptr_aaray<T>(palign<T>(align, size), &pdelete);
}

/**
 * \brief 	declares a memory aligned shared_ptr
 * \param 	align 	the byte alignment
 * \param 	size 	the memory size
 * \returns a shared_ptr pointing at a memory aligned address
 */
template<class T>
std::shared_ptr<T> make_shared_aligned(size_t align, size_t size)
{
	return std::shared_ptr<T>(palign<T>(align, size), &pdelete);
}

/**
 * \brief 	declares a memory aligned shared_ptr array
 * \param 	align 	the byte alignment
 * \param 	size 	the memory size
 * \returns a shared_ptr array pointing at a memory aligned address
 */
template<class T>
std::shared_ptr<T[]> make_shared_aarray(size_t align, size_t size)
{
	return std::shared_ptr<T[]>(palign<T>(align, size), &pdelete);
}

#endif//__ALIGNED_MEMORY_H__