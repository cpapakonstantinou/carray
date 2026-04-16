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
	static_assert(N >= 1 && N <= 4, "carray supports rank <= 4");
	
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
		_shape(new size_t[N]{ static_cast<size_t>(ijk)... })
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
		_shape(a._shape), 
		_buffer(a._buffer), 
		_ptr(a._ptr)
		{}

		/**	
		*	\brief	move constructor.
		*	Internally carray uses a shared_ptr to manage the buffer.
		*	Move construction is safely supported.
		*	\param 	i	number of rows
		*/
		carray(carray<T, N, A>&& a) noexcept:
		_shape(std::move(a._shape)),
		_buffer(std::move(a._buffer)),
		_ptr(std::move(a._ptr))
		{}

		
		/**	\brief	Copy assignment operator. */
		carray<T, N, A>& 
		operator=(const carray<T, N, A>& a)
		{
			if (&a != this) 
			{
				_shape=a._shape; 
				_buffer=a._buffer; 
				_ptr = a._ptr;
			}
			return *this;
		}

		/**	\brief Move assignment operator. */
		carray<T, N, A>& 
		operator=(carray<T, N, A>&& a) noexcept
		{
			if (&a != this)
			{
				_shape = std::move(a._shape); 
				_buffer = std::move(a._buffer); 
				_ptr = std::move(a._ptr);
			}
			return *this;
		}

	private:				

		using vector_t = T*; ///< vector datatype
		using matrix_t = T**; ///< matrix datatype
		using tensor_t = T***; ///< tensor datatype
		using tetrad_t = T****; ///< tetrad datatype
		using shape_t = std::shared_ptr<size_t[]>;///< shared array shape datatype		
		using buffer_t = std::shared_ptr<T[]>; ///< shared contiguous array datatype underlying the memory of the carray
		using ptr_t = std::shared_ptr<void>; ///< Hierarchical pointer datatype
		
		shape_t _shape; ///< shape of the carray
		buffer_t _buffer; ///< contiguous memory of the carray
		ptr_t _ptr; ///< Hierarchical pointer for structured memory access to buffer

		inline void
		allocate_memory()
		{
			size_t n = std::accumulate(_shape.get(), _shape.get() + N, 1, std::multiplies<size_t>());
			_buffer = make_shared_aarray<T>(A, n); 

			if constexpr (N == 1)
			{
				 _ptr = std::shared_ptr<void>(_buffer.get(), [](void*){});
			}
			else if constexpr (N == 2)
			{
				matrix_t matrix = static_cast<matrix_t>(palign<vector_t>(A, _shape[0]));
				for (size_t i = 0; i < _shape[0]; ++i)
					matrix[i] = &_buffer[i * _shape[1]];
				_ptr = std::shared_ptr<void>(matrix, &pdelete);
			}
			else if constexpr (N == 3)
			{
				tensor_t tensor = static_cast<tensor_t>(palign<matrix_t>(A, _shape[0]));
				matrix_t matrix = static_cast<matrix_t>(palign<vector_t>(A, _shape[0] * _shape[1]));

				for (size_t i = 0; i < _shape[0]; ++i)
				{
					tensor[i] = &matrix[i * _shape[1]];
					for (size_t j = 0; j < _shape[1]; ++j)
						tensor[i][j] = &_buffer[(i * _shape[1] + j) * _shape[2]];
				}
				_ptr = std::shared_ptr<void>(tensor, [](void* ptr){
					pdelete(static_cast<tensor_t>(ptr)[0]); pdelete(ptr);});
			}
			else if constexpr (N == 4)
			{
				tetrad_t tetrad = static_cast<tetrad_t>(palign<tensor_t>(A, _shape[0]));
				tensor_t tensor = static_cast<tensor_t>(palign<matrix_t>(A, _shape[0] * _shape[1]));
				matrix_t matrix = static_cast<matrix_t>(palign<vector_t>(A, _shape[0] * _shape[1] * _shape[2]));

				for (size_t t = 0; t < _shape[0]; ++t)
				{
					tetrad[t] = &tensor[t * _shape[1]];
					for (size_t i = 0; i < _shape[1]; ++i)
					{
						tetrad[t][i] = &matrix[(t * _shape[1] + i) * _shape[2]];
						for (size_t j = 0; j < _shape[2]; ++j)
							tetrad[t][i][j] = &_buffer[((t * _shape[1] + i) * _shape[2] + j) * _shape[3]];
					}
				}

				_ptr = std::shared_ptr<void>(tetrad, [](void* ptr){
					auto p = static_cast<tetrad_t>(ptr);
					pdelete(p[0][0]);
					pdelete(p[0]);
					pdelete(ptr);
				});
			}
		}

	public:
	/**	\brief Buffer RO operator. Used for read-only access to memory. */
	template<typename... IJK>
	const T& operator()(IJK... ijk) const 
	{
		static_assert(sizeof...(IJK) == N, "number of indices must match rank of array");

		size_t idx[] = { static_cast<size_t>(ijk)... };

		if constexpr (N == 1) return static_cast<vector_t>(_ptr.get())[idx[0]];
		else if constexpr (N == 2) return static_cast<matrix_t>(_ptr.get())[idx[0]][idx[1]];
		else if constexpr (N == 3) return static_cast<tensor_t>(_ptr.get())[idx[0]][idx[1]][idx[2]];
		else if constexpr (N == 4) return static_cast<tetrad_t>(_ptr.get())[idx[0]][idx[1]][idx[2]][idx[3]];
		else static_assert(N <= 4, "rank of array not supported, for rank > 4");
	}

	/**	\brief Buffer RW operator. Used for read-write access to memory. */
	template<typename... IJK>
	decltype(auto) operator[](IJK&&... ijk)
	{
		size_t idx[] = { static_cast<size_t>(ijk)... };  

		if constexpr (N == 1) return static_cast<vector_t>(_ptr.get())[idx[0]];  
		else if constexpr (N == 2) return static_cast<matrix_t>(_ptr.get())[idx[0]];
		else if constexpr (N == 3) return static_cast<tensor_t>(_ptr.get())[idx[0]];
		else if constexpr (N == 4) return static_cast<tetrad_t>(_ptr.get())[idx[0]];
	}

	/**	\brief Range begin operator. Beginning of contiguous memory block. */
	T* 
	begin()
	{
		return _buffer.get();
	}

	/**	\brief Range end operator. End of contiguous memory block. */
	T* 
	end()
	{
		return _buffer.get() + std::accumulate(_shape.get(), _shape.get() + N, 1, std::multiplies<size_t>());
	}
	
	/** \brief View acquisition. Acquire the hierarchical view (_ptr) for the buffer*/
	decltype(auto)
	get()
	{
		if constexpr (N == 1) return static_cast<vector_t>(_ptr.get());  
		else if constexpr (N == 2) return static_cast<matrix_t>(_ptr.get());
		else if constexpr (N == 3) return static_cast<tensor_t>(_ptr.get());
		else if constexpr (N == 4) return static_cast<tetrad_t>(_ptr.get());
	} 
};

//Convenience alias for construction. 
//Default alignment is suitable for cache efficiency of most architectures
//Specification of alignment per application is recommended. 
template<typename T> using cvector = carray<T, 1, 64>;///< shortcut for array of Type T and Rank 1.
template<typename T> using cmatrix = carray<T, 2, 64>;///< shortcut for array of Type T and Rank 2.
template<typename T> using ctensor = carray<T, 3, 64>;///< shortcut for array of Type T and Rank 3.
template<typename T> using ctetrad = carray<T, 4, 64>;///< shortcut for array of Type T and Rank 3.

#endif //__C_ARRAY_H__