
file(GLOB SRCS "*.c")
file(GLOB HDS "*.h")

add_library(uvcurl ${SRCS} ${HDS})