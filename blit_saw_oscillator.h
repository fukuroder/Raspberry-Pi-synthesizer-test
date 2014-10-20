/*
 * blit_saw_oscillator.h
 *
 * Copyright (c) 2014, fukuroda (https://github.com/fukuroder)
 * Released under the MIT license
 */

#pragma once
#include<array>

//
class blit_saw_oscillator_note
{
public:
    //
    blit_saw_oscillator_note();

    // ADSR
    enum ADSR
    {
        //
        Off,

        //
        On,
    };

    //
    ADSR envelope;

    //
    double t;

    //
    double value;

    //
    int n;

    //
    double dt;

    //
    unsigned char note_no;

    //
    double velocity;
};

//
class blit_saw_oscillator
{
public:
    // constructor
    blit_saw_oscillator(double leak, double srate);

    //
    void trigger(unsigned char note_no, unsigned char velocity);

    //
    void release(unsigned char note_no);

    //
    double render();

    //
    void next();

protected:
    //
    static const int _note_no_center = 69;

    //
    double _Leak;

    //
    double srate;

    //
    std::array<double, (1<<10)+1> _sinTable;

    //
    std::array<blit_saw_oscillator_note, 4> _notes;

    //
    double LinearInterpolatedSin( double t );

    //
    double BLIT( double t, int n );
};
