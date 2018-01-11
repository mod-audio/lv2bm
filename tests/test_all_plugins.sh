#!/bin/bash

source utils.sh || source tests/utils.sh

for p in `lv2ls`; do
    cprint green "Testing $p"
    $LV2BM $p

    if [ "$?" != "0" ]; then
        cprint red "Failled testing $p"
        break
    fi
done
