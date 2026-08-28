cmake_minimum_required(VERSION 3.20)
project(solan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-38-2
)
FetchContent_MakeAvailable(asio)

FetchContent_Declare(
    picosha2
    GIT_REPOSITORY https://github.com/okdshin/PicoSHA2.git
    GIT_TAG master
)
FetchContent_MakeAvailable(picosha2)

target_include_directories(SOLAN PRIVATE ${picosha2_SOURCE_DIR})

add_executable(solan src/main.cpp src/sender.cpp src/receiver.cpp)

target_include_directories(solan PRIVATE ${asio_SOURCE_DIR}/include)
target_compile_definitions(solan PRIVATE ASIO_STANDALONE)

if(WIN32)
    target_link_libraries(solan PRIVATE ws2_32 mswsock)
endif()