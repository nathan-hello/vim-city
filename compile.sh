#!/bin/sh
set -x
cc src/main.c -o vimcity -I./external/raygui-4.0/src -lraylib -lm
