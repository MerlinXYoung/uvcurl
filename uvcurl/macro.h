#pragma once

#define uvcurl_malloc(T) \
(T*) malloc(sizeof(T));

#define uvcurl_malloc_obj(T, obj) \
T* obj = uvcurl_malloc(T);

