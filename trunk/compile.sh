#!/bin/bash
export PATH="/opt/share/mvapich2/bin:$PATH"
MAKE=/usr/bin/make
$MAKE -C spea2/src && $MAKE
