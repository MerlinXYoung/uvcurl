#include <uvcurl/Multi.hpp>
#include <memory>
#include <functional>
#include <cassert>
#include <sstream>
#include <openssl/md5.h>
#include <cstring>

using namespace std;
using namespace uvcurl;
std::shared_ptr<uvcurl::Multi> g_multi;

class QQAuth 
{
    const char* qq_auth_url="http://ysdktest.qq.com/auth/qq_check_token";
    const char* qq_appid = "1106662470";
    const char* qq_appkey = "ZQMqEM5I4m5jx68q";

    struct qq_auth_context_t
    {
        uint32_t client_id_;
        CURL* curl_;
        std::string data_;
        qq_auth_context_t(uint32_t client_id, CURL* curl):
            client_id_(client_id),curl_(curl),data_(){}
    };
public:
    int do_auth(uint32_t client_id, const char* openid, const char* appkey) ;
};


int QQAuth::do_auth(uint32_t client_id, const char* openid, const char* appkey) 
{
    time_t now = time(NULL);
    char md5_src[100];
    snprintf(md5_src, sizeof(md5_src),"%s%ld", qq_appkey, now);
    unsigned char md5[16];
    MD5((const unsigned char*)md5_src, strlen(md5_src), md5);
    char sig[33]={'\0'};
    char tmp[3]={'\0'};
    for(int  i=0; i<16; i++ ){
        sprintf(tmp,"%02x",md5[i]);
        strcat(sig,tmp);
    }
    std::stringstream ss;
    ss<<qq_auth_url<<"?timestamp="<<now<<"&appid="<<qq_appid<<"&sig="<<sig<<"&openid="<<openid<<"&openkey="<<appkey;//<<"&userip"
    std::string url = ss.str();
    CURL *handle = curl_easy_init();

    auto context = new qq_auth_context_t(client_id, handle);
#if 1
    typedef size_t (*write_callback_t)(char *ptr, size_t size, size_t nmemb, void *userdata);
    CURLcode code = curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION,
     (write_callback_t)([](char *ptr, size_t size, size_t nmemb, void *userdata)->size_t{
        printf("ptr[%p] size[%lu] nmemb[%lu] userdata[%p]|n", ptr, size, nmemb, userdata);
        size_t xSize = size*nmemb;
        std::string * pStr = reinterpret_cast<std::string*>(userdata);
        if ( pStr )
            pStr->append((char*)ptr, xSize);

        return xSize;
    })
    );
    printf("code[%d]\n", code);
    code = curl_easy_setopt(handle, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&(context->data_)));
    printf("code[%d]\n", code);
#else
    char filename[50];
    sprintf(filename, "qqauth");
    FILE *file;

    file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening %s\n", filename);
        return -1;
    }
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, file);
#endif
    
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());

    //curl_multi_add_handle(curl_handle, handle);
    g_multi->async_preform(handle, [context](CURL* curl){
        printf("done curl[%p]\n", curl);
        assert(curl == context->curl_);
        long res_code =0; 
        char *done_url;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL,
                            &done_url);
        int res=curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res_code);
        printf("client[%u] url[%s] http_code[%ld]\n", context->client_id_, done_url, res_code);
        // auto client = ClientMgr::instance().Get(context->client_id_);
        // if(!client)
        //     return ;
        //正确响应后，请请求转写成本地文件的文件
        if(( res == CURLE_OK ) && (res_code == 200 || res_code == 201))
        {
            printf("http rsp[%s]\n", context->data_.c_str());
            // client->auth_cb(0);
        }


    });

    fprintf(stderr, "Added %s \n", url.c_str());
}
int main(int argc, char** argv)
{
    uv_loop_t* loop = uv_default_loop();
    g_multi = make_shared<Multi>(loop);

    QQAuth auth;
    auth.do_auth(789, "1234", "9876");

    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}