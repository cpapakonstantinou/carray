#ifndef __C_ARRAY_H__
#define __C_ARRAY_H__
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
 * \file carray.h header only support for dynamically allocated memory aligned arrays
 * \author cpapakonstantinou
 * \date 2025
 **/
#include <cstdint>
#include <numeric>
#include <memory>
#include <assert.h>
#include <aligned_memory.h>

/**
* 	\brief	dynamically allocated and aligned contiguous memory arrays.
*	Row major contiguous memory allocation.
*	Memory alignment can be specified by template instantiation.
*	T - Type
*	N - Rank
*	A - Alignment
*/
template<class T, size_t N, size_t A>
class carray
{
	static_assert(N >= 1 && N <= 3, "carray supports rank <= 3");
	
	public:
		/**
		 *	\brief constructor.
		 *	Variadic constructor which accepts a parameter pack.
		 *	Parameter pack must expand to N elements.
		 * 
		 *	\param 	ijk	parameter pack
		 */
		template<class... IJK>
		explicit
		carray(IJK&&... ijk):
		shape_(new size_t[N]{ static_cast<size_t>(ijk)... })
		{
			static_assert(sizeof...(IJK) == N, "number of indices must match rank of array");
			allocate_memory();
		}

		/**	
		*	\brief copy constructor.
		*	Internally carray uses a shared_ptr to manage the buffer.
		*	Copy construction is safely supported
		*	\param 	an initialized carray class
		*/
		carray(carray<T, N, A>& a):
		shape_(a.shape_), 
		buffer_(a.buffer_), 
		ptr_(a.ptr_)
		{}

		/**	
		*	\brief	move constructor.
		*	Internally carray uses a shared_ptr to manage the buffer.
		*	Move construction is safely supported.
		*	\param 	i	number of rows
		*/
		carray(carray<T, N, A>&& a) noexcept:
		shape_(std::move(a.shape_)),
		buffer_(std::move(a.buffer_)),
		ptr_(std::move(a.ptr_))
		{}

		
		/**	\brief	Copy assignment operator. */
		carray<T, N, A>& 
		operator=(const carray<T, N, A>& a)
		{
			if (&a != this) 
			{
				shape_=a.shape_; 
				buffer_=a.buffer_; 
				ptr_ = a.ptr_;
			}
			return *this;
		}

		/**	\brief Move assignment operator. */
		carray<T, N, A>& 
		operator=(carray<T, N, A>&& a) noexcept
		{
			if (&a != this)
			{
				shape_ = std::move(a.shape_); 
				buffer_ = std::move(a.buffer_); 
				ptr_ = std::move(a.ptr_);
			}
			return *this;
		}

	private:				

		using vector_t = T*; ///< vector datatype
		using matrix_t = T**; ///< matrix datatype
		using tensor_t = T***; ///< tensor datatype
		using shape_t = std::shared_ptr<size_t[]>;///< shared array shape datatype		
		using buffer_t = std::shared_ptr<T[]>; ///< shared contiguous array datatype underlying the memory of the carray
		using ptr_t = std::shared_ptr<void>; ///< Hierarchical pointer datatype
		
		shape_t shape_; ///< shape of the carray
		buffer_t buffer_; ///< contiguous memory of the carray
		ptr_t ptr_; ///< Hierarchical pointer for structured memory access to buffer

		inline void
		allocate_memory()
		{
			size_t n = std::accumulate(shape_.get(), shape_.get() + N, 1, std::multiplies<size_t>());
			buffer_ = make_shared_aarray<T>(A, n); 

			if constexpr (N == 1)
			{
				 ptr_ = std::shared_ptr<void>(buffer_.get(), [](void*){});
			}
			else if constexpr (N == 2)
			{
				matrix_t matrix = static_cast<matrix_t>(palign<vector_t>(A, shape_[0]));
				for (size_t i = 0; i < shape_[0]; ++i)
					matrix[i] = &buffer_[i * shape_[1]];
				ptr_ = std::shared_ptr<void>(matrix, &pdelete);
			}
			else if constexpr (N == 3)
			{
				tensor_t tensor = static_cast<tensor_t>(palign<matrix_t>(A, shape_[0]));
				matrix_t matrix = static_cast<matrix_t>(palign<vector_t>(A, shape_[0] * shape_[1]));

				for (size_t i = 0; i < shape_[0]; ++i)
				{
					tensor[i] = &matrix[i * shape_[1]];
					for (size_t j = 0; j < shape_[1]; ++j)
						tensor[i][j] = &buffer_[(i * shape_[1] + j) * shape_[2]];
				}
				ptr_ = std::shared_ptr<void>(tensor, [](void* ptr){
					pdelete(static_cast<tensor_t>(ptr)[0]); pdelete(ptr);});
			}
		}


	public:
	/**	\brief Buffer RO operator. Used for read-only access to memory. */
	template<typename... IJK>
	const T& operator()(IJK... ijk) const 
	{
		static_assert(sizeof...(IJK) == N, "number of indices must match rank of array");

		size_t idx[] = { static_cast<size_t>(ijk)... };

		if constexpr (N == 1) return static_cast<vector_t>(ptr_.get())[idx[0]];
		else if constexpr (N == 2) return static_cast<matrix_t>(ptr_.get())[idx[0]][idx[1]];
		else if constexpr (N == 3) return static_cast<tensor_t>(ptr_.get())[idx[0]][idx[1]][idx[2]];
		else static_assert(N <= 3, "rank of array not supported, for rank > 3");
	}

	/**	\brief Buffer RW operator. Used for read-write access to memory. */
	template<typename... IJK>
	decltype(auto) operator[](IJK&&... ijk)
	{
		size_t idx[] = { static_cast<size_t>(ijk)... };  

		if constexpr (N == 1) return static_cast<vector_t>(ptr_.get())[idx[0]];  
		else if constexpr (N == 2) return static_cast<matrix_t>(ptr_.get())[idx[0]];
		else if constexpr (N == 3) return static_cast<tensor_t>(ptr_.get())[idx[0]];
	}

	/**	\brief Range begin operator. Beginning of contiguous memory block. */
	T* 
	begin()
	{
		return buffer_.get();
	}

	/**	\brief Range end operator. End of contiguous memory block. */
	T* 
	end()
	{
		return buffer_.get() + std::accumulate(shape_.get(), shape_.get() + N, 1, std::multiplies<size_t>());
	}
};

//Convenience alias for construction. 
//Default alignment is suitable for cache efficiency of most architectures
//Specification of alignment per application is recommended. 
template<typename T> using cvector = carray<T, 1, 64>;///< shortcut for array of Type T and Rank 1.
template<typename T> using cmatrix = carray<T, 2, 64>;///< shortcut for array of Type T and Rank 2.
template<typename T> using ctensor = carray<T, 3, 64>;///< shortcut for array of Type T and Rank 3.

#endif //__C_ARRAY_H__