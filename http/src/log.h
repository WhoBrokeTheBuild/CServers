#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define log_info(M, ...) \
    do { log_(stdout, M "\n", ##__VA_ARGS__); } while (0)

#define log_error(M, ...) \
    do { log_(stdout, M "\n", ##__VA_ARGS__); } while (0)

#define log_access(M, ...) \
    do { log_(stdout, M "\n", ##__VA_ARGS__); } while (0)

void log_(FILE * fp, const char * format, ...);

#pragma clang diagnostic pop

#pragma GCC diagnostic pop
