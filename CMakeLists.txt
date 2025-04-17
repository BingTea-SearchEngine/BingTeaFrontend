cmake_minimum_required(VERSION 3.31)

set(THIS BingTeaFrontend)

project(${THIS} CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

enable_testing()

include(FetchContent)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.12.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

FetchContent_Declare(
    Gateway
    GIT_REPOSITORY https://github.com/BingTea-SearchEngine/Gateway.git
    GIT_TAG main
)
FetchContent_MakeAvailable(Gateway)

FetchContent_Declare(
    ExpressionParser
    GIT_REPOSITORY https://github.com/BingTea-SearchEngine/ExpressionParser.git
    GIT_TAG main
)
FetchContent_MakeAvailable(ExpressionParser)

FetchContent_Declare(
    IndexInterface
    GIT_REPOSITORY https://github.com/BingTea-SearchEngine/IndexInterface.git
    GIT_TAG main
)
FetchContent_MakeAvailable(IndexInterface)

set(GATEWAY_SOURCE_DIR ${gateway_SOURCE_DIR})
set(GATEWAY_INCLUDE_DIR "${gateway_SOURCE_DIR}/lib")
message(STATUS "Gateway project source directory: ${GATEWAY_SOURCE_DIR}")
message(STATUS "Gateway include directory: ${GATEWAY_INCLUDE_DIR}")

set(EXPRESSIONPARSER_SOURCE_DIR ${expressionparser_SOURCE_DIR})
set(EXPRESSIONPARSER_INCLUDE_DIR "${expressionparser_SOURCE_DIR}/lib")

set(INDEXINTERFACE_SOURCE_DIR ${indexinterface_SOURCE_DIR})
set(INDEXINTERFACE_INCLUDE_DIR "${indexinterface_SOURCE_DIR}/lib")

set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

add_executable(${THIS} src/BingTeaFrontend.cpp src/Utils.cpp src/SearchEngine.cpp src/RenderHTML.cpp)
target_link_libraries(${THIS} PUBLIC spdlog::spdlog argparse GatewayClient IndexInterface)
# target_include_directories(${THIS} PU)

# Specify build type
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
