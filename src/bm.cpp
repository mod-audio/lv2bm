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
#include <iomanip>
#include <cstdlib>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "bm.h"

using namespace std;

static inline double bench_elapsed_s(const struct timespec* start, const struct timespec* end)
{
    return ((end->tv_sec - start->tv_sec)
            + ((end->tv_nsec - start->tv_nsec) * 0.000000001));
}

static inline struct timespec bench_start()
{
    struct timespec start_t;
    clock_gettime(CLOCK_REALTIME, &start_t);
    return start_t;
}

static inline double bench_end(const struct timespec* start_t)
{
    struct timespec end_t;
    clock_gettime(CLOCK_REALTIME, &end_t);
    return bench_elapsed_s(start_t, &end_t);
}

Bench::Bench(const char* uri, uint32_t sample_rate, uint32_t frame_size, uint32_t n_frames)
{
    this->sample_rate = sample_rate;
    this->frame_size = frame_size;
    this->n_frames = n_frames;

    // create plugin instance
    plugin = new Plugin(uri, sample_rate, frame_size);

    // set default vars values
    n_points_default = 4;
    smaller.jack_load = 100.0;
    bigger.jack_load = 0.0;

    // create testing points for each parameter
    slicing_parameters();

    // create input buffer
    input_buffer = new float[frame_size];

    // TODO: select input buffer
    // random: 2 * ((float) rand() / (float) RAND_MAX) - 1.0;
    for (uint32_t i = 0; i < frame_size; i++)
    {
        input_buffer[i] = 1.0;
    }
}

Bench::~Bench()
{
    delete plugin;
    delete[] input_buffer;
}

void Bench::slicing_parameters(void)
{
    params.resize(plugin->control->inputs_by_index.size());

    // loop all input controls ports and store the quantity of points to test
    for (uint32_t i = 0; i < plugin->control->inputs_by_index.size(); i++)
    {
        port_data_t *port = &plugin->control->inputs_by_index[i];
        if (port->is_enumeration || port->is_scale_point)
        {
            n_points_to_test.push_back(port->scale_points.count);
        }
        else if (port->is_toggled || port->is_trigger)
        {
            n_points_to_test.push_back(2);
        }
        else
        {
            n_points_to_test.push_back(n_points_default);
        }
    }
}

void Bench::test_points(uint32_t depth, vector<uint32_t> & params, vector<uint32_t> & n_points)
{
    if (depth > 0)
    {
        for (uint32_t i = 0; i < n_points[depth-1]; i++)
        {
            params[depth-1] = i;
            test_points(depth-1, params, n_points);
        }
    }
    else
    {
        for (uint32_t i = 0; i < params.size(); i++)
        {
            float range = plugin->control->inputs_by_index[i].max - plugin->control->inputs_by_index[i].min;
            float step = range / (float)(n_points[i] - 1);

            float value;
            value = plugin->control->inputs_by_index[i].min;
            value += (step * params[i]);

            // integer
            if (plugin->control->inputs_by_index[i].is_integer)
            {
                value = (int32_t) round(value);
            }

            // enumeration and scale point
            if (plugin->control->inputs_by_index[i].is_enumeration ||
                plugin->control->inputs_by_index[i].is_scale_point)
            {
                uint32_t index = params[i];
                value = plugin->control->inputs_by_index[i].scale_points.values[index];
            }

            // TODO: toggle and trigger
            if (plugin->control->inputs_by_index[i].is_toggled ||
                plugin->control->inputs_by_index[i].is_trigger)
            {
            }

            // TODO: logarithmic
            if (plugin->control->inputs_by_index[i].is_logarithmic)
            {
            }

            plugin->control->inputs_by_index[i].value = value;

            bench_info_t tmp;
            run_and_calc(&tmp);

            if (tmp.jack_load < smaller.jack_load) smaller = tmp;
            if (tmp.jack_load > bigger.jack_load) bigger = tmp;
        }
    }
}

void Bench::run_and_calc(bench_info_t* var)
{
    plugin->run(frame_size);

    struct timespec ts = bench_start();
    for (uint32_t i = 0; i < n_frames; ++i)
    {
        plugin->run(frame_size);
    }
    var->total = bench_end(&ts);

    var->average = (var->total / (double)n_frames);
    double jack_latency = (double) frame_size / sample_rate;
    var->jack_load = 2.0 * (var->average * 100.0) / jack_latency;
}

void Bench::process(void)
{
    // copies the input buffer to plugin inputs
    for (uint32_t i = 0; i < plugin->audio->inputs_by_index.size(); i++)
    {
        plugin->audio->inputs_by_index[i].write_buffer(input_buffer, frame_size);
    }

    // process the benchmark using the minimum controls values
    plugin->control->set_value(MINIMUM_PRESET_LABEL);
    run_and_calc(&min);

    // process the benchmark using the maximum controls values
    plugin->control->set_value(MAXIMUM_PRESET_LABEL);
    run_and_calc(&max);

    // process the benchmark using the default controls values
    plugin->control->set_value(DEFAULT_PRESET_LABEL);
    run_and_calc(&def);

    if (full_test)
    {
        test_points(params.size(), params, n_points_to_test);

        if (min.jack_load > bigger.jack_load) bigger = min;
        if (max.jack_load > bigger.jack_load) bigger = max;
        if (def.jack_load > bigger.jack_load) bigger = def;

        if (min.jack_load < smaller.jack_load) smaller = min;
        if (max.jack_load < smaller.jack_load) smaller = max;
        if (def.jack_load < smaller.jack_load) smaller = def;
    }
}

void Bench::print(void)
{
    printf("Plugin: %s\n", plugin->uri.c_str());
    printf("%12s%14s%13s%13s\n", "TestName", "TotalTime(s)", "AvrTime(s)", "JackLoad(%)");
    printf("%12s%14.8f%13.8f%13f\n", "MinValues", min.total, min.average, min.jack_load);
    printf("%12s%14.8f%13.8f%13f\n", "DefValues", def.total, def.average, def.jack_load);
    printf("%12s%14.8f%13.8f%13f\n", "MaxValues", max.total, max.average, max.jack_load);

    if (full_test)
    {
        printf("%12s%14.8f%13.8f%13f\n", "BestResult", smaller.total, smaller.average, smaller.jack_load);
        printf("%12s%14.8f%13.8f%13f\n", "WorstResult", bigger.total, bigger.average, bigger.jack_load);
    }

    printf("\n");
}
