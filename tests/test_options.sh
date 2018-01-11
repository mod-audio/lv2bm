#!/bin/bash

source utils.sh || source tests/utils.sh

function run_test {
    # increment test number
    N=$((N+1))

    # print and run lv2bm
    cprint blue "Test #$N: $LV2BM $@"
    $LV2BM $@

    # print test status
    if [ $? -eq 0 ]; then
        cprint green "Test finished without error"
    else
        cprint red "Test returned error"
    fi
    echo
}

PLUGIN="http://lv2plug.in/plugins/eg-amp"

run_test
run_test $PLUGIN
run_test $PLUGIN --rate 44100
run_test $PLUGIN --frame-size 256
run_test $PLUGIN --n-frames 750
run_test $PLUGIN --full-test
run_test $PLUGIN --input sweep
run_test $PLUGIN --output /tmp/sample.flac
run_test $PLUGIN --version
run_test $PLUGIN --help
run_test $PLUGIN -r 44100 -f 256 -n 750 -i sawtooth -o /tmp/sample.flac
