#include <stdio.h>
#include <stdlib.h>
#include <uvcurl/uvcurl.h>
uvcurl_multi_t* multi = NULL;
void cb(CURL* curl)
{
    fprintf(stderr, "curl[%p] done\n", curl);
}
void add_download(const char *url, int num) {
    char filename[50];
    sprintf(filename, "%d.download", num);
    FILE *file;

    file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening %s\n", filename);
        return;
    }

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_URL, url);
    uvcurl_multi_add_easy(multi, curl, cb);
    fprintf(stderr, "curl[%p] Added download %s -> %s\n", curl, url, filename);
}

int main(int argc, char** argv)
{
    uv_loop_t* loop = uv_default_loop();
    if (argc <= 1)
        return 0;

    if (curl_global_init(CURL_GLOBAL_DEFAULT)) {
        fprintf(stderr, "Could not init cURL\n");
        return 1;
    }

    
    multi = uvcurl_multi_init( loop);
    while (argc-- > 1) {
        add_download(argv[argc], argc);
    }

    uv_run(loop, UV_RUN_DEFAULT);
    uvcurl_multi_cleanup(multi);
    return 0;
}