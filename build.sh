#!/usr/bin/env bash

bin=./bin
tests=./tests
bin=./bin
build=./build
cflags="-Wall -Wextra -ggdb"
raylib=./lib/raylib-5.0_linux_amd64

set -xe

for f in $bin/*.c; do
    cc $cflags -o $build/$(basename $f .c) $f $raylib/lib/libraylib.a -I$raylib/include -lm
done

for f in $tests/*.c; do
    cc $cflags -o $build/$(basename $f .c) $f $raylib/lib/libraylib.a -I$raylib/include -lm
done

$build/main
