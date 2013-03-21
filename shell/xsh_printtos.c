#include <shell.h>
#include <stddef.h>
#include <thread.h>
#include <stdio.h>
#include <lab1.h>

thread myprogA(int, char**);

static int i = 0;

static void createChildWithStdout(void* func, char* name, int argc, char** argv)
{
	tid_typ child = create(func, 1024, 20, name, 2, argc, argv);
	if (child != SYSERR)
	{
		thrtab[child].fdesc[1] = stdout;
		ready(child, RESCHED_NO);
	} else {
		printf("Failed to spawn %s\n", name);
	}
}

shellcmd xsh_printtos(int argc, char** argv)
{
	createChildWithStdout(myprogA, "printtos", argc, argv);
	resched();
	return 0;
}

shellcmd xsh_printprocstks(int argc, char** argv)
{
	createChildWithStdout(myprogA, "procstks1", argc, argv);
	createChildWithStdout(myprogA, "procstks2", argc, argv);
	createChildWithStdout(myprogA, "procstks3", argc, argv);
	resched();
	return 0;
}
