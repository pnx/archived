#!/bin/bash
#
# Helper for touching a number of files in a directory.

if [ $# -lt 1 ]; then
    echo "usage: $0 <dir> [ <nfiles> ]"
    exit 1
fi

if [ ! -d $1 ]; then
    echo "ERROR: '${1}' is not a directory"
    exit 1
fi

NFILES=1024

if [ $# -gt 1 ]; then
    if [ ! -z "${2##[0-9]*}" ]; then
        echo "ERROR: '${2}' is not a positive integer"
        exit 1
    fi
    NFILES=$2
fi

for i in `seq 0 ${NFILES}`; do
    touch "${1}/file${i}"
done
