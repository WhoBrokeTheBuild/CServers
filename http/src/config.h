#include <stdint.h>

typedef struct {
    const char * pid_file;
    const char * error_log_file;
    const char * access_log_file;
} config_t;

void parse_mime_config(const char * file);
void free_mime_config(void);

const char * get_mime_type(const char * ext);
