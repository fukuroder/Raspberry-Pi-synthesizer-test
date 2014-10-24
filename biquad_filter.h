/*
 * raspi_synth_test.cpp
 *
 * Copyright (c) 2014, fukuroda (https://github.com/fukuroder)
 * Released under the MIT license
 */

// Biquad Filter�N���X
#pragma once

class biquad_filter
{
protected:
    // �t�B���^�W��
    double _a0, _a1, _a2, _b1, _b2;

    // �o�b�t�@
    double _in_z, _in_zz, _out_z, _out_zz;

public:
    // �R���X�g���N�^
    biquad_filter(double cutoff, double resonance, double low, double band, double high);

    // ���s
    double process(double in);

    // �t�B���^�W���X�V
    void update(double cutoff, double resonance, double low, double band, double high);

    // �o�b�t�@�̃��Z�b�g
    void reset_buffer();
};
