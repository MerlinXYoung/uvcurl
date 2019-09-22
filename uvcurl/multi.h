#pragma once
#include <uv.h>
#include <curl/curl.h>
#ifdef __cplusplus
extern "C" {
#endif
#if 0
typedef struct uvcurl_multi_s
{
    uv_timer_t timer_;
    CURLM* curlm_;
}uvcurl_multi_t;
#else
typedef struct uvcurl_multi_s uvcurl_multi_t;
#endif

typedef void (*done_cb_t)(CURL* curl);
uvcurl_multi_t* uvcurl_multi_init(uv_loop_t* loop);
uvcurl_multi_t* uvcurl_multi_init_with_default_loop();
void uvcurl_multi_cleanup(uvcurl_multi_t* multi);
CURLMcode uvcurl_multi_add_easy(uvcurl_multi_t* multi, CURL* easy, done_cb_t done_cb);
uv_loop_t* uvcurl_multi_get_loop(uvcurl_multi_t* multi);

#ifdef __cplusplus
}
#endif