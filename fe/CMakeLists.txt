cmake_minimum_required(VERSION 2.10)
project(fronend)

add_library(fe parser.tab.c scanner.c)
target_compile_options(fe PUBLIC -g)