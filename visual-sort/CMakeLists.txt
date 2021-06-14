cmake_minimum_required(VERSION 2.8)

project(visual-sort)

set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -mwindows")

include_directories(imgui)
aux_source_directory(imgui IMGUI_SRCS)

set(VIEWS view.c main-view.c about-view.c)
set(SORTERS sorter.c bubble-sorter.c quick-sorter.c)
set(GUIPARTS menubar.c)
set(SRCS main.c utility.c ${VIEWS} ${SORTERS} ${GUIPARTS})

add_executable(visual-sort ${SRCS} ${IMGUI_SRCS})
