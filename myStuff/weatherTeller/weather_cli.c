#include <curl/curl.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t len;
} ResponseBuffer;

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    ResponseBuffer *buf = (ResponseBuffer *)userp;

    char *new_data = realloc(buf->data, buf->len + total + 1);
    if (new_data == NULL) {
        return 0;
    }

    buf->data = new_data;
    memcpy(buf->data + buf->len, contents, total);
    buf->len += total;
    buf->data[buf->len] = '\0';

    return total;
}

static int is_valid_zip(const char *zip) {
    size_t len = strlen(zip);
    if (len != 5) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        if (zip[i] < '0' || zip[i] > '9') {
            return 0;
        }
    }

    return 1;
}

static int is_valid_country(const char *country) {
    size_t len = strlen(country);
    if (len < 2 || len > 3) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        if (!isalpha((unsigned char)country[i])) {
            return 0;
        }
    }

    return 1;
}

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [--json|-j] <zip_code> [country_code]\n", program_name);
    fprintf(stderr, "Example (text): %s 10001\n", program_name);
    fprintf(stderr, "Example (country): %s 10001 US\n", program_name);
    fprintf(stderr, "Example (json): %s --json 10001 US\n", program_name);
}

int main(int argc, char **argv) {
    int json_output = 0;
    const char *zip = NULL;
    const char *country = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--json") == 0 || strcmp(argv[i], "-j") == 0) {
            json_output = 1;
            continue;
        }

        if (zip == NULL) {
            zip = argv[i];
            continue;
        }

        if (country == NULL) {
            country = argv[i];
            continue;
        }

        print_usage(argv[0]);
        return 1;
    }

    if (zip == NULL) {
        print_usage(argv[0]);
        return 1;
    }

    if (country == NULL) {
        country = "US";
    }

    if (!is_valid_zip(zip)) {
        fprintf(stderr, "Error: ZIP code must be 5 numeric digits.\n");
        return 1;
    }

    if (!is_valid_country(country)) {
        fprintf(stderr, "Error: country code must be 2-3 alphabetic characters (e.g., US, CA, GBR).\n");
        return 1;
    }

    int status = 1;
    CURLcode res;
    CURL *curl = NULL;
    ResponseBuffer response = {0};

    char url[256];
    if (json_output) {
        snprintf(url, sizeof(url), "https://wttr.in/%s,%s?format=j1", zip, country);
    } else {
        snprintf(url, sizeof(url), "https://wttr.in/%s,%s?format=%%C+%%t", zip, country);
    }

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        fprintf(stderr, "Error: failed to initialize libcurl.\n");
        return 1;
    }

    curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "Error: failed to create CURL handle.\n");
        goto cleanup;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "weather-cli/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        fprintf(stderr, "Weather service returned HTTP %ld\n", http_code);
        goto cleanup;
    }

    if (response.len == 0) {
        fprintf(stderr, "Weather service returned empty response.\n");
        goto cleanup;
    }

    if (json_output) {
        printf("%s\n", response.data);
    } else {
        printf("ZIP %s (%s): %s\n", zip, country, response.data);
    }
    status = 0;

cleanup:
    free(response.data);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return status;
}
