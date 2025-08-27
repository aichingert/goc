#!/bin/sh

gcc src/parser.c src/tokenize.c src/arena.c src/goc.c -o goc
