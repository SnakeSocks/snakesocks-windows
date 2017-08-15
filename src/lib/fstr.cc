#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "syserr.hpp"
/*
 * This function is adapted from linux glibc demo.
 * with GNU license.
 */
char *
fstr(const char *fmt, ...)
{
    int n;
    int size = 100;     /* Guess we need no more than 100 bytes */
    char *p, *np;
    va_list ap;

    if ((p = (char *)malloc(size)) == NULL)
        die("malloc returns null.");

    while (1) {

        /* Try to print in the allocated space */

        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);

        /* Check error code */

        if (n < 0)
            die("vsnprintf returns %d.", n);

        /* If that worked, return the string */

        if (n < size)
            return p;

        /* Else try again with more space */

        size = n + 1;       /* Precisely what is needed */

        if ((np = (char *)realloc (p, size)) == NULL) {
            free(p);
            die("make_message realloc failed.");
            return NULL;
        } else {
            p = np;
        }
    }
}
