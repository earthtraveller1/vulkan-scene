# This file is responsible for building and configuring all of the tests
# in this project. It should be obvious since this file is called "tests"

add_executable(matrix-multiplication matrix-multiplication.cpp)
add_test(NAME matrix-multiplication COMMAND matrix-multiplication)