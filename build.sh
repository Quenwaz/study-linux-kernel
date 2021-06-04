#!/bin/bash
usage="usage: $0 -r[root path] -s[source path]\n\nargs:\n    -r,-ROOT_DIR-----makefile root path\n    -s,-SRC_DIR-----source file path\n\n"

ROOT_DIR=""
SRC_DIR=""
SRC_FILE=""

if [ -z "$1" ]
then
    echo -e $usage
else 
    GETOPT_ARGS=`getopt -o r:s:f: -al ROOT_DIR:,SRC_DIR:SRC_FILE: -- "$@"`
    eval set -- "$GETOPT_ARGS"
    #获取参数
    while [ -n "$1" ]
    do
        case "$1" in
                -r|--ROOT_DIR) ROOT_DIR=$2; shift 2;;
                -s|--SRC_DIR) SRC_DIR=$2; shift 2;;
                -f|--SRC_FILE) SRC_FILE=$2; shift 2;;
                --) break ;;
        esac
    done
fi


if [[ -d "$ROOT_DIR" && -d "$SRC_DIR" ]]
then
    # echo "ROOT_DIR=$ROOT_DIR, SRC_DIR=$SRC_DIR"
    make DIR=$ROOT_DIR SRCDIR=$SRC_DIR SRCFILE=$SRC_FILE
else
    echo -e $usage
fi