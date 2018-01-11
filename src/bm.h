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

#ifndef BENCH_H
#define BENCH_H

#include <sndfile.hh>

#include "plugin.h"
#include "input_gen.h"

using namespace std;

struct bench_info_t {
    double total, average, jack_load;
    std::map<uint32_t,port_data_t> plugin_preset;
};

class Bench {
private:
    void slice_parameters(void);
    std::vector<uint32_t> params;
    Generator *generator;
    SndfileHandle sndfile;

public:
    Bench(const char* uri, uint32_t sample_rate, uint32_t frame_size, uint32_t n_frames,
          const char *signal, const char *output);
    ~Bench();

    void run_and_calc(bench_info_t* var, bool save_output=false);
    void process(void);
    void print(void);
    void test_points(uint32_t depth, vector<uint32_t> & params, vector<uint32_t> & n_points);

    uint32_t sample_rate, frame_size, n_frames;
    Plugin *plugin;

    bench_info_t min, max, def, smaller, bigger;

    bool full_test;

    uint32_t n_points_default;
    std::vector<uint32_t> n_points_to_test;
};

#endif
