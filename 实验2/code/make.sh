#!/bin/sh
bison -d cmm.y
flex cmm.l
bison -d cmm.y
gcc cmm_analyser.c cmm.tab.c -ly -lfl -o cmm
