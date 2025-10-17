#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#if defined(__has_include)
#  if __has_include(<cjson/cJSON.h>)
#    include <cjson/cJSON.h>
#  elif __has_include("cJSON.h")
#    include "cJSON.h"
#  else
#include <stddef.h>
#include <stdbool.h>

typedef struct cJSON {
    int type;
    char *valuestring;
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;
} cJSON;

static inline cJSON *cJSON_Parse(const char *value) { (void)value; return NULL; }
static inline void cJSON_Delete(cJSON *item) { (void)item; }
static inline cJSON *cJSON_GetObjectItem(const cJSON * const object, const char * const string) { (void)object; (void)string; return NULL; }
static inline int cJSON_IsArray(const cJSON * const item) { (void)item; return 0; }
static inline int cJSON_IsString(const cJSON * const item) { (void)item; return 0; }
static inline cJSON *cJSON_GetArrayItem(const cJSON *array, int index) { (void)array; (void)index; return NULL; }
#define cJSON_ArrayForEach(element, array) for((element) = NULL; element; (element) = (element)->next)

#  endif
#else
#  include "cJSON.h"
#endif

#define MIRROR_FILE "mirrors.json"
#define PACKAGE_DIR "packages"
#define DEFAULT_MIRROR_URL "https://raw.githubusercontent.com/Sacrafex/SPRY/main/mirrors.json"

// Function to download a file using system curl/wget
int download_file(const char *url, const char *output_path) {
    char cmd[1024];
    int ret;

    // Try curl first (quiet, fail on error, follow redirects)
    if (snprintf(cmd, sizeof(cmd), "curl -fsSL --retry 3 -o '%s' '%s'", output_path, url) >= (int)sizeof(cmd)) {
        fprintf(stderr, "URL or path too long\n");
        return 1;
    }
    ret = system(cmd);
    if (ret == 0) return 0;

    // Fallback to wget
    if (snprintf(cmd, sizeof(cmd), "wget -q -O '%s' '%s'", output_path, url) >= (int)sizeof(cmd)) {
        fprintf(stderr, "URL or path too long\n");
        return 1;
    }
    ret = system(cmd);
    if (ret == 0) return 0;

    fprintf(stderr, "Download failed: neither curl nor wget succeeded\n");
    return 1;
}

// Function to ensure the mirror file exists
void ensure_mirror_file() {
    struct stat st;
    if (stat(MIRROR_FILE, &st) != 0) {
        printf("Mirror file not found. Downloading default mirror file...\n");
        if (download_file(DEFAULT_MIRROR_URL, MIRROR_FILE) == 0) {
            printf("Default mirror file downloaded successfully.\n");
        } else {
            fprintf(stderr, "Failed to download default mirror file.\n");
            exit(1);
        }
    }
}

// Function to load mirrors from the JSON mirror file
void load_mirrors() {
    ensure_mirror_file();

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

    printf("Available mirrors:\n");
    cJSON *mirror;
    cJSON_ArrayForEach(mirror, mirrors) {
        cJSON *url = cJSON_GetObjectItem(mirror, "url");
        if (url && cJSON_IsString(url)) {
            printf("- %s\n", url->valuestring);
        }
    }

    cJSON_Delete(json);
}

// Function to install a package
void install_package(const char *package_name) {
    ensure_mirror_file();

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

    cJSON *mirror = cJSON_GetArrayItem(mirrors, 0); // Use the first mirror
    if (!mirror) {
        fprintf(stderr, "No mirrors available\n");
        cJSON_Delete(json);
        return;
    }

    cJSON *url = cJSON_GetObjectItem(mirror, "url");
    if (!url || !cJSON_IsString(url)) {
        fprintf(stderr, "Invalid mirror URL\n");
        cJSON_Delete(json);
        return;
    }

    char package_url[512];
    char package_path[512];

    snprintf(package_url, sizeof(package_url), "%s/%s.tar.gz", url->valuestring, package_name);
    snprintf(package_path, sizeof(package_path), "%s/%s.tar.gz", PACKAGE_DIR, package_name);

    printf("Downloading package from: %s\n", package_url);
    if (download_file(package_url, package_path) == 0) {
        printf("Package '%s' downloaded successfully.\n", package_name);
        // TODO: Extract the package and parse metadata.json
    }

    cJSON_Delete(json);
}

// Function to list installed packages
void list_packages() {
    struct stat st = {0};

    if (stat(PACKAGE_DIR, &st) == -1) {
        printf("No packages installed.\n");
        return;
    }

    printf("Installed packages:\n");
    // TODO: Iterate through PACKAGE_DIR and list packages
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command> [package_name]\n", argv[0]);
        printf("Commands:\n");
        printf("  install <package_name>  Install a package\n");
        printf("  remove <package_name>   Remove a package\n");
        printf("  list                    List all packages\n");
        printf("  update                  Update mirrors and packages\n");
        return 1;
    }

    if (strcmp(argv[1], "install") == 0) {
        if (argc < 3) {
            printf("Please provide a package name to install.\n");
            return 1;
        }
        install_package(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        list_packages();
    } else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }

    return 0;
}