#include "log.h"

#include <stdarg.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"

void log_(FILE * fp, const char * format, ...)
{
    va_list vargs;
    va_start(vargs, format);
    vfprintf(fp, format, vargs);
    va_end(vargs);
}

#pragma clang diagnostic pop

#pragma GCC diagnostic pop
