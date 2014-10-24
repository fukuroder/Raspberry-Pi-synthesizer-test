/*
 * raspi_synth_test.cpp
 *
 * Copyright (c) 2014, fukuroda (https://github.com/fukuroder)
 * Released under the MIT license
 */

// Biquad Filterクラス
#pragma once

class biquad_filter
{
protected:
    // フィルタ係数
    double _a0, _a1, _a2, _b1, _b2;

    // バッファ
    double _in_z, _in_zz, _out_z, _out_zz;

public:
    // コンストラクタ
    biquad_filter(double cutoff, double resonance, double low, double band, double high);

    // 実行
    double process(double in);

    // フィルタ係数更新
    void update(double cutoff, double resonance, double low, double band, double high);

    // バッファのリセット
    void reset_buffer();
};
