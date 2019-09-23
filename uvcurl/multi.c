
#include "multi.h"
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "macro.h"
#define __UVCURL_LOG__
#ifdef __UVCURL_LOG__
#define uvcurl_log(fmt, ...) \
printf("[0x%lX]%s:%u:" fmt "\n",pthread_self(), __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define uvcurl_log(fmt, ...) 
#endif
#define uvcurl_check_ret(res) \
do{ \
    if(0 != res){ \
        uvcurl_log("fuck uvcurl_check_ret[%d]", res); \
        return res ; \
    } \
}while(0)

#define uvcurl_check_ptr_ret(ptr, res) \
do{ \
    if(NULL == ptr){ \
        uvcurl_log("fuck uvcurl_check_ptr_ret"); \
        return res; \
    } \
}while(0)

typedef struct uvcurl_curl_private_s
{
    uvcurl_curl_done_cb_t cb;
    void * data;
}uvcurl_curl_private_t;
typedef struct uvcurl_multi_s
{
    uv_timer_t timer_;
    CURLM* curlm_;
}uvcurl_multi_t;

typedef struct curl_context_s {
    uv_poll_t pool_;
    curl_socket_t sockfd;
    uvcurl_multi_t* multi_;
} curl_context_t;

static curl_context_t *_create_curl_context(uvcurl_multi_t* multi, curl_socket_t sockfd) {
    uvcurl_log("");
    assert(NULL != multi);
    uvcurl_malloc_obj(curl_context_t, context);
    if(NULL == context)
        return NULL;
    context->sockfd = sockfd;
    context->multi_ = multi;

    int r = uv_poll_init_socket(multi->timer_.loop, &context->pool_, sockfd);
    assert(r == 0);
    context->pool_.data = context;

    return context;
}

static void _curl_close_cb(uv_handle_t *handle) {
    uvcurl_log("");
    assert(NULL != handle);
    curl_context_t *context = (curl_context_t*) handle->data;
    free(context);
}

static void _destroy_curl_context(curl_context_t *context) {
    uvcurl_log("context[%p]",context);
    assert(NULL != context);
    uv_close((uv_handle_t*) &context->pool_, _curl_close_cb);
}

uvcurl_multi_t* uvcurl_multi_init_default_uv_loop()
{
    uvcurl_log("");
    return uvcurl_multi_init(uv_default_loop());
}

static void _check_multi_info(uvcurl_multi_t* multi) {
    uvcurl_log("multi[%p]", multi);
    assert(NULL != multi);
    char *done_url;
    CURLMsg *message;
    int pending;

    while ((message = curl_multi_info_read(multi->curlm_, &pending))) {
        switch (message->msg) {
        case CURLMSG_DONE:
            curl_easy_getinfo(message->easy_handle, CURLINFO_EFFECTIVE_URL,
                            &done_url);
            printf("curl[%p] url[%s] DONE\n", message->easy_handle, done_url);
            uvcurl_curl_private_t* p=NULL;
            
            CURLcode code= curl_easy_getinfo(message->easy_handle, CURLINFO_PRIVATE, (char**)&p);
            assert(0 == code && NULL != p);
            uvcurl_log("code[%d] p[%p] cb[%p] data[%p]", code, p, p->cb, p->data);
            curl_multi_remove_handle(multi->curlm_, message->easy_handle);
            if(NULL != p)
            {
                p->cb(message->easy_handle, p->data);
                free(p);
            }    
            curl_easy_cleanup(message->easy_handle);
            break;

        default:
            fprintf(stderr, "CURLMSG default\n");
            abort();
        }
    }
}
static void _timer_cb(uv_timer_t* handle)
{
    uvcurl_log("");
    assert(NULL != handle);
    uvcurl_multi_t* multi = (uvcurl_multi_t*)handle->data;
    int running_handles;

    curl_multi_socket_action(multi->curlm_, CURL_SOCKET_TIMEOUT, 0, &running_handles);
    _check_multi_info(multi);
}
static void _start_timeout(CURLM *curlm, long timeout_ms, void *userp) {
    uvcurl_log("");
    assert(NULL != curlm);
    uvcurl_multi_t* multi = (uvcurl_multi_t*)userp;
    if (timeout_ms <= 0)
        timeout_ms = 1; /* 0 means directly call socket_action, but we'll do it in a bit */
        uvcurl_log("timer[%p] timeout_ms[%ld]", &multi->timer_, timeout_ms);
    uv_timer_start(&multi->timer_, _timer_cb, timeout_ms, 0);
}
static void _curl_perform(uv_poll_t *req, int status, int events) {
    uvcurl_log("req[%p] status[%d] events[%d]", req, status, events);
    curl_context_t *context = (curl_context_t*)req;
    uvcurl_multi_t* multi = context->multi_;
    uv_timer_stop(&multi->timer_);
    int running_handles;
    int flags = 0;
    if (status < 0)                      flags = CURL_CSELECT_ERR;
    if (!status && events & UV_READABLE) flags |= CURL_CSELECT_IN;
    if (!status && events & UV_WRITABLE) flags |= CURL_CSELECT_OUT;
    uvcurl_log("curlm[%p] sockfd[%d] ", multi->curlm_, context->sockfd);
    curl_multi_socket_action(multi->curlm_, context->sockfd, flags, &running_handles);
    _check_multi_info(multi);   
}

static int _handle_socket(CURL *easy, curl_socket_t s, int action, void *userp, void *socketp) {
    uvcurl_log("easy[%p] socket[%d] action[%d] userp[%p] socketp[%p]",
        easy, s, action, userp, socketp);
    assert(NULL != easy);
    assert(NULL != userp);
    //assert(NULL != socketp);
    uvcurl_multi_t* multi = (uvcurl_multi_t*)userp;
    curl_context_t *curl_context;
    if (action == CURL_POLL_IN || action == CURL_POLL_OUT) {
        if (socketp) {
            curl_context = (curl_context_t*) socketp;
        }
        else {
            curl_context = _create_curl_context(multi, s);
            curl_multi_assign(multi->curlm_, s, (void *) curl_context);
            uvcurl_log("context[%p]", curl_context);
        }
    }

    switch (action) {
        case CURL_POLL_IN:
            uv_poll_start(&curl_context->pool_, UV_READABLE, _curl_perform);
            break;
        case CURL_POLL_OUT:
            uv_poll_start(&curl_context->pool_, UV_WRITABLE, _curl_perform);
            break;
        case CURL_POLL_REMOVE:
            if (socketp) {
                uv_poll_stop(&((curl_context_t*)socketp)->pool_);
                _destroy_curl_context((curl_context_t*) socketp);                
                curl_multi_assign(multi->curlm_, s, NULL);
            }
            break;
        default:
            abort();
    }

    return 0;
}
uvcurl_multi_t* uvcurl_multi_init(uv_loop_t* loop)
{
    assert(NULL != loop);
    uvcurl_malloc_obj(uvcurl_multi_t, multi);
    if(NULL == multi)
        return NULL;
    uvcurl_log("multi[%p]", multi);
    do{
        
        int res = uv_timer_init(loop, &(multi->timer_));
        if(0 != res) 
        {
            break;
        }
        multi->timer_.data = multi;
        multi->curlm_ = curl_multi_init();
        uvcurl_log("multi[%p] curlm[%p]", multi, multi->curlm_);
        if(NULL == multi->curlm_)
            break;
        
        curl_multi_setopt(multi->curlm_, CURLMOPT_SOCKETFUNCTION, _handle_socket);
        curl_multi_setopt(multi->curlm_, CURLMOPT_SOCKETDATA, multi);
        curl_multi_setopt(multi->curlm_, CURLMOPT_TIMERFUNCTION, _start_timeout);
        curl_multi_setopt(multi->curlm_, CURLMOPT_TIMERDATA, multi);
        return multi;
    }while(0);
    free(multi);
    return NULL;
}
static void _timer_close_cb(uv_handle_t* handle)
{
    uvcurl_log("");
}
void uvcurl_multi_cleanup(uvcurl_multi_t* multi)
{
    uvcurl_log("");
    if(NULL == multi)
        return;
    uv_close((uv_handle_t*)&multi->timer_, _timer_close_cb);
    curl_multi_cleanup(multi->curlm_);
}

CURLMcode uvcurl_multi_add_easy(uvcurl_multi_t* multi, CURL* easy, uvcurl_curl_done_cb_t done_cb, void* data) 
{
    
    uvcurl_log("multi[%p] curlm[%p] easy[%p] cb[%p]", multi, multi->curlm_, easy, done_cb);
    assert(NULL != multi);
    assert(NULL != easy);
    uvcurl_malloc_obj(uvcurl_curl_private_t, p);
    p->cb = done_cb;
    p->data = data;
    CURLcode code = curl_easy_setopt(easy, CURLOPT_PRIVATE, (void*)p);
    uvcurl_log("set CURLOPT_PRIVATE code[%d]", code);
    

    return curl_multi_add_handle(multi->curlm_, easy);
}

uv_loop_t* uvcurl_multi_get_loop(uvcurl_multi_t* multi)
{
    assert(NULL != multi);
    return multi->timer_.loop;
}
