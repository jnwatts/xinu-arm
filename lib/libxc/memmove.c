/**
 * @file memcpy.c
 * @provides memcpy.
 *
 * $Id: memcpy.c 2020 2009-08-13 17:50:08Z mschul $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. */

/**
 * Memory copy, copy a location in memory from src to dst.
 * @param s destination location
 * @param ct source location
 * @param n amount of data (in bytes) to copy
 * @return pointer to destination
 */
void *memmove(void *s, const void *ct, int n)
{
    register int i;
    char *dst;
    char *src;

    if (s == ct || n <= 0)
    	return s;
    else if (s < ct)
    {
    	dst = (char *)s;
    	src = (char *)ct;
    	for (i = 0; i < n; i++)
    	{
        	*dst++ = *src++;
    	}
	}
	else
	{
		dst = ((char*)s) + (n - 1);
		src = ((char*)ct) + (n - 1);
		for (i = 0; i < n; i++)
    	{
        	*dst-- = *src--;
    	}
	}
    return s;
}
