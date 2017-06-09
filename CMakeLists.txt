cmake_minimum_required(VERSION 2.6)

# main
project(main)
file(GLOB_RECURSE MAIN_SRC src/main.cpp src/*.h)

# clax
file(GLOB_RECURSE CLAX_SRC clax/src/*.c clax/src/*.h)
add_library(clax ${CLAX_SRC})

# freeglut
set(FREEGLUT d:/lib/freeglut)
include_directories(clax/src ${FREEGLUT}/include)
link_directories(${FREEGLUT}/lib)

list(APPEND PROJECT_LIBRARIES clax freeglut)

add_executable(${PROJECT_NAME} ${MAIN_SRC})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES})

