cmake_minimum_required(VERSION 3.13)

include(cmake/add-app.cmake)

add_subdirectory(tinyc)
add_subdirectory(lib)
add_subdirectory(api)
add_subdirectory(app)
