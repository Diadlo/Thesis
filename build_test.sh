#!/bin/sh

g++ ../res/sql_searcher.cpp -lsqlite3 -std=c++14 -I. -o sql_searcher
g++ ../res/binary_searcher.cpp -std=c++14 -I. -o binary_searcher
