project(Thesis)
cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME thesis)
set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/trietree.h)

target_link_libraries(${PROJECT_NAME} sqlite3)

add_executable(filler src/filler.cpp)
target_link_libraries(filler sqlite3)

add_custom_target(fill_db
        COMMAND
            cmake -E echo "Fill database" &&
            ./filler ${CMAKE_SOURCE_DIR}/words.txt ${CMAKE_BINARY_DIR}/words.db
        WORKING_DIRECTORY
            ${CMAKE_BINARY_DIR}
        DEPENDS
            filler)

add_custom_target(generate
        COMMAND
            cmake -E echo "Generating headers" &&
            ./${PROJECT_NAME} words.db words_array.h words_trie_tree.h
        WORKING_DIRECTORY
            ${CMAKE_BINARY_DIR}
        DEPENDS
            ${PROJECT_NAME}
            fill_db)

# Tests
function (add_test name)
    add_executable(${name} test/${name}.cpp)
    target_link_libraries(${name} sqlite3)
    add_dependencies(${name} generate)
    target_include_directories(${name} PUBLIC ${CMAKE_BINARY_DIR})
endfunction()

add_test(sql_searcher)
add_test(binary_searcher)
add_test(trie_searcher)
