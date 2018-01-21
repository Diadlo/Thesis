project(Thesis)

set(PROJECT_NAME thesis)
add_executable(${PROJECT_NAME}
        src/main.cpp
        )

target_link_libraries(${PROJECT_NAME} sqlite3)
