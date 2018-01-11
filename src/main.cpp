/*
 * Copyright (C) 2017 Ricardo Crudo <ricardo.crudo@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };

    // default options values
    unsigned int rate = 48000, frame_size = 128, n_frames = rate / frame_size;
    bool full_test = false;

    const char *default_input_signal = "sine";
    const char *input_signal = default_input_signal;
    const char *output = 0;

    bool no_arguments_passed = false;
    if (argc < 2)
        no_arguments_passed = true;

    // parse the command line options
    int opt, option_index;
    while ((opt = getopt_long(argc, argv, "hr:f:n:ti:o:V", long_options, &option_index)) != -1 ||
           no_arguments_passed) {
        switch (opt) {
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

        case 'i':
            input_signal = optarg;
            break;

        case 'o':
            output = optarg;
            break;

        case 'V':
            cout << argv[0] << " version " << version << endl;
            cout << "source code: https://github.com/moddevices/lv2bm" << endl;

            exit(EXIT_SUCCESS);
            break;

        default:
        case 'h':
            cout << "Usage: " << argv[0] << " [OPTIONS] URIs" << endl;
            cout << "  -r, --rate            Defines the sample rate. Default: " << rate << endl << endl;
            cout << "  -f, --frame-size      Defines the frame size. Equivalent to option -p of the JACK." << endl;
            cout << "                        Default: " << frame_size << endl << endl;
            cout << "  -n, --n-frames        Defines the number of frames, i.e. how many times the 'run'" << endl;
            cout << "                        function of the plugin executes. Default: " << n_frames << endl << endl;
            cout << "  --full-test           Run the plugins using differents controls values combinations." << endl;
            cout << "                        This test might take a long time depending on the amount of" << endl;
            cout << "                        controls the plugin has." << endl << endl;
            cout << "  -i, --input           Select the signal to apply to the audio inputs of the plugin." << endl;
            cout << "                        Valid inputs:" << endl;
            cout << "                          sine:       Sine wave 1kHz" << endl;
            cout << "                          square:     Square wave 1kHz" << endl;
            cout << "                          sweep:      Sine sweep 20Hz to 20KHz in n-frames" << endl;
            cout << "                          white:      Uniform white noise" << endl;
            cout << "                          gwhite:     Gaussian shaped white noise" << endl;
            cout << "                          pink:       Pink noise" << endl;
            cout << "                          impulse:    1 sample spike 100Hz, 0dBFS" << endl;
            cout << "                          sawtooth:   Sawtooth wave 100Hz" << endl;
            cout << "                          triangle:   Triangle wave 100Hz" << endl << endl;
            cout << "  -o, --output FILE     Write the plugin outputs to a FLAC file. The generated file" << endl;
            cout << "                        contains the audio using the default values of controls." << endl << endl;
            cout << "  -V, --version         Print program version and exit." << endl << endl;
            cout << "  -h, --help            Print this help message and exit." << endl;

            if (opt != 'h') exit(EXIT_FAILURE);
            else exit(EXIT_SUCCESS);
        }
    }

    // run the benchmark
    for (int i = 0; i < (argc - optind); i++) {
        try {
            Bench bench = Bench(argv[optind+i], rate, frame_size, n_frames, input_signal, output);
            bench.full_test = full_test;
            bench.process();
            bench.print();
        }
        catch(exception& e) {
            cout << e.what() << endl;
        }
    }

    return 0;
}

// TODO: dump plugin test information when receive segfault
// TODO: generate MIDI input for plugins that need it
// TODO: allow to select the output unit
// TODO(?): create option to print the controls values for max, min, def, best, worst
// https://github.com/x42/midigen.lv2
