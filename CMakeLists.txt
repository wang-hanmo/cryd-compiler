cmake_minimum_required (VERSION 3.13)


project(compiler)
set(CMAKE_CXX_STANDARD 17)
# SET(CMAKE_CXX_COMPILER "D:/msys64/mingw64/bin/clang++")
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
add_definitions(-DYY_NO_UNISTD_H)

# set(CMAKE_CXX_FLAGS   "-g") 
# SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 设置FLAGS
#[[
if (WIN32)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
     set(CMAKE_CXX_FLAGS"${CMAKE_CXX_FLAGS} -Wl,--stack,8388608 -fpermissive")
  elseif(MSVC)
    # add options for Visual C/C++ Compiler here
    set(CMAKE_CXX_FLAGS"${CMAKE_CXX_FLAGS} /F 8388608")
  endif()
endif()]]
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CL_64)
       message(STATUS "building clang x64")
       add_definitions(-DBUILD_CLANG_X64)
    else()
       message(STATUS "building clang x86")
       add_definitions(-DBUILD_CLANG_X86)
    endif()
endif()
# 定义变量
set(PROJ_DIR ${CMAKE_CURRENT_SOURCE_DIR})
# set(INCLUDE_DIR ${PROJ_DIR}/dependencies/include)
set(SOURCE_DIR ${PROJ_DIR}/src)
set(GEN_SOURCE_DIR ${PROJ_DIR}/gen_src)
set(BUILD_DIR ${PROJ_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${BUILD_DIR})

# 添加头文件目录
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR}/back_end)
include_directories(${SOURCE_DIR}/front_end)
include_directories(${SOURCE_DIR}/back_end)
include_directories(${SOURCE_DIR}/optimizer)
include_directories(${SOURCE_DIR}/global)
include_directories(${SOURCE_DIR}/ir)
include_directories(${GEN_SOURCE_DIR})

# 通过bison生成语法分析源代码
add_custom_command(
    PRE_BUILD
    DEPENDS ${SOURCE_DIR}/front_end/parser.y
    COMMAND bison -d ${SOURCE_DIR}/front_end/parser.y -o ${GEN_SOURCE_DIR}/parser.yy.cpp
    OUTPUT ${GEN_SOURCE_DIR}/parser.yy.cpp ${GEN_SOURCE_DIR}/parser.yy.hpp
    WORKING_DIRECTORY ${PROJ_DIR}
)
# 通过flex产生词法分析源代码
add_custom_command(
    PRE_BUILD
    DEPENDS ${SOURCE_DIR}/front_end/lexer.l ${GEN_SOURCE_DIR}/parser.yy.hpp
    COMMAND flex -o ${GEN_SOURCE_DIR}/lexer.ll.cpp --header-file=${GEN_SOURCE_DIR}/lexer.ll.h ${SOURCE_DIR}/front_end/lexer.l
    OUTPUT ${GEN_SOURCE_DIR}/lexer.ll.cpp ${GEN_SOURCE_DIR}/lexer.ll.h
    WORKING_DIRECTORY ${PROJ_DIR}
)

# 添加目标文件
add_executable(compiler)
target_sources(compiler PRIVATE
    ${GEN_SOURCE_DIR}/lexer.ll.cpp
    ${GEN_SOURCE_DIR}/parser.yy.cpp
)
# 添加子目录
add_subdirectory(src)


