
#ifndef BENCH_H
#define BENCH_H

#include "plugin.h"

using namespace std;

struct bench_info_t
{
    double total, average, jack_load;
    std::map<uint32_t,port_data_t> plugin_preset;
};

class Bench
{
private:
    void slicing_parameters(void);
    std::vector<uint32_t> params;
    float *input_buffer;

public:
    Bench(const char* uri, uint32_t sample_rate, uint32_t frame_size, uint32_t n_frames);
    ~Bench();

    void run_and_calc(bench_info_t* var);
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
