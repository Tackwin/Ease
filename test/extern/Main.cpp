#include <assert.h>
#include <stdio.h>

extern int f() noexcept;

int main() {
	assert(f() == 42);
	printf("Extern: OK.\n");
	return 0;
}