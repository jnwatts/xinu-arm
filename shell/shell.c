/**
 * @file     shell.c
 * @provides shell.
 *
 * $Id: shell.c 2157 2010-01-19 00:40:07Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <ctype.h>
#include <interrupt.h>
#include <shell.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tty.h>
#include <thread.h>
#include <nvram.h>
#include <conf.h>

const struct centry commandtab[] = {
    {"printtos", FALSE, xsh_printtos},
    {"printprocstks", FALSE, xsh_printprocstks},
    {"ledon", FALSE, xsh_ledon},
    {"ledoff", FALSE, xsh_ledoff},
	{"usbStart", FALSE, xsh_usbStart},
	{"kbdStart", FALSE, xsh_kbdStart},

#if NETHER
    {"arp", FALSE, xsh_arp},
#endif
    {"clear", TRUE, xsh_clear},
    {"date", FALSE, xsh_date},
#if USE_TLB
    {"dumptlb", FALSE, xsh_dumptlb},
#endif
#if NETHER
    {"ethstat", FALSE, xsh_ethstat},
#endif
    {"exit", TRUE, xsh_exit},
#if NFLASH
    {"flashstat", FALSE, xsh_flashstat},
#endif
#ifdef GPIO_BASE
    {"gpiostat", FALSE, xsh_gpiostat},
#endif
    {"help", FALSE, xsh_help},
    {"kill", TRUE, xsh_kill},
#ifdef GPIO_BASE
    {"led", FALSE, xsh_led},
#endif
    {"memstat", FALSE, xsh_memstat},
    {"memdump", FALSE, xsh_memdump},
#if NETHER
    {"nc", FALSE, xsh_nc},
    {"netdown", FALSE, xsh_netdown},
#if NETEMU
    {"netemu", FALSE, xsh_netemu},
#endif
    {"netstat", FALSE, xsh_netstat},
    {"netup", FALSE, xsh_netup},
#endif
#if NVRAM
    {"nvram", FALSE, xsh_nvram},
#endif
    {"ps", FALSE, xsh_ps},
#if NETHER
    {"ping", FALSE, xsh_ping},
    {"rdate", FALSE, xsh_rdate},
#endif
    {"reset", FALSE, xsh_reset},
#if NETHER
    {"route", FALSE, xsh_route},
#endif
    {"sleep", TRUE, xsh_sleep},
#if NETHER
    {"snoop", FALSE, xsh_snoop},
#endif
#if USE_TAR
    {"tar", FALSE, xsh_tar},
#endif
#if NETHER
    {"tcpstat", FALSE, xsh_tcpstat},
    {"telnet", FALSE, xsh_telnet},
    {"telnetserver", FALSE, xsh_telnetserver},
#endif
    {"test", FALSE, xsh_test},
    {"testsuite", TRUE, xsh_testsuite},
    {"uartstat", FALSE, xsh_uartstat},
#if USE_TLB
    {"user", FALSE, xsh_user},
#endif
#if NETHER
    {"udpstat", FALSE, xsh_udpstat},
    {"vlanstat", FALSE, xsh_vlanstat},
    {"voip", FALSE, xsh_voip},
    {"xweb", FALSE, xsh_xweb},
#endif
    {"?", FALSE, xsh_help}
};

typedef struct {
	int size;
	char* com[SHELL_BUFLEN];
}historyTab;


const ulong ncommand = sizeof(commandtab) / sizeof(struct centry);

// History Stuff
int histIndex = 0;	// index in read
int curHistIndex = 0;	// index to overwrite for next command
int curHistElements = 1;// number of elements in history array
#define MAX_HISTORY 5
historyTab history[MAX_HISTORY];


/**
 * The Xinu shell.  Provides an interface to execute commands.
 * @param descrp descriptor of device on which the shell is open
 * @return OK for successful exit, SYSERR for unrecoverable error
 */
thread shell(int indescrp, int outdescrp, int errdescrp)
{
    char buf[SHELL_BUFLEN];     /* line input buffer        */
    short buflen;               /* length of line input     */
    char tokbuf[SHELL_BUFLEN + SHELL_MAXTOK];   /* token value buffer       */
    short ntok;                 /* number of tokens         */
    char *tok[SHELL_MAXTOK];    /* pointers to token values */
    char *outname;              /* name of output file      */
    char *inname;               /* name of input file       */
    bool background;            /* is background proccess?  */
    syscall child;              /* pid of child thread      */
    ushort i, j;                /* temp variables           */
    irqmask im;                 /* interrupt mask state     */

    //historyTab hist[MAX_HISTORY];

    /* hostname variables */
    char hostnm[NET_HOSTNM_MAXLEN + 1]; /* hostname of backend      */
    char *hostptr;              /* pointer to hostname      */
    int hostname_strsz;         /* nvram hostname name size */
    device *devptr;             /* device pointer           */

    printf( "Welcome to the shell!\r\n" );

    /* Enable interrupts */
    enable();

    hostptr = NULL;
    devptr = NULL;
    hostname_strsz = 0;
    bzero(hostnm, NET_HOSTNM_MAXLEN + 1);

    /* Setup buffer for string for nvramGet call for hostname */
#ifdef ETH0
    if (!isbaddev(ETH0))
    {
        /* Determine the hostname of the main network device */
        devptr = (device *)&devtab[ETH0];
        hostname_strsz = strnlen(NET_HOSTNAME, NVRAM_STRMAX) + 1;
        hostname_strsz += DEVMAXNAME;
        char nvramget_hostname_str[hostname_strsz];
        sprintf(nvramget_hostname_str, "%s_%s", devptr->name,
                NET_HOSTNAME);

        /* Acquire the backend's hostname */
#if NVRAM
        hostptr = nvramGet(nvramget_hostname_str);
#endif                          /* NVRAM */
        if (hostptr != NULL)
        {
            memcpy(hostnm, hostptr, NET_HOSTNM_MAXLEN);
            hostptr = hostnm;
        }
    }
#endif

    /* Set command devices for input, output, and error */
    stdin = indescrp;
    stdout = outdescrp;
    stderr = errdescrp;

    /* Print shell banner */
    printf(SHELL_BANNER);
    /* Print shell welcome message */
    printf(SHELL_START);

    /* Continually receive and handle commands */
    while (TRUE)
    {
        /* Display prompt */
        printf(SHELL_PROMPT);

        if (NULL != hostptr)
        {
            printf("@%s$ ", hostptr);
        }
        else
        {
            printf("$ ");
        }

        /* Setup proper tty modes for input and output */
        //control(stdin, TTY_CTRL_CLR_IFLAG, TTY_IRAW, NULL);
	control(stdin, TTY_CTRL_SET_IFLAG, TTY_IRAW, NULL);
        //control(stdin, TTY_CTRL_SET_IFLAG, TTY_ECHO, NULL);
	control(stdin, TTY_CTRL_CLR_IFLAG, TTY_ECHO, NULL);

        /* Read command */
        buflen = shellRead(stdin, buf, SHELL_BUFLEN - 1);

	printf("\n");

        /* Check for EOF and exit gracefully if seen */
        if (EOF == buflen)
        {
            break;
    	}

        /* Parse line input into tokens */
        if (SYSERR == (ntok = lexan(buf, buflen, &tokbuf[0], &tok[0])))
        {
            fprintf(stderr, SHELL_SYNTAXERR);
            continue;
        }

        /* Ensure parse generated tokens */
        if (0 == ntok)
        {
            continue;
        }

        /* Initialize command options */
        inname = NULL;
        outname = NULL;
        background = FALSE;

        /* Mark as background thread, if last token is '&' */
        if ('&' == *tok[ntok - 1])
        {
            ntok--;
            background = TRUE;
        }

        /* Check each token and perform special handling of '>' and '<' */
        for (i = 0; i < ntok; i++)
        {
            /* Background '&' should have already been handled; Syntax error */
            if ('&' == *tok[i])
            {
                ntok = -1;
                break;
            }

            /* Setup for output redirection if token is '>'  */
            if ('>' == *tok[i])
            {
                /* Syntax error */
                if (outname != NULL || i >= ntok - 1)
                {
                    ntok = -1;
                    break;
                }

                outname = tok[i + 1];
                ntok -= 2;

                /* shift tokens (not to be passed to command */
                for (j = i; j < ntok; j++)
                {
                    tok[j] = tok[j + 2];
                }
                continue;
            }

            /* Setup for input redirection if token is '<' */
            if ('<' == *tok[i])
            {
                /* Syntax error */
                if (inname != NULL || i >= ntok - 1)
                {
                    ntok = -1;
                    break;
                }
                inname = tok[i + 1];
                ntok -= 2;

                /* shift tokens (not to be passed to command */
                for (j = i; j < ntok; j++)
                {
                    tok[j] = tok[j + 2];
                }

                continue;
            }
        }

        /* Handle syntax error */
        if (ntok <= 0)
        {
            fprintf(stderr, SHELL_SYNTAXERR);
            continue;
        }

        /* Lookup first token in the command table */
        for (i = 0; i < ncommand; i++)
        {
            if (0 == strncmp(commandtab[i].name, tok[0], SHELL_BUFLEN))
            {
                break;
            }
        }

        /* Handle command not found */
        if (i >= ncommand)
        {
            fprintf(stderr, "%s: command not found\n", tok[0]);
            continue;
        }

        /* Handle command if it is built-in */
        if (commandtab[i].builtin)
        {
            if (inname != NULL || outname != NULL || background)
            {
                fprintf(stderr, SHELL_SYNTAXERR);
            }
            else
            {
                (*commandtab[i].procedure) (ntok, tok);
            }
            continue;
        }

        /* Spawn child thread for non-built-in commands */
        child =
            create(commandtab[i].procedure,
                   SHELL_CMDSTK, SHELL_CMDPRIO,
                   commandtab[i].name, 2, ntok, tok);

        /* Ensure child command thread was created successfully */
        if (SYSERR == child)
        {
            fprintf(stderr, SHELL_CHILDERR);
            continue;
        }

        /* Set file descriptors for newly created thread */
        if (NULL == inname)
        {
            thrtab[child].fdesc[0] = stdin;
        }
        else
        {
            thrtab[child].fdesc[0] = getdev(inname);
        }
        if (NULL == outname)
        {
            thrtab[child].fdesc[1] = stdout;
        }
        else
        {
            thrtab[child].fdesc[1] = getdev(outname);
        }
        thrtab[child].fdesc[2] = stderr;

        if (background)
        {
            /* Make background thread ready, but don't reschedule */
            im = disable();
            ready(child, RESCHED_NO);
            restore(im);
        }
        else
        {
            /* Clear waiting message; Reschedule; */
            while (recvclr() != NOMSG);
            im = disable();
            ready(child, RESCHED_YES);
            restore(im);

            /* Wait for command thread to finish */
            while (receive() != child);
            sleep(10);
        }
    }

    /* Close shell */
    fprintf(stdout, SHELL_EXIT);
    sleep(10);
    return OK;
}

int shellRead(int dev, char * buf, uint len){
	int count = 0;	//# of characters read
	int cLoc = 0; 	//cursor location
	int ch = 0;	//character for swtich comparison later
	int overwrite = 0; //character for overwriting with left arrow
	bool q = FALSE;	//break boolean to see if \r has been entered
	int cmdI = 0;	//command index for tab completion
	int k = 0;	//index in command name string
	int matchCount = 0;	//number of commands matched for tab completion
	do{
		read(dev, buf + count, 1);
		ch = buf[count];
		
		switch (ch)
		{
			// backspace or delete
			case '\b':
			case 0x7F:
				if ( count > 0 ){
					count--;
					cLoc--;
					if (count == cLoc){
						putc(dev, '\b');
						putc(dev, ' ');
						putc(dev, '\b');
					}
					else{
						int tmp = cLoc+1;
						while (tmp <= count){
							buf[tmp-1] = buf[tmp];
							tmp++;
						}
						tmp = cLoc;
						putc(dev, '\b');
						while (tmp < count){
							putc(dev, buf[tmp]);
							tmp++;
						}
						putc(dev, ' ');
						putc(dev, '\b');
						while (tmp > cLoc){
							putc(dev, '\b');
							tmp--;
						}
					}
				}
				break;
			// carraige return
			case '\r':
				q = TRUE;
				buf[count] = '\n';
				count++;
				cLoc++;
				break;
			// tab
			case '\t':
				while (cmdI < ncommand){
					k = 0;
					char * cmdname = commandtab[cmdI].name;
					while (k < count){
						if ( cmdname[k] != buf[k] )
							break;
						k++;
					}
					if (k == count && k != 0){
						int tmp = count;
						while ( tmp > 0 ){
							putc(dev, '\b');
							tmp--;
						}
						count = 0;
						while (1) {
							if (cmdname[count] == '\0')
								break;
							buf[count] = cmdname[count];
							putc(dev, buf[count]);
							count++;
						}
						cLoc = count;
						break;
					}
					cmdI++;
				}
				cmdI = 0;
				k = 0;
				break;
			// escape
			case 0x1B:
				read(dev, buf + count, 1);
				// check for [ character
				if (buf[count] == 0x5b){
					read(dev, buf + count, 1);
					int c = buf[count];
					int tmp = 0;
					switch (c)
					{
						case 0x41://Up Arrow
							history[histIndex].size = count;
							memcpy(history[histIndex].com, buf, SHELL_BUFLEN);

							histIndex = (histIndex + curHistElements - 1) % curHistElements;
							
							
							while (tmp < count){
								putc(dev, '\b');
								tmp++;
							}
							tmp = 0;
							while (tmp < count){
								putc(dev, ' ');
								tmp++;
							}
							tmp = 0;
							while (tmp < count){
								putc(dev, '\b');
								tmp++;
							}

							count = history[histIndex].size;
							memcpy(buf, history[histIndex].com, SHELL_BUFLEN);

							tmp = 0;
							while (tmp < count){
								if (buf[tmp] != '\n')
									putc(dev, buf[tmp]);
								else{
									count--;
									break;
								}
								tmp++;
							}
							cLoc = count;

							break;
						case 0x42://Down Arrow
							history[histIndex].size = count;
							memcpy(history[histIndex].com, buf, SHELL_BUFLEN);

							histIndex = (histIndex + 1) % curHistElements;
							
							//tmp = 0;
							while (tmp < count){
								putc(dev, '\b');
								tmp++;
							}
							tmp = 0;
							while (tmp < count){
								putc(dev, ' ');
								tmp++;
							}
							tmp = 0;
							while (tmp < count){
								putc(dev, '\b');
								tmp++;
							}

							count = history[histIndex].size;
							memcpy(buf, history[histIndex].com, SHELL_BUFLEN);

							tmp = 0;
							while (tmp < count){
								if (buf[tmp] != '\n')
									putc(dev, buf[tmp]);
								else{
									count--;
									break;
								}
								tmp++;
							}
							cLoc = count;

							break;
						case 0x43://Right Arrow
							if (cLoc < count){
								putc(dev, buf[cLoc]);
								cLoc++;
							}
							break;
						case 0x44://Left Arrow
							if (cLoc > 0){
								putc(dev, '\b');
								cLoc--;
							}
							break;
						default:
							//putc(dev, c);
							break;
					}
				}
				break;
			default:
				//count++;
				//cLoc++;
				putc(dev, ch);
				overwrite = buf[count];
				if (cLoc < count){
					int temp = count+1;
					while (temp >= cLoc){
						buf[temp+1] = buf[temp];
						temp--;
					}
					buf[cLoc] = overwrite;
					temp+=2;

					while (temp <= count){
						putc(dev, buf[temp]);
						temp++;
					}

					while (temp > cLoc+1){
						putc(dev, '\b');
						temp--;
					}
				}
				count++;
				cLoc++;
				break;
		}
		
	} while ( count < len && !q);

	if (buf[count-1] == '\r')
		buf[count-1] = '\n';
	
	if ( count > 1 ){
		history[curHistIndex].size = count;
		memcpy(history[curHistIndex].com, buf, SHELL_BUFLEN);
		curHistElements++;
		if (curHistElements > MAX_HISTORY)
			curHistElements = MAX_HISTORY;
		curHistIndex = (curHistIndex + 1) % curHistElements;
		histIndex = curHistIndex;
	}

	return count;
}
