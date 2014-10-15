lv2bm
=====


About
-----

lv2bm is a benchmark tool to LV2 plugins, it was inspired on lv2bench from lilv.
It features:

- allows to select which URIs to test
- uses minimum, maximum and default control values to run the plugins
- uses several controls combinations in the full test mode
- the output shows the JACK load percent
- can be used along with valgrind to detect plugin memory issues


Build
-----

lv2bm uses a simple Makefile to build the source code.
The steps to build and install are:

    make
    make install

The default instalation path is /usr/local/bin, this can be modified passing
the variable PREFIX to make install:

    make install PREFIX=/usr

Or you can use the INSTALL_PATH if don't want the bin subdirectory.

Dependencies:

    glib            >= 2.0
    liblilv         >= 0.14.2


Usage
-----

**lv2bm [OPTIONS] URIs**

Valid options:

    -r, --rate            Defines the sample rate. Default: 44100

    -f, --frame-size      Defines the frame size. Equivalent to option -p on JACK.
                          Default: 256

    -n, --n-frames        Defines the number of frames. How many times the run
                          function of the plugin will execute. Default: 64

    --full-test           The full test will run the plugins using differents
                          controls values combinations. This test can take a long
                          time, depending on the amount of controls the plugin has.


Full test
---------

The full test will explore multiples control values combinations. Plugins with enumeration,
scale points and toggle will have all points tested. The other type of controls will be
sliced resulting in 4 points to be tested. Along with valgrind this evaluation is useful to
stress the plugin trying find out segfaults or memory issues (leak, invalid read/write, ...).

Following is a real case of use. A time ago the CabinetIV plugin, of the CAPS collection, owned
a bug that was causing a segfault just when the model parameter was configured to value "Sixty-two".
Moreover, the segfault just happened in a specific machine (little RAM memory). Several hours would be
saved with this tool in hands.

If you interested in reproduce this situation to familiarize yourself with the procedure, you can reach
our caps-lv2 repository and checkout to the commit just before the bug resolution (383a16c2), build it,
install to your LV2 path and try:

    valgrind --leak-check=full --show-reachable=no lv2bm http://portalmod.com/plugins/caps/CabinetIV

The valgrind output should be something like that:

    ==13521== Memcheck, a memory error detector
    ==13521== Copyright (C) 2002-2013, and GNU GPL'd, by Julian Seward et al.
    ==13521== Using Valgrind-3.10.0 and LibVEX; rerun with -h for copyright info
    ==13521== Command: lv2bm --full-test http://portalmod.com/plugins/caps/CabinetIV
    ==13521== Parent PID: 2668
    ==13521==
    ==13521== Invalid read of size 16
    ==13521==    at 0x7092EE9: CabinetIV::switch_model(int) (in /home/ricardo/.lv2/caps.lv2/caps.so)
    ==13521==    by 0x7099E85: void CabinetIV::cycle<&(store_func(float*, unsigned int, float, float)), DSP::NoOversampler, 1>(unsigned int, DSP::NoOversampler&) (in /home/ricardo/.lv2/caps.lv2/caps.so)
    ==13521==    by 0x709E55E: Descriptor<CabinetIV>::_run_lv2(void*, unsigned int) (in /home/ricardo/.lv2/caps.lv2/caps.so)
    ==13521==    by 0x40AA97: lilv_instance_run (lilv.h:1559)
    ==13521==    by 0x40D12A: Lilv::Instance::run(unsigned int) (lilvmm.hpp:293)
    ==13521==    by 0x40C976: Plugin::run(unsigned int) (plugin.cpp:383)
    ==13521==    by 0x4032CE: Bench::run_and_calc(bench_info_t*) (bm.cpp:132)
    ==13521==    by 0x4035CB: Bench::process() (bm.cpp:169)
    ==13521==    by 0x41048D: main (main.cpp:69)
    ==13521==  Address 0x7356ff4 is not stack'd, malloc'd or (recently) free'd
    ...
    ==13521== HEAP SUMMARY:
    ==13521==     in use at exit: 1,500 bytes in 5 blocks
    ==13521==   total heap usage: 29,558 allocs, 29,553 frees, 2,017,352 bytes allocated
    ==13521==
    ==13521== LEAK SUMMARY:
    ==13521==    definitely lost: 0 bytes in 0 blocks
    ==13521==    indirectly lost: 0 bytes in 0 blocks
    ==13521==      possibly lost: 0 bytes in 0 blocks
    ==13521==    still reachable: 1,500 bytes in 5 blocks
    ==13521==         suppressed: 0 bytes in 0 blocks
    ==13521== Reachable blocks (those to which a pointer was found) are not shown.
    ==13521== To see them, rerun with: --leak-check=full --show-leak-kinds=all
    ==13521==
    ==13521== For counts of detected and suppressed errors, rerun with: -v
    ==13521== ERROR SUMMARY: 175 errors from 8 contexts (suppressed: 0 from 0)

Now, go to the next commit (or master) and build, install and run the valgrind command again. You'll
note that the output shows 0 errors.

    ==13763== Memcheck, a memory error detector
    ==13763== Copyright (C) 2002-2013, and GNU GPL'd, by Julian Seward et al.
    ==13763== Using Valgrind-3.10.0 and LibVEX; rerun with -h for copyright info
    ==13763== Command: lv2bm --full-test http://portalmod.com/plugins/caps/CabinetIV
    ==13763== Parent PID: 2668
    ==13763==
    ==13763==
    ==13763== HEAP SUMMARY:
    ==13763==     in use at exit: 1,500 bytes in 5 blocks
    ==13763==   total heap usage: 29,566 allocs, 29,561 frees, 2,018,804 bytes allocated
    ==13763==
    ==13763== LEAK SUMMARY:
    ==13763==    definitely lost: 0 bytes in 0 blocks
    ==13763==    indirectly lost: 0 bytes in 0 blocks
    ==13763==      possibly lost: 0 bytes in 0 blocks
    ==13763==    still reachable: 1,500 bytes in 5 blocks
    ==13763==         suppressed: 0 bytes in 0 blocks
    ==13763== Reachable blocks (those to which a pointer was found) are not shown.
    ==13763== To see them, rerun with: --leak-check=full --show-leak-kinds=all
    ==13763==
    ==13763== For counts of detected and suppressed errors, rerun with: -v
    ==13763== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)


Examples of use
---------------

**Basic test**

    $ lv2bm http://lv2plug.in/plugins/eg-amp
    Plugin: http://lv2plug.in/plugins/eg-amp
        TestName  TotalTime(s)   AvrTime(s)  JackLoad(%)
       MinValues    0.00002221   0.00000035     0.011954
       DefValues    0.00002237   0.00000035     0.012040
       MaxValues    0.00003317   0.00000052     0.017855

**Full test**

    $ lv2bm --full-test http://lv2plug.in/plugins/eg-amp
    Plugin: http://lv2plug.in/plugins/eg-amp
        TestName  TotalTime(s)   AvrTime(s)  JackLoad(%)
       MinValues    0.00002223   0.00000035     0.011965
       DefValues    0.00002236   0.00000035     0.012038
       MaxValues    0.00003312   0.00000052     0.017828
      BestResult    0.00002204   0.00000034     0.011867
     WorstResult    0.00003312   0.00000052     0.017828

**Multiple URIs**

    $ lv2bm `lv2ls | grep swh-plugins`
    Plugin: http://plugin.org.uk/swh-plugins/alaw
        TestName  TotalTime(s)   AvrTime(s)  JackLoad(%)
       MinValues    0.00157707   0.00002464     0.848985
       DefValues    0.00157577   0.00002462     0.848287
       MaxValues    0.00161628   0.00002525     0.870089

    Plugin: http://plugin.org.uk/swh-plugins/allpass_c
        TestName  TotalTime(s)   AvrTime(s)  JackLoad(%)
       MinValues    0.00109872   0.00001717     0.591475
       DefValues    0.00048767   0.00000762     0.262529
       MaxValues    0.00073013   0.00001141     0.393050

    Plugin: http://plugin.org.uk/swh-plugins/allpass_l
        TestName  TotalTime(s)   AvrTime(s)  JackLoad(%)
       MinValues    0.00031596   0.00000494     0.170088
       DefValues    0.00019513   0.00000305     0.105044
       MaxValues    0.00011171   0.00000175     0.060138
    ...
