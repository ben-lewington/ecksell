#!/usr/bin/env bash

cc=cc
build=./build
cflags="-Wall -Wextra -ggdb -pedantic"
raylib=./lib/raylib
raygui=./lib/raygui

set -xe

pushd $raylib; zig build; popd
$cc $cflags -o $build/raygui.o -DRAYGUI_IMPLEMENTATION $raygui/src/raygui.h -I$raylib/zig-out/include
for f in $raygui/examples/**/*.c; do
    $cc $cflags -o $build/rg_examples/$(basename $f .c) $f $raylib/zig-out/lib/libraylib.a -I$raylib/zig-out/include -I$raylib/src/external -I$raygui/src -lm || true
done
