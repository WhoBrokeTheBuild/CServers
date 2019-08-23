#include "config.h"

#include "log.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct mime 
{
    struct mime *   next;
    char *          type_id;
    char *          type_name;
    char *          subtype_name;
    char **         extensions;
    size_t          extension_count;
} mime_t;

static mime_t _root_mime = { 0 };

// https://tools.ietf.org/html/rfc6838#section-4.2
static const size_t MAX_MIME_TYPE_LENGTH = 127;
static const size_t MAX_MIME_EXT_LENGTH  = 32;

void parse_mime_config(const char * file)
{
    FILE * fp       = NULL;
    size_t len      = 0;
    ssize_t read    = 0;
    char * line     = NULL;
    char * token    = NULL;

    size_t id_len       = 0;
    size_t type_len     = 0;
    size_t subtype_len  = 0;

    mime_t * last_mime  = NULL;
    mime_t * new_mime   = NULL;

    size_t new_ext_count    = 0;
    char ** new_exts        = NULL;

    fp = fopen(file, "r");
    if (!fp) {
        log_error("fopen failed: %s", strerror(errno));
        goto cleanup;
    }

    last_mime = &_root_mime;
    while (last_mime->next) {
        last_mime = last_mime->next;
    }

    while ((read = getline(&line, &len, fp)) != -1) {
        if (len > 0) {
            new_mime = (mime_t *)malloc(sizeof(mime_t));
            memset(new_mime, 0, sizeof(mime_t));

            token = strtok(line, "/");

            if (!token) {
                log_error("strtok failed: %s", strerror(errno));
                goto cleanup;
            }

            type_len = strlen(token);
            if (type_len >= MAX_MIME_TYPE_LENGTH) {
                log_error("maximum mime type length of %zu exceeded: %s", MAX_MIME_TYPE_LENGTH, token);
                goto cleanup;
            }

            new_mime->type_name = strndup(token, MAX_MIME_TYPE_LENGTH);
            if (!new_mime->type_name) {
                log_error("strndup failed: %s", strerror(errno));
                goto cleanup;
            }

            token = strtok(NULL, " \t");

            if (!token) {
                log_error("strtok failed: %s", strerror(errno));
                goto cleanup;
            }

            subtype_len = strlen(token);
            if (subtype_len >= MAX_MIME_TYPE_LENGTH) {
                log_error("maximum mime subtype length of %zu exceeded: %s", MAX_MIME_TYPE_LENGTH, token);
                goto cleanup;
            }

            new_mime->subtype_name = strndup(token, MAX_MIME_TYPE_LENGTH);
            if (!new_mime->subtype_name) {
                log_error("strndup failed: %s", strerror(errno));
                goto cleanup;
            }

            id_len = type_len + 1 + subtype_len;
            new_mime->type_id = (char *)malloc(sizeof(char) * id_len + 1);

            if (!new_mime->type_id) {
                log_error("malloc failed: %s", strerror(errno));
                goto cleanup;
            }

            strcat(strcat(strcat(new_mime->type_id, new_mime->type_name), "/"), new_mime->subtype_name);

            new_mime->extension_count = 0;
            new_mime->extensions = NULL;

            token = strtok(NULL, " \t\n");
            while (token) {
                if (strlen(token) >= MAX_MIME_EXT_LENGTH) {
                    log_error("maximum mime extension length of %zu exceeded: %s", MAX_MIME_EXT_LENGTH, token);
                    goto cleanup;
                }

                new_ext_count = new_mime->extension_count + 1;
                new_exts = (char **)realloc(new_mime->extensions, sizeof(char **) * new_ext_count);
                
                if (!new_exts) {
                    log_error("realloc failed: %s", strerror(errno));
                    goto cleanup;
                }

                new_exts[new_ext_count - 1] = strndup(token, MAX_MIME_EXT_LENGTH);
                new_mime->extension_count = new_ext_count;
                new_mime->extensions = new_exts;

                token = strtok(NULL, " \t\n");
            }

            log_info("loaded %s", new_mime->type_id);

            last_mime->next = new_mime;
            last_mime = last_mime->next;
            new_mime = NULL;
        }
    }

cleanup:
    if (new_mime) {
        free(new_mime->type_name);
        free(new_mime->subtype_name);
        for (size_t i = 0; i < new_mime->extension_count; ++i) {
            free(new_mime->extensions[i]);
        }
        free(new_mime->extensions);
        free(new_mime);
    }

    free(line);

    fclose(fp);
}

void free_mime_config()
{
    mime_t * mime = NULL;

    mime = &_root_mime;
    while (mime) {
        free(mime->type_id);
        mime->type_id = NULL;

        free(mime->type_name);
        mime->type_name = NULL;

        free(mime->subtype_name);
        mime->subtype_name = NULL;

        for (size_t i = 0; i < mime->extension_count; ++i) {
            free(mime->extensions[i]);
        }
        free(mime->extensions);
        mime->extensions = NULL;

        mime = mime->next;
    }

    while (_root_mime.next) {
        mime = _root_mime.next;
        _root_mime.next = mime->next;
        free(mime);
    }
}

const char * get_mime_type(const char * ext)
{
    mime_t * mime = NULL;

    mime = &_root_mime;
    while (mime) {
        for (size_t i = 0; i < mime->extension_count; ++i) {
            if (strncmp(mime->extensions[i], ext, MAX_MIME_EXT_LENGTH) == 0) {
                return mime->type_id;
            }
        }
        mime = mime->next;
    }

    return NULL;
}
