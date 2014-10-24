/*
 * raspi_synth_test.cpp
 *
 * Copyright (c) 2014, fukuroda (https://github.com/fukuroder)
 * Released under the MIT license
 */

#include<math.h>
#include<stdexcept>
#include "biquad_filter.h"

// コンストラクタ
biquad_filter::biquad_filter(double cutoff, double resonance, double low, double band, double high)
{
    update(cutoff, resonance, low, band, high);
    reset_buffer();
}

// 実行
double biquad_filter::process(double in)
{
    const double denormal_canceler = 1.0e-100;
    const double out = _a0*in + _a1*_in_z + _a2*_in_zz + _b1*_out_z + _b2*_out_zz;

    _in_zz = _in_z + denormal_canceler;
    _in_z = in + denormal_canceler;
    _out_zz = _out_z + denormal_canceler;
    _out_z = out + denormal_canceler;

    return out;
}

// フィルタ係数更新
void biquad_filter::update(double cutoff, double resonance, double low, double band, double high)
{
    if( resonance < 1.0e-3 )
    {
        throw std::runtime_error("resonance < 1.0e-3");
    }

    const double s = ::tan( cutoff * M_PI / 44100.0 );
    const double t = s / resonance;
    const double u = s*s + t + 1.0;

    _a0 = (low*s*s + band*t + high)/u;
    _a1 = 2.0*(low*s*s - high)/u;
    _a2 = (low*s*s - band*t + high)/u;
    _b1 = -2.0*(s*s - 1.0)/u;
    _b2 = -(s*s - t + 1.0)/u;
}

// バッファのリセット
void biquad_filter::reset_buffer()
{
    _in_z = 0.0; _in_zz = 0.0; _out_z = 0.0; _out_zz = 0.0;
}
