cmake_minimum_required(VERSION 3.19)
project(tetris C)

set(CMAKE_C_STANDARD 99)

set(SRC main.c tetris.c)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

find_library(MENU_LIBRARY menu)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARIES} ${MENU_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
target_link_libraries(${PROJECT_NAME} m)
