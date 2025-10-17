#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(__has_include)
#  if __has_include("cJSON.h")
#    include "cJSON.h"
#  elif __has_include(<cjson/cJSON.h>)
#    include <cjson/cJSON.h>
#  else
/* Minimal cJSON stub to allow compilation when cJSON is not installed.
   This stub does not implement real JSON parsing; it only provides the
   symbols used by this program so it can be built without the cJSON library. */
typedef struct cJSON {
    struct cJSON *next;
    struct cJSON *child;
    char *string;
    char *valuestring;
} cJSON;

static inline cJSON *cJSON_Parse(const char *s) { (void)s; return NULL; }
static inline void cJSON_Delete(cJSON *item) { (void)item; }
static inline cJSON *cJSON_GetObjectItem(cJSON *object, const char *string) { (void)object; (void)string; return NULL; }
static inline int cJSON_IsArray(const cJSON *item) { (void)item; return 0; }
static inline int cJSON_IsObject(const cJSON *item) { (void)item; return 0; }
static inline int cJSON_IsString(const cJSON *item) { (void)item; return 0; }
static inline const char *cJSON_GetStringValue(const cJSON *item) { return item ? item->valuestring : NULL; }
#define cJSON_ArrayForEach(element, array) for (element = NULL; element; element = element->next)

#  endif
#else
#include "cJSON.h"
#endif

#define MIRROR_FILE "mirrors.json"
#define DEFAULT_STORAGE_MB 2048 // Default Storage allocation in MB

// Function to load mirrors and check for low or no host support
void check_host_support(int allocated_storage_mb) {
    FILE *file = fopen(MIRROR_FILE, "r");
    if (!file) {
        perror("Error opening mirror file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(length + 1);
    if (!data) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    cJSON *json = cJSON_Parse(data);
    free(data);

    if (!json) {
        fprintf(stderr, "Error parsing JSON\n");
        return;
    }

    cJSON *mirrors = cJSON_GetObjectItem(json, "mirrors");
    if (!mirrors || !cJSON_IsArray(mirrors)) {
        fprintf(stderr, "Invalid JSON format\n");
        cJSON_Delete(json);
        return;
    }

    printf("Checking mirrors for low or no host support...\n");
    cJSON *mirror;
    cJSON_ArrayForEach(mirror, mirrors) {
        cJSON *url = cJSON_GetObjectItem(mirror, "url");
        cJSON *packages = cJSON_GetObjectItem(mirror, "packages");
        cJSON *signing_details = cJSON_GetObjectItem(mirror, "signing_details");

        if (url && cJSON_IsString(url) && packages && cJSON_IsObject(packages)) {
            printf("Mirror: %s\n", url->valuestring);

            cJSON *package;
            cJSON_ArrayForEach(package, packages) {
                const char *package_name = package->string;
                const char *version = cJSON_GetStringValue(package);

                if (package_name && version) {
                    printf("  Package: %s, Version: %s\n", package_name, version);
                    // TODO: Check host support logic here
                }
            }
        }

        if (signing_details && cJSON_IsObject(signing_details)) {
            printf("  Signing details available.\n");
        } else {
            printf("  Warning: No signing details provided.\n");
        }
    }

    printf("Allocated storage: %d MB\n", allocated_storage_mb);

    cJSON_Delete(json);
}

int main(int argc, char *argv[]) {
    int allocated_storage_mb = DEFAULT_STORAGE_MB;

    if (argc > 1) {
        allocated_storage_mb = atoi(argv[1]);
        if (allocated_storage_mb <= 0) {
            printf("Invalid storage allocation. Using default: %d MB\n", DEFAULT_STORAGE_MB);
            allocated_storage_mb = DEFAULT_STORAGE_MB;
        }
    }

    printf("Starting host management with %d MB allocated storage...\n", allocated_storage_mb);
    check_host_support(allocated_storage_mb);

    return 0;
}