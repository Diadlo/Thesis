project(Thesis)
cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 11)

add_library(sqlite_db STATIC include/sqlite_db.h src/sqlite_db.cpp)
target_include_directories(sqlite_db PUBLIC include)
target_link_libraries(sqlite_db sqlite3)

add_executable(code_generator
        src/code_generator.cpp
        src/trietree.h)

target_link_libraries(code_generator sqlite_db)

add_executable(filler src/filler.cpp)
target_link_libraries(filler sqlite_db)

set(DB_FILE "${CMAKE_BINARY_DIR}/words.db" CACHE STRING "Path to generated database")
set(WORD_COUNT 10000 CACHE STRING "Count words to generate")

add_custom_command(
        OUTPUT
            ${DB_FILE}
        COMMAND
            cmake -E echo "Fill database" &&
            ./filler ${WORD_COUNT} ${DB_FILE}
        WORKING_DIRECTORY
            ${CMAKE_BINARY_DIR}
        DEPENDS
            filler)

add_custom_command(
        OUTPUT
            words_array.h words_trie_tree.h
        COMMAND
            cmake -E echo "Generating headers" &&
            ./code_generator ${DB_FILE} words_array.h words_trie_tree.h
        DEPENDS
            ${DB_FILE}
        WORKING_DIRECTORY
            ${CMAKE_BINARY_DIR}
        DEPENDS
            code_generator)

# Tests
function (add_test name)
    set(sources ${ARGN})
    add_executable(${name} ${sources} test/${name}.cpp)
    target_include_directories(${name} PUBLIC ${CMAKE_BINARY_DIR})
endfunction()

add_test(sql_searcher words_array.h words_trie_tree.h)
target_link_libraries(sql_searcher sqlite_db)
add_test(binary_searcher words_array.h)
add_test(trie_searcher   words_trie_tree.h)
