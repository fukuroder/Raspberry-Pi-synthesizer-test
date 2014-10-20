/*
 * blit_saw_oscillator.cpp
 *
 * Copyright (c) 2014, fukuroda (https://github.com/fukuroder)
 * Released under the MIT license
 */

#include<math.h>
#include<algorithm>
#include "blit_saw_oscillator.h"

// constructor
blit_saw_oscillator_note::blit_saw_oscillator_note()
:envelope(Off)
,t(0.0)
,value(0.0)
,n(0)
,dt(0.0)
,note_no(0)
,velocity(0.0)
{}

// constructor
blit_saw_oscillator::blit_saw_oscillator(double leak=0.995, double srate=44100.0)
:_Leak(leak),srate(srate)
{
    // sine wave table
    for(unsigned int ii = 0; ii < _sinTable.size()/*=2^n+1*/; ++ii)
    {
        _sinTable[ii] = sin( 2.0*M_PI * ii/(_sinTable.size()-1));
    }
}

//
void blit_saw_oscillator::trigger(unsigned char note_no, unsigned char velocity)
{
    //
    auto available_note = std::find_if(
        _notes.begin(),
        _notes.end(),
        [](const blit_saw_oscillator_note& n){return n.envelope == blit_saw_oscillator_note::Off;});

    if( available_note != _notes.end() )
    {
        available_note->note_no = note_no;
        available_note->velocity = (double)velocity / 127;
        available_note->envelope = blit_saw_oscillator_note::On;

        //
        double freq = 440.0*( ::pow(2.0, (note_no - _note_no_center)/12.0 ));
        available_note->n = static_cast<int>(srate / 2.0 / freq);
        available_note->dt = freq / srate;
    	available_note->value = 0.0;
        available_note->t = 0.5;
    }
}

//
void blit_saw_oscillator::release(unsigned char note_no)
{
    auto target_note = std::find_if(
        _notes.begin(),
        _notes.end(),
        [note_no](const blit_saw_oscillator_note& n){return n.note_no == note_no;});

    if( target_note != _notes.end() )
    {
        //
        target_note->envelope = blit_saw_oscillator_note::Off;
    }
}

//
double blit_saw_oscillator::LinearInterpolatedSin(double x)
{
    //
    double pos = (_sinTable.size()-1) * x;

    //
    unsigned int idx_A = static_cast<int>(pos);

    //
    double s = pos - idx_A;

    //
    return (1.0-s) * _sinTable[idx_A] + s*_sinTable[idx_A+1];
}

//
double blit_saw_oscillator::BLIT(double t, int n)
{
    //
    double den = LinearInterpolatedSin(0.5*t);

    if( den < 1.0e-12 )
    {
        return 2.0*(2*n+1);
    }

    double num = LinearInterpolatedSin(::fmod((n+0.5)*t, 1.0));

    return 2.0*num/den;
}

//
double blit_saw_oscillator::render()
{
    double value = 0.0;
    for(auto &note : _notes)
    {
        if( note.envelope == blit_saw_oscillator_note::On ){
            // add
            value += note.value * note.velocity;
        }
    }
    return value;
}

//
void blit_saw_oscillator::next()
{
    for(auto& note : _notes)
    {
        if( note.envelope == blit_saw_oscillator_note::On ){
            // add
            note.t += note.dt;
            if ( 1.0 <= note.t )note.t -= 1.0;

            note.value = note.value*_Leak + (BLIT(note.t, note.n)-2.0)*note.dt;
        }
    }

}
