cmake_minimum_required(VERSION  2.8)

include_directories(../include)

AUX_SOURCE_DIRECTORY(. SRC_LIST)



# add_library(test ${src})  

# set(THREADS_PREFER_PTHREAD_FLAG ON)
# find_package(Threads REQUIRED)
# target_link_libraries(test PUBLIC Threads::Threads)

ADD_EXECUTABLE(main ${SRC_LIST})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
target_link_libraries(main ${CMAKE_THREAD_LIBS_INIT})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)