#include <stdio.h>
#include <stddef.h>
#include <thread.h>
#include <lab1.h>

void printtos(void)
{
	register int reg_sp asm("sp");
	unsigned int *sp = reg_sp;
	int i = 0;
	printf("Stack:\n");
	for (i = 0; i < 5; ++i) {
		printf(" %x = %x\n", sp, *sp);
		sp++;
	}
	printf("\n");

}
