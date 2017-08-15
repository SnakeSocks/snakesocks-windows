cmake_minimum_required(VERSION 3.8)

#set(CMAKE_C_COMPILER "C:\\\\\\\\MinGW\\\\\\\\bin\\\\\\\\fake_gcc.sh")
#set(CMAKE_CXX_COMPILER "C:\\\\\\\\MinGW\\\\\\\\bin\\\\\\\\fake_g++.sh")
set(BOOST_ROOT "C:\\\\bash_home\\\\boost_1_64_0")
set(BOOST_LIBRARYDIR "C:\\\\bash_home\\\\boost_1_64_0\\\\bin.v2\\\\libs")

project(SnakeSocksCli)
# makefile for windows

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

add_definitions(-DCOMPILE_SS_MAIN_CLIENT)
add_definitions(-D___NO_THREAD_POOL)
add_definitions(-D_WIN32_WINNT=0x501)
#add_definitions(-DADAPT_WIN32_DLIB)

set(CMAKE_MAKE_PROGRAM make)
set(CMAKE_CXX_FLAGS_DEBUG "-g -DMALLOC_CHECK_=2")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_VERBOSE_MAKEFILE ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories(./src)
include_directories(./src/lib)
include_directories(./dlfcn-win32)
include_directories(${BOOST_ROOT})
include_directories(/c/MinGW/include)

set(Boost_USE_STATIC_LIBS ON)
find_package(Threads REQUIRED)
#find_package(Boost REQUIRED system)

set(CLIENT_SRC src/client.cc src/socks5.cc src/tunnel.cc src/module.cc src/lib/sio.cc src/lib/NetLib.cc src/lib/fstr.cc)
add_executable(skcli ${CLIENT_SRC})
target_link_libraries(skcli Threads::Threads)
#target_link_libraries(skcli Boost::system)
target_link_libraries(skcli "C:\\bash_home\\boost_1_64_0\\bin.v2\\libs\\system\\build\\gcc-mingw-6.3.0\\release\\link-static\\threading-multi\\libboost_system-mgw63-mt-1_64.a")
target_link_libraries(skcli ./dlfcn-win32/libdl.a)
target_link_libraries(skcli Ws2_32.lib)

#if(FOR_M32)
#    set_target_properties(skcli PROPERTIES COMPILE_FLAGS -m32 LINK_FLAGS -m32 )
#endif(FOR_M32)
