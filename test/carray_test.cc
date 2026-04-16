#include <memory>
#include <iostream>
#include <cstdint>
#include <exception>
#include <assert.h>
#include <carray.h>

#define VERBOSE 0 

std::tuple<bool, bool, bool, bool>
contiguous_memory_test()
{
	bool v=true,m=true,t=true,t4=true;

	using test_t = uint8_t;
	
	int rows=4,cols=3,depth=2,field=2;

	cvector<test_t> vec(rows);

	cmatrix<test_t> mat(rows,cols);

	ctensor<test_t> tensor(rows,cols,depth);

	ctetrad<test_t> tetrad(rows,cols,depth,field);

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

	for(int i =0; i<rows; i++)
		for(int j=0; j<cols; j++)
			for(int k =0; k<depth; k++)
				for(int f=0; f<field; f++)
			{
				#if VERBOSE
					printf("&A(i=%d,j=%d,k=%d,f=%d) = %p\n",i,j,k,f, &(tetrad(i, j, k, f)));
				#endif
				t4 &= (&(tetrad(i, j, k, f)) == &(tetrad(0, 0, 0, 0)) + i*cols*depth*field + j*depth*field + k*field + f);
			}

	return std::make_tuple(v, m, t, t4);
}

std::tuple<bool, bool, bool, bool>
alignment_test()
{
	size_t rows = 256;
	bool b8, b32, b64, b128;

	carray<uint8_t, 1,  8> valign8b(rows);
	b8 = ((uintptr_t)(&(valign8b(0))) % 8 == 0);

	carray<uint16_t, 2, 32> valign32b(rows, 1);
	b32 = ((uintptr_t)(&(valign32b(0,0))) % 32 == 0);

	carray<uint16_t, 3, 64> valign64b(rows, 1, 1);
	b64 = ((uintptr_t)(&(valign64b(0,0,0))) % 64 == 0);

	carray<uint16_t, 4, 128> valign128b(rows, 1, 1, 1);
	b128 = ((uintptr_t)(&(valign128b(0,0,0,0))) % 128 == 0);

	#if VERBOSE
		printf("&b8 = %p\n",&(valign8b(0)));
		printf("&b32 = %p\n",&(valign32b(0, 0)));
		printf("&b64 = %p\n",&(valign64b(0, 0, 0)));
		printf("&b128 = %p\n",&(valign128b(0, 0, 0)));
	#endif

	return std::make_tuple(b8, b32, b64, b128);
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


std::tuple<bool, bool, bool, bool> 
view_test()
{
	bool v = true, m = true, t = true, t4 = true;

	using test_t = uint8_t;
	
	int rows=4,cols=3,depth=2,field=2;

	cvector<test_t> vec(rows);

	cmatrix<test_t> mat(rows, cols);

	ctensor<test_t> tensor(rows, cols, depth);

	ctetrad<test_t> tetrad(rows, cols, depth, field);


	vec[rows-1] = rows;
	mat[rows-1][cols-1] = rows*cols;
	tensor[rows-1][cols-1][depth-1] = rows*cols*depth;
	tetrad[rows-1][cols-1][depth-1][field-1] = rows*cols*depth*field;

	auto r1 = [&rows](test_t* _ )->bool { return _[rows-1] == rows; };
	auto r2 = [&rows, &cols](test_t** _ )->bool { return _[rows-1][cols-1] == rows*cols; };
	auto r3 = [&rows, &cols, &depth](test_t*** _ )->bool { return _[rows-1][cols-1][depth-1] == rows*cols*depth; };
	auto r4 = [&rows, &cols, &depth, &field](test_t**** _ )->bool { return _[rows-1][cols-1][depth-1][field-1] == rows*cols*depth*field; };


	v = r1( vec.get() );
	m = r2( mat.get() );
	t = r3( tensor.get() );
	t4 = r4( tetrad.get() );

	return std::make_tuple(v, m, t, t4);


}

int main(int argc, char* argv[])
{
	try
	{
		auto status = [](bool x){return x ? "OK" : "FAIL";};
		
		auto [v, m, t, t4] = contiguous_memory_test();

		printf("[%s] Contiguous vector\n[%s] Contiguous matrix\n[%s] Contiguous tensor\n[%s] Contiguous tetrad\n", status(v), status(m), status(t), status(t4) );

		auto [b8, b32, b64, b128] = alignment_test();

		printf("[%s] 8 byte aligned \n[%s] 32 byte aligned \n[%s] 64 byte aligned\n[%s] 128 byte aligned\n", status(b8), status(b32), status(b64), status(b128) );

		auto [mc, cc] = constructor_test();

		printf("[%s] move construction \n[%s] copy construction\n", status(mc), status(cc) );

		// auto [r, c] = major_order_test();

		// printf("[%s] row major order \n[%s] column major order\n", status(r), status(c) );

		auto [r1, r2, r3, r4] = view_test();
		printf("[%s] rank1 view\n[%s] rank2 view\n[%s] rank3 view\n[%s] rank4 view\n", status(r1), status(r2), status(r3), status(r4));

	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}