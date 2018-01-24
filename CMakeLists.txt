project(Thesis)
cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME thesis)
set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
        src/main.cpp
        )

target_link_libraries(${PROJECT_NAME} sqlite3)
