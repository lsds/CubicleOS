#define _GNU_SOURCE 
#include <sys/mman.h>
#include <stdio.h>

void main() {
	printf("pkey alloc = %d\n", pkey_alloc(0,0));
	perror("pkey");
}
