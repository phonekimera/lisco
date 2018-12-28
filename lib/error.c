#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

static char *progname = "tate";

void
error(int status, int errnum, const char *format, ...)
{
    va_list args;

    fprintf(stderr, "%s: ", progname);
    if (errnum) {
        fprintf(stderr, "%s: ", strerror(errnum));
    }

    va_start(args, format);
    vfprintf(stderr, format, args); 
    va_end(args);

    fputs("\n", stderr);

    if (status) exit(status);
}

void
error_at_line(int status, int errnum, const char *filename, int lineno,
              const char *format, ...)
{
    va_list args;

    fprintf(stderr, "%s:", progname);
    if (errnum) {
        fprintf(stderr, " %s:", strerror(errnum));
    }
    
    fprintf(stderr, "%s:%d: ", filename, lineno);

    va_start(args, format);
    vfprintf(stderr, format, args); 
    va_end(args);

    fputs("\n", stderr);

    if (status) exit(status);
}
