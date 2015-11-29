
#include <iostream>
#include "bm.h"
#include "info.h"

#include <stdlib.h>
#include <getopt.h>

using namespace std;

// The version is extracted from git history
extern const char version[];

int main(int argc, char *argv[])
{
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"rate", required_argument, 0, 'r'},
        {"frame-size", required_argument, 0, 'f'},
        {"n-frames", required_argument, 0, 'n'},
        {"full-test", no_argument, 0, 't'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };

    // default options values
    unsigned int rate = 44100, frame_size = 256, n_frames = 64;
    bool full_test = false;

    // parse the command line options
    int opt, option_index;
    while ((opt = getopt_long(argc, argv, "hr:f:n:tV", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
            case 'r':
                rate = atoi(optarg);
                break;

            case 'f':
                frame_size = atoi(optarg);
                break;

            case 'n':
                n_frames = atoi(optarg);
                break;

            case 't':
                full_test = true;
                break;

            case 'V':
                printf(
                    "%s version: %s\n"
                    "source code: https://github.com/portalmod/lv2bm\n",
                argv[0], version);

                exit(EXIT_SUCCESS);
                break;

            default:
            case 'h':
                cout << "Usage: " << argv[0] << " [OPTIONS] URIs" << endl;
                cout << "  -r, --rate            Defines the sample rate. Default: " << rate << endl << endl;
                cout << "  -f, --frame-size      Defines the frame size. Equivalent to option -p of the JACK." << endl;
                cout << "                        Default: " << frame_size << endl << endl;
                cout << "  -n, --n-frames        Defines the number of frames. How many times the run " << endl;
                cout << "                        function of the plugin will execute. Default: " << n_frames << endl << endl;
                cout << "  --full-test           The full test will run the plugins using differents" << endl;
                cout << "                        controls values combinations. This test can take a long" << endl;
                cout << "  -V, --version         print program version and exit" << endl;
                cout << "  -h, --help            print this help and exit" << endl;
                cout << "                        time, depending on the amount of controls the plugin has." << endl << endl;

                if (opt != 'h') exit(EXIT_FAILURE);
                else exit(EXIT_SUCCESS);
        }
    }

    // run the benchmark
    for (int i = 0; i < (argc - optind); i++)
    {
        try
        {
            Bench bench = Bench(argv[optind+i], rate, frame_size, n_frames);
            bench.full_test = full_test;
            bench.process();
            bench.print();
        }
        catch(...) {}
    }

    return 0;
}

// TODO: dump plugin test information when receive segfault
// TODO: do something, even a print, when run lv2bm without options
// TODO: generate MIDI input for plugins that need it
// TODO: choose differents audio inputs as in http://carlh.net/plugins/torture.php
// TODO: allow to select the output unit
// TODO(?): create option to print the controls values for max, min, def, best, worst