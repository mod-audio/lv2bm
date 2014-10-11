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
    glibmm          >= 2.4
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


Examples
--------

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
