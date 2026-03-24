cmake_minimum_required(VERSION 3.10)
project(FlowerShop)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SQLITE3_ROOT "D:/DB/SQLite")

set(SQLITE3_INCLUDE_DIR "${SQLITE3_ROOT}/include")
set(SQLITE3_LIBRARY "${SQLITE3_ROOT}/lib/sqlite3.lib")
set(SQLITE3_DLL "${SQLITE3_ROOT}/bin/sqlite3.dll")

if(NOT EXISTS ${SQLITE3_INCLUDE_DIR}/sqlite3.h)
    message(FATAL_ERROR "sqlite3.h not found")
endif()

if(NOT EXISTS ${SQLITE3_LIBRARY})
    message(FATAL_ERROR "sqlite3.lib not found")
endif()

include_directories(${SQLITE3_INCLUDE_DIR})

set(SOURCES
    src/main.cpp
    src/database.cpp
    src/auth.cpp
    src/flower_shop.cpp
    src/utils.cpp
)

add_executable(flower_shop ${SOURCES})
target_link_libraries(flower_shop ${SQLITE3_LIBRARY})

add_custom_command(TARGET flower_shop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/sql
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sql
    COMMENT "Copying SQL files..."
)

add_custom_command(TARGET flower_shop POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${SQLITE3_DLL}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Copying sqlite3.dll..."
)