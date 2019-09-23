
file(GLOB SRCS "*.c" "*.cpp")

foreach(var ${SRCS})
    string(REGEX REPLACE ".*/" "" var ${var})

    set(target ${var}.t)
    add_executable(${target} ${var})
    target_link_libraries(${target} uvcurl libuv curl ssl crypto)
endforeach(var)

