cmake_minimum_required(VERSION 3.20)
project(lan_file_share LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-38-2
)
FetchContent_MakeAvailable(asio)

add_executable(lan_file_share src/main.cpp)

target_include_directories(lan_file_share PRIVATE ${asio_SOURCE_DIR}/include)
target_compile_definitions(lan_file_share PRIVATE ASIO_STANDALONE)

if(WIN32)
    target_link_libraries(lan_file_share PRIVATE ws2_32 mswsock)
endif()