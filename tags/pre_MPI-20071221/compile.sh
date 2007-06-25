#!/bin/bash
BASE_DIR=$HOME
MAKE=/usr/bin/make
cd MOGA/include/moea
$MAKE -f makefile
cd ../../../
$MAKE


