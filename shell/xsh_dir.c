/**
 * @file     xsh_clear.c
 * @provides xsh_clear.
 *
 * $Id: xsh_clear.c 2065 2009-09-04 21:44:36Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <filesystem.h>

/**
 * Shell command (cd) changes the current working directory.
 * @param nargs number of arguments
 * @param args  array of arguments
 * @return non-zero value on error
 */
shellcmd xsh_dir(int nargs, char *args[])
{
    /* Output help, if '--help' argument was supplied */
    if (nargs == 2 && strncmp(args[1], "--help", 7) == 0)
    {
        printf("Usage: %s [dir]\n\n", args[0]);
        printf("Description:\n");
        printf("\nEnumerates through files and subfolders of a directory.\n");
        printf("Options:\n");
        printf("\t--help\t display this help and exit\n");
        printf("\t[dir]\t Optionally, a directory to enumerate through. Defaults to the working directory.\n");
        return 1;
    }

    /* Check for correct number of arguments */
    if (nargs > 2)
    {
        fprintf(stderr, "%s: too many arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information.\n",
                args[0]);
        return 1;
    }

    fshandle handle;
    errcode err = CreateFile(nargs > 1 ? args[1] : ".", &handle, FSMODE_OPEN | FSMODE_DIR, FSACCESS_READ);

    if (err)
    {
        printf("Error opening directory.\n");
        return 1;
    }
    else
    {
        char nameBuffer[MAXNAME + 1];
        int i = 0;
        while (!EnumFiles(handle, i++, nameBuffer))
        {
            printf("%s\n", nameBuffer);
            memset(nameBuffer, 0, MAXNAME + 1);
        }
        CloseFile(handle);
    }

    return 0;
}