#include <memory>
#include <iostream>
#include <cstdint>
#include <exception>
#include <assert.h>
#include <carray.h>

#define VERBOSE 0 

std::tuple<bool, bool, bool>
contiguous_memory_test()
{
	bool v=true,m=true,t=true;

	using test_t = uint8_t;
	
	int rows=4,cols=3,depth=2;

	cvector<test_t> vec(rows);

	cmatrix<test_t> mat(rows,cols);

	ctensor<test_t> tensor(rows,cols,depth);

	for(int i =0; i<rows; i++)
	{
		#if VERBOSE
			printf("&A(i=%d) = %p\n",i, &(vec(i)));
		#endif
		v &= (&(vec(i)) == &(vec(0)) + i);
	}

	for(int i =0; i<rows; i++)
		for(int j=0; j<cols; j++)
		{
			#if VERBOSE
				printf("&A(i=%d,j=%d) = %p\n",i,j, &(mat(i, j)));
			#endif
			m &= (&(mat(i,j)) == &(mat(0,0)) + i * cols + j);
		}
	
	for(int i =0; i<rows; i++)
		for(int j=0; j<cols; j++)
			for(int k =0; k<depth; k++)
			{
				#if VERBOSE
					printf("&A(i=%d,j=%d,k=%d) = %p\n",i,j,k, &(tensor(i, j, k)));
				#endif
				t &= (&(tensor(i, j, k)) == &(tensor(0, 0, 0)) + i*cols*depth + j*depth + k);
			}

	return std::make_tuple(v, m, t);
}

std::tuple<bool, bool, bool>
alignment_test()
{
	size_t rows = 64;
	bool b8, b32, b64;

	carray<uint8_t, 1,  8> valign8b(rows);
	b8 = ((uintptr_t)(&(valign8b(0))) % 8 == 0);

	carray<uint16_t, 2, 32> valign32b(rows, 1);
	b32 = ((uintptr_t)(&(valign32b(0,0))) % 32 == 0);

	carray<uint16_t, 3, 64> valign64b(rows, 1, 1);
	b64 = ((uintptr_t)(&(valign64b(0,0,0))) % 64 == 0);

	#if VERBOSE
		printf("&b8 = %p\n",&(valign8b(0)));
		printf("&b32 = %p\n",&(valign32b(0, 0)));
		printf("&b64 = %p\n",&(valign64b(0, 0, 0)));
	#endif

	return std::make_tuple(b8, b32, b64);
}

std::tuple<bool,bool>
constructor_test()
{
	bool move, copy;

	cmatrix<uint8_t> mv{ cmatrix<uint8_t>{2,2} };  // Move construct from temporary
	mv[0][0]=1;
	move = mv[0][0];
 
	cmatrix<uint8_t> cv = mv;

	copy = (cv[0][0] == mv[0][0]) & ( (&(cv[0][0])) == (&(mv[0][0])) );
	
	#if VERBOSE
		printf("Move construction test: %s = %u\n", "mv[0][0]", mv[0][0]);
		printf("Copy construction test: %s = %u\n", "cv[0][0]", cv[0][0]);

	#endif
	return std::make_tuple(move, copy);
}

// std::tuple<bool, bool>
// major_order_test()
// {
// 	bool r = true, c = true;

// 	int rows = 3, cols = 3;

// 	carray<uint8_t, 2, 64, ROW_MAJOR> rm{rows, cols};
// 	carray<uint8_t, 2, 64, COL_MAJOR> cm{rows, cols};

// 	// Test for row-major ordering
// 	for (int i = 0; i < rows; i++) 
// 		for (int j = 0; j < cols; j++) 
// 		{
// 			#if VERBOSE
// 				printf("ROW MAJOR &A(i=%d,j=%d) = %p\n",i,j, &(rm(i, j)));
// 			#endif
// 			r &= (&(rm(i, j)) == &(rm(0, 0)) + i * cols + j);
// 		}

// 	// Test for column-major ordering
// 	for (int i = 0; i < rows; i++) 
// 		for (int j = 0; j < cols; j++) 
// 		{
// 			#if VERBOSE
// 				printf("COL MAJOR &A(i=%d,j=%d) = %p\n",i,j, &(cm(i, j)));
// 			#endif
// 			c &= (&(cm(i, j)) == &(cm(0, 0)) + j * rows + i);
// 		}

// 	return std::make_tuple(r, c);
// }


int main(int argc, char* argv[])
{
	try
	{
		auto status = [](bool x){return x ? "OK" : "FAIL";};
		
		auto [v,m,t] = contiguous_memory_test();

		printf("[%s] Contiguous vector\n[%s] Contiguous matrix\n[%s] Contiguous tensor\n", status(v), status(m), status(t) );

		auto [b8, b32, b64] = alignment_test();

		printf("[%s] 8 byte aligned \n[%s] 32 byte aligned \n[%s] 64 byte aligned\n", status(b8), status(b32), status(b64) );

		auto [mc, cc] = constructor_test();

		printf("[%s] move construction \n[%s] copy construction\n", status(mc), status(cc) );

		// auto [r, c] = major_order_test();

		// printf("[%s] row major order \n[%s] column major order\n", status(r), status(c) );

	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}