cmake_minimum_required(VERSION 3.19)
project(Flies LANGUAGES CXX)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

add_library(${PROJECT_NAME} STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(${PROJECT_NAME}
    PUBLIC
    cxx_std_20
)

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES ${SOURCE_FILES} ${HEADER_FILES}
)