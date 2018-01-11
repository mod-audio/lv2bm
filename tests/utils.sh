#!/bin/bash

LV2BM=./lv2bm
if [ "$(basename `pwd`)" = "tests" ]; then
    LV2BM=../lv2bm
fi

function cprint {
    green="\e[0;32m"
    red="\e[1;31m"
    yellow="\e[1;33m"
    blue="\e[1;34m"

    c=`echo $1 | awk '{print tolower($0)}'`
    eval "color=\$$c"
    shift
    echo -e "$color$@\e[0m"
}
