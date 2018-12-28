#ifndef _ERROR_H
# define _ERROR_H 1

extern void error(int __status, int __errnum, const char *__format, ...);
extern void error_at_line (int __status, int __errnum, const char *__fname,
                           unsigned int __lineno, const char *__format, ...);
#endif
