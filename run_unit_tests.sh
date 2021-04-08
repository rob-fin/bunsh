#!/usr/bin/env bash

readonly UNIT_BIN="unit_test_bin"
gcc -o "$UNIT_BIN" test/cunit_runner.c test/parser_suite.c src/parser.c src/buffers.c -lcunit -I.
./"$UNIT_BIN" 2> /dev/null
rm "$UNIT_BIN"