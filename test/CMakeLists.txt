


link_directories(${PROJECT_BINARY_DIR}/uvcurl)
add_executable(uvget "main.c")
target_link_libraries(uvget uvcurl libuv curl)
add_dependencies(uvget uvcurl)

add_executable(uvgetpp "main.cpp")
target_link_libraries(uvgetpp uvcurl libuv curl)
add_dependencies(uvgetpp uvcurl)