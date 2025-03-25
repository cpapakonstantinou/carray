#include<cstdint>
#include<aligned_memory.h>

int main(void)
{

	size_t size = 8;

	unique_ptr_aligned<uint16_t[]> x(palign<uint16_t>(8, size), &pdelete);

	auto y = make_unique_aligned<uint8_t>(32, size);

	auto yy = make_unique_aarray<uint8_t>(32, size);	

	std::shared_ptr<uint8_t[]> z(palign<uint8_t>(32, size), &pdelete);

	auto a = make_shared_aarray<uint8_t>(64, size);

	printf("x (unique) has alignment 8: %p : %d\n", x.get(), ((uintptr_t)x.get() & (uintptr_t)(8 - 1)) == 0);
	printf("y (unique) has alignment 32: %p : %d\n", y.get(), ((uintptr_t)y.get() & (uintptr_t)(32 - 1)) == 0);
	printf("z (shared) has alignment 32: %p : %d\n", z.get(), ((uintptr_t)z.get() & (uintptr_t)(32 - 1)) == 0);
	printf("a (shared) has alignment 64: %p : %d\n", a.get(), ((uintptr_t)a.get() & (uintptr_t)(64 - 1)) == 0);

	printf("Freeing yy pointer by reset()\n");
	yy.reset();

	printf("%p\n",&(yy[0]));

	return 0;
}