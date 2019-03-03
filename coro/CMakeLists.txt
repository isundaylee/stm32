add_library(coro
    src/coro/Throttler.cpp
    )

target_include_directories(coro
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
