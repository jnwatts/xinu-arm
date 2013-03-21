#include <stdio.h>
#include <stddef.h>
#include <thread.h>
#include <lab1.h>

static int* getStackPointer(void);

void printprocstks(void)
{
	struct thrent *entry = &thrtab[thrcurrent];
	printf("Proc[%s].Pid    = %d\r\n"
	       "\tStack: Base   = 0x%X\r\n"
	       "\t       Len    = 0x%X\r\n"
	       "\t       Limit  = 0x%X\r\n"
	       "\t       StkPtr = 0x%X\r\n",
	       entry->name, thrcurrent, entry->stkbase,
	       entry->stklen, entry->stkbase - entry->stklen,
	       getStackPointer());
}

static int* getStackPointer(void)
{
	register int sp asm("sp");
	return (int*)sp;
}
