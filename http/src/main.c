
#include "config.h"
#include "log.h"

int main(
    __attribute__((unused)) int argc, 
    __attribute__((unused)) char** argv
    )
{
    parse_mime_config("example/mime.conf");

    char * tests[] = {
        "html",
        "css",
        "asdf",
        "pdf",
        "gif",
        "mp3",
        "ogg",
        "js",
        "jpeg",
        "rpm",
    };

    const size_t test_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < test_count; ++i) {
        const char * mime = get_mime_type(tests[i]);
        if (mime) {
            log_info("%s = %s", tests[i], mime);
        } else {
            log_info("%s = not found", tests[i]);
        }
    }

    free_mime_config();
    return 0;
}
