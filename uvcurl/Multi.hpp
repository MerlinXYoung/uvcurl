#pragma once
#include <uvcurl/uvcurl.h>

#if __cplusplus < 201103L
#error "Should use -std=c++11 option for compile"
#else
#include <memory>
#include <functional>
namespace cppuvcurl
{
class Multi
{
public:
    using native_multi_ptr =std::unique_ptr<uvcurl_multi_t, void(*)(uvcurl_multi_t*)>; 
    using done_cb_t = std::function<void(CURL*)> ;
    //using done_cb_map_t = std::unordered_map<CURL*, done_cb_t>;
    Multi(uv_loop_t* loop= uv_default_loop()):multi_(uvcurl_multi_init(loop), uvcurl_multi_cleanup)
    {

    }
    ~Multi()=default;
    void async_preform(CURL* easy, done_cb_t cb )
    {
        done_cb_t* pcb = new done_cb_t(cb);
        uvcurl_multi_add_easy(multi_.get(), easy, [](CURL* easy, void* data){

            std::unique_ptr<done_cb_t> guard((done_cb_t*)data);
            (*guard)(easy);
        }, pcb);
    }
private:
    native_multi_ptr multi_;
};
#endif
}