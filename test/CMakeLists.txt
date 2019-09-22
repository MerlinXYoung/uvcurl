
file(GLOB SRCS "*.c")

add_executable(uvget ${SRCS})
target_link_libraries(uvget uvcurl libuv curl)