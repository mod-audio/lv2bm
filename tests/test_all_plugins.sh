#!/bin/bash

LV2BM=./lv2bm
if [ "$(basename `pwd`)" = "tests" ]; then    
    LV2BM=../lv2bm
fi

function cprint {
    green="\e[0;32m"
    red="\e[1;31m"
    yellow="\e[1;33m"

    c=`echo $1 | awk '{print tolower($0)}'`
    eval "color=\$$c"
    echo -e "$color$2\e[0m"
}

for p in `lv2ls`; do
    cprint green "Testing $p"
    $LV2BM $p

    if [ "$?" != "0" ]; then
        cprint red "Failled testing $p"
        break
    fi
done
