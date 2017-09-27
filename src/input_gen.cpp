#ifndef _GNU_SOURCE
#define _GNU_SOURCE // needed for M_PI
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "input_gen.h"

Generator::Generator(uint32_t sample_rate, const char *signal, double duration)
{
    phase = 0.0;
    phase_inc = 1000.0 / (float) sample_rate;
    k_period100 = sample_rate / 100;
    k_period1   = sample_rate;
    k_period5s  = sample_rate * 5;
    k_cnt = 0;

    swp_log_a = swp_log_b = 0;
    swp_period = swp_cnt = 0;
    rseed = 0;
    g_pass = false;
    g_rn1 = 0;
    b0 = b1 = b2 = b3 = b4 = b5 = b6 = 0;

    // log frequency sweep
    const double f_min = 20.;
    const double f_max = (sample_rate * .5) < 20000. ? (sample_rate * .5) : 20000.;
    swp_period = ceil(duration * sample_rate);
    swp_log_b = log(f_max / f_min) / swp_period;
    swp_log_a = f_min / (swp_log_b * sample_rate);

    rseed = time(NULL) % UINT_MAX;
    if (rseed == 0) rseed = 1;

    // use default signal level
    lvl_db = -18.0;
    lvl_coeff = 0.0;
    lvl_coeff_target = powf(10, 0.05 * lvl_db);

    // create buffer
    // the output vector size should be frame_size not sample_rate
    // but well... some extra bytes won't hurt
    output = new float[sample_rate];

    // select the signal
    mode = 0; // sine (default)
    if (strcmp(signal, "square") == 0) mode = 1;
    else if (strcmp(signal, "white") == 0) mode = 2;
    else if (strcmp(signal, "gwhite") == 0) mode = 3;
    else if (strcmp(signal, "pink") == 0) mode = 4;
    else if (strcmp(signal, "impulse") == 0) mode = 5;
    else if (strcmp(signal, "sweep") == 0) mode = 6;
    else if (strcmp(signal, "sawtooth") == 0) mode = 9;
    else if (strcmp(signal, "triangle") == 0) mode = 10;

    const char *signals_name[] = {"Sine Wave", "Square Wave", "Uniform White Noise",
        "Gaussian Shaped White Noise", "Pink Noise", "Impulse", "Sine Sweep", NULL, NULL,
        "Sawtooth Wave", "Triangle Wave"};

    signal_name = signals_name[mode];
}

Generator::~Generator(void)
{
    delete[] output;
}

float* Generator::get_frame(uint32_t n_samples)
{
    lvl_coeff += .1 * (lvl_coeff_target - lvl_coeff) + 1e-12;

    if (mode <= 0)      { gen_sine(n_samples); }
    else if (mode <= 1) { gen_square(n_samples); }
    else if (mode <= 2) { gen_uniform_white(n_samples); }
    else if (mode <= 3) { gen_gaussian_white(n_samples); }
    else if (mode <= 4) { gen_pink(n_samples); }
    else if (mode <= 5) { gen_kroneker_delta(n_samples, k_period100); }
    else if (mode <= 6) { gen_sine_log_sweep(n_samples); }
    else if (mode <= 7) { gen_kroneker_delta(n_samples, k_period1); }
    else if (mode <= 8) { gen_kroneker_delta(n_samples, k_period5s); }
    else if (mode <= 9) { gen_sawtooth(n_samples, k_period100); }
    else                { gen_triangle(n_samples, k_period100); }

    return output;
}

/* pseudo-random number generators */

inline uint32_t Generator::rand_int(void)
{
    // 31bit Park-Miller-Carta Pseudo-Random Number Generator
    // http://www.firstpr.com.au/dsp/rand31/
    uint32_t hi, lo;
    lo = 16807 * (rseed & 0xffff);
    hi = 16807 * (rseed >> 16);

    lo += (hi & 0x7fff) << 16;
    lo += hi >> 15;
    lo = (lo & 0x7fffffff) + (lo >> 31);
    return (rseed = lo);
}

inline float Generator::rand_float(void)
{
    return (rand_int() / 1073741824.f) - 1.f;
}

float Generator::rand_gauss(void)
{
    // Gaussian White Noise
    // http://www.musicdsp.org/archive.php?classid=0#109
    float x1, x2, r;

    if (g_pass) {
        g_pass = false;
        return g_rn1;
    }

    do {
        x1 = rand_float();
        x2 = rand_float();
        r = x1 * x1 + x2 * x2;
    } while ((r >= 1.0f) || (r < 1e-22f));

    r = sqrtf(-2.f * logf(r) / r);

    g_pass = true;
    g_rn1 = r * x2;
    return r * x1;
}

void Generator::gen_sine(uint32_t n_samples)
{
    float _phase = phase;
    const float level = lvl_coeff_target;

    for (uint32_t i = 0 ; i < n_samples; ++i) {
        output[i] = level * sinf(2.0f * M_PI * _phase);
        _phase += phase_inc;
    }

    phase = fmodf(_phase, 1.0);
}

void Generator::gen_square(uint32_t n_samples)
{
    float _phase = phase;
    const float level = lvl_coeff_target;

    for (uint32_t i = 0 ; i < n_samples; ++i) {
        output[i] = sinf(2.0f * M_PI * _phase) > 0 ? level : - level;
        _phase += phase_inc;
    }

    phase = fmodf(_phase, 1.0);
}

void Generator::gen_uniform_white(uint32_t n_samples)
{
    const float level = lvl_coeff_target;

    for (uint32_t i = 0 ; i < n_samples; ++i) {
        output[i] = level * rand_float();
    }
}

void Generator::gen_gaussian_white(uint32_t n_samples)
{
    const float level = lvl_coeff_target * 0.7079f;

    for (uint32_t i = 0 ; i < n_samples; ++i) {
        output[i] = level * rand_gauss();
    }
}

void Generator::gen_pink(uint32_t n_samples)
{
    float *out = output;
    const float level = lvl_coeff_target / 2.527f;

    // localize variables
    float _b0 = b0;
    float _b1 = b1;
    float _b2 = b2;
    float _b3 = b3;
    float _b4 = b4;
    float _b5 = b5;
    float _b6 = b6;

    while (n_samples-- > 0) {
        // Paul Kellet's refined method
        // http://www.musicdsp.org/files/pink.txt
        // NB. If 'white' consists of uniform random numbers,
        // the pink noise will have an almost gaussian distribution.
        const float white = level * rand_float();
        _b0 = .99886f * _b0 + white * .0555179f;
        _b1 = .99332f * _b1 + white * .0750759f;
        _b2 = .96900f * _b2 + white * .1538520f;
        _b3 = .86650f * _b3 + white * .3104856f;
        _b4 = .55000f * _b4 + white * .5329522f;
        _b5 = -.7616f * _b5 - white * .0168980f;
        *out++ = _b0 + _b1 + _b2 + _b3 + _b4 + _b5 + _b6 + white * 0.5362f;
        _b6 = white * 0.115926f;
    }

    // copy back variables
    b0 = _b0;
    b1 = _b1;
    b2 = _b2;
    b3 = _b3;
    b4 = _b4;
    b5 = _b5;
    b6 = _b6;
}

void Generator::gen_kroneker_delta(uint32_t n_samples, const uint32_t period)
{
    float *out = output;
    memset(out, 0, n_samples * sizeof (float));

    uint32_t _k_cnt = k_cnt;

    while (n_samples > _k_cnt) {
        out[_k_cnt] = 1.0f;
        _k_cnt += period;
    }

    k_cnt = _k_cnt - n_samples;
}

void Generator::gen_sawtooth(uint32_t n_samples, const uint32_t period)
{
    float *out = output;
    uint32_t _k_cnt = k_cnt % period;

    for (uint32_t i = 0 ; i < n_samples; ++i) {
        out[i] = -1.f + 2.f * _k_cnt / (float) period;
        _k_cnt = (_k_cnt + 1) % period;
    }

    k_cnt = _k_cnt;
}

void Generator::gen_triangle(uint32_t n_samples, const uint32_t period)
{
    float *out = output;
    uint32_t _k_cnt = k_cnt % period;

    for (uint32_t i = 0 ; i < n_samples; ++i) {
        out[i] = -1.f + 2.f * fabsf(1 - 2.f * _k_cnt / (float) period);
        _k_cnt = (_k_cnt + 1) % period;
    }

    k_cnt = _k_cnt;
}

void Generator::gen_sine_log_sweep(uint32_t n_samples)
{
    float *out = output;
    uint32_t _swp_cnt = swp_cnt;
    const uint32_t _swp_period = swp_period;
    const double _swp_log_a = swp_log_a;
    const double _swp_log_b = swp_log_b;

    for (uint32_t i = 0 ; i < n_samples; ++i) {
        const double phase = _swp_log_a * exp(_swp_log_b * _swp_cnt) - _swp_log_a;
        out[i] = .12589f * sin(2. * M_PI * (phase - floor(phase)));
        _swp_cnt = (_swp_cnt + 1) % _swp_period;
    }

    swp_cnt = _swp_cnt;
}
