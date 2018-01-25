#!/bin/sh

function build {
    name="$1"
    echo "Build ${name}"
    g++ ../res/${name}.cpp -lsqlite3 -std=c++14 -I. -o ${name}
}

build sql_searcher
build binary_searcher
build trie_searcher
