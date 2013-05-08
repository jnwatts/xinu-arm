#include <stdio.h>
#include <stddef.h>
#include <thread.h>
#include <string.h>
#include <lab1.h>

void myfuncA(void);
const char PRINTTOS_NAME[] = "printtos";

int myprogA(int argc, char** argv)
{
	int part = 2;
	if (strncmp(argv[0], PRINTTOS_NAME, sizeof(PRINTTOS_NAME)) == 0) {
		part = 2;
	} else { 
		part = 3;
	}
	if (part == 2) {
		printf("Before calling myfuncA()\r\n");
		printtos();
		myfuncA();
		printf("After calling myfuncA()\r\n");
		printtos();
	} else if (part == 3) {
		printprocstks();
	}
	return 0;
}

void myfuncA(void)
{
	printf("Inside myfuncA()\r\n");
	printtos();
}

