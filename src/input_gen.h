/*
 * code from:
 * testsignal - LV2 Test Signal Generator
 * Robin Gareus <robin@gareus.org>
 * https://github.com/x42/testsignal.lv2
 */

#ifndef GENERATOR_H
#define GENERATOR_H

class Generator {
private:
    int mode;

    // plugin ports
    float* reflevel;
    float* output;

    // signal level
    float  lvl_db; // corresponds to 'reflevel'
    float  lvl_coeff_target;
    float  lvl_coeff;

    // sine/square wave generator state
    float  phase;
    float  phase_inc;

    // impulse period counters
    uint32_t k_cnt;
    uint32_t k_period100;
    uint32_t k_period1;
    uint32_t k_period5s;

    // sweep settings
    double swp_log_a, swp_log_b;
    // sweep counter
    uint32_t swp_period;
    uint32_t swp_cnt;

    // pseudo-random number state
    uint32_t rseed;
    bool  g_pass;
    float g_rn1;
    float b0, b1, b2, b3, b4, b5, b6; // pink noise

    uint32_t rand_int(void);
    float rand_float(void);
    float rand_gauss(void);
    void gen_sine(uint32_t n_samples);
    void gen_square(uint32_t n_samples);
    void gen_uniform_white(uint32_t n_samples);
    void gen_gaussian_white(uint32_t n_samples);
    void gen_pink(uint32_t n_samples);
    void gen_kroneker_delta(uint32_t n_samples, const uint32_t period);
    void gen_sawtooth(uint32_t n_samples, const uint32_t period);
    void gen_triangle(uint32_t n_samples, const uint32_t period);
    void gen_sine_log_sweep(uint32_t n_samples);

public:
    Generator(uint32_t sample_rate, const char *signal, double duration);
    ~Generator();

    const char *signal_name;
    float* get_frame(uint32_t n_samples);
};

#endif
