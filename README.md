# carray
carray is a class that encapsulates the construction of dynamically allocated & memory aligned contiguous arrays. Such arrays may find application in scientific and high performance computing. 

- Libraries such as BLAS, FFTW, and others require contiguous arrays.
- SIMD (Single Instruction, Multiple Data) use memory alignment for efficient memory access patterns.

carray is a lightweight header only library that takes advantage of modern c++ semantics, and stdlib.

A carray's type, rank, and alignment can be specified by template insantiation. The size of the array is passed to the constructor. 
The cmatrix alias is defaulted to align at a 64-byte address. Most CPUs have a 64 byte cache line size.

- type alias is also defined for ctensor.
- carray currently supports up to rank 3 arrays

``` c++
//<TYPE, RANK, ALIGN>
carray<uint8_t, 2, 64>
cmatrix<uint8_t> mat(rows, cols);

```

## Memory Layout and Alignment 

One points at sequential addressess linearly in contiguous memory. Traversal of this kind of memory is what CPU subsystems are best at, and what optimizes cache performance.
Internally carray has a contiguous memory buffer and a pointer to the bufffer that arranges access to the buffer in row-major format.

``` C++
for(int i =0; i<rows; i++)
{
  for(int j=0; j<cols; j++)
  {
    printf("&A(i=%d,j=%d) = %p\n",i,j, &(mat(i, j)));
  }
}


&A(i=0,j=0) = 0x636f8d627040
&A(i=0,j=1) = 0x636f8d627041
&A(i=0,j=2) = 0x636f8d627042
&A(i=1,j=0) = 0x636f8d627043
&A(i=1,j=1) = 0x636f8d627044
&A(i=1,j=2) = 0x636f8d627045
&A(i=2,j=0) = 0x636f8d627046
&A(i=2,j=1) = 0x636f8d627047
&A(i=2,j=2) = 0x636f8d627048
&A(i=3,j=0) = 0x636f8d627049
&A(i=3,j=1) = 0x636f8d62704a
&A(i=3,j=2) = 0x636f8d62704b
```
### Smart pointer buffer
Internally carray uses a shared smart pointer for the buffer. So one gets the same access syntax as for C language arrays. 

Copy construction is supported so if you copy assign a carray, both instances share the same memory buffer and the lifetime of the buffer is managed by the lifetime of the last class instance. 

```C++
cmatrix<uint8_t> mv{ cmatrix<uint8_t>{2,2} };  // Move construct from temporary	
cmatrix<uint8_t> cv = mv; // Copy construct 

// Memory addresses of the copy constructed array will be shared 
copy = (cv[0][0] == mv[0][0]) & ( (&(cv[0][0])) == (&(mv[0][0])) );
```

