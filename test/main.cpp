#include <stdio.h>
#include <stdlib.h>
#include <uvcurl/Multi.hpp>
#include <memory>
#if __cplusplus >= 201402L
#elif __cplusplus >= 201103L
namespace std
{
template<class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}
#else
#error("error c++ std less 11")
#endif
using namespace std;

void add_download(shared_ptr<cppuvcurl::Multi> multi, const char *url, int num) {
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
    multi->async_preform(curl, [num](CURL* curl){
        printf("curl[%p] num[%d] done!\n", curl, num);
    });
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

    auto multi = make_shared<cppuvcurl::Multi>(loop);
    
    
    while (argc-- > 1) {
        add_download(multi, argv[argc], argc);
    }

    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}