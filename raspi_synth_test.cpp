/*
 * raspi_synth_test.cpp
 *
 * Copyright (c) 2014, fukuroda (https://github.com/fukuroder)
 * Released under the MIT license
 */

#include<alsa/asoundlib.h>  // sudo apt-get install libasound2-dev
#include<climits>
#include<iostream>
#include<sstream>
#include<fstream>
#include<string>
#include<vector>
#include<array>
#include<map>
#include<algorithm>
#include<stdexcept>

#include"blit_saw_oscillator.h"
#include"biquad_filter.h"

int main()
{
    snd_pcm_t *pcm = nullptr;
    snd_rawmidi_t *nanoKEY2 = nullptr;
    snd_rawmidi_t *nanoKONTROL2 = nullptr;

    try
    {
        // open audio device
        // BEHRINGER UCA222 ---> http://www.behringer.com/EN/Products/UCA222.aspx
        if( ::snd_pcm_open(
            &pcm,
            "hw:CODEC",
            SND_PCM_STREAM_PLAYBACK,
            0) )
        {
            throw std::runtime_error("snd_pcm_open error");
        }

        // set parameters
        if( ::snd_pcm_set_params(
            pcm,
            SND_PCM_FORMAT_S16,
            SND_PCM_ACCESS_RW_INTERLEAVED,
            1, // mono
            44100,
            0,
            20*1000) ) // 20ms
        {
            throw std::runtime_error("snd_pcm_set_params error");
        }

        // get buffer size and period size
        snd_pcm_uframes_t buffer_size = 0;
        snd_pcm_uframes_t period_size = 0;
        if( ::snd_pcm_get_params(
            pcm,
            &buffer_size,
            &period_size) )
        {
            throw std::runtime_error("snd_pcm_get_params error");
        }

        // allocate buffer
        std::vector<short> buffer(period_size);

        std::cout << "start ("
                  << "buffer_size=" << buffer_size << ", "
                  << "period_size=" << period_size << ")"
                  << std::endl;

        std::cout << "MIDI:" << std::flush;

        // open MIDI device
        // KORG nanoKEY2 ---> http://www.korg.co.jp/Product/Controller/nano2/nanoKEY.html
        if( ::snd_rawmidi_open(
            &nanoKEY2,
            nullptr,
            "hw:nanoKEY2",
            SND_RAWMIDI_NONBLOCK ) )
        {
            throw std::runtime_error("snd_rawmidi_open error");
        }

        // オシレータ作成
        blit_saw_oscillator oscillator(0.995, 44100);

        // open MIDI device
        // KORG nanoKONTROL2 ---> http://www.korg.co.jp/Product/Controller/nano2/nanoKONTROL.html
        if( ::snd_rawmidi_open(
            &nanoKONTROL2,
            nullptr,
            "hw:nanoKONTROL2",
            SND_RAWMIDI_NONBLOCK ) )
        {
            throw std::runtime_error("snd_rawmidi_open error");
        }

        // parameters
        double volume = 1.0;
        double cutoff = 400.0;
        double resonance = M_SQRT1_2;
        double low = 1.0;
        double band = 0.0;
        double high = 0.0;

        // Biquad Filter作成
        biquad_filter filter(cutoff, resonance, low, band, high);

        do{
            //--------------------
            // MIDI event process
            //--------------------
            do{
                // read MIDI input
                std::array<unsigned char, 3> midi_data = {};
                ssize_t midi_read_count = ::snd_rawmidi_read(nanoKEY2, midi_data.data(), midi_data.size());

                if( midi_read_count != (int)midi_data.size() )
                {
                    // no more data
                    break;
                }

                ::printf("\rMIDI message:0x%02x 0x%02x 0x%02x", midi_data[0], midi_data[1], midi_data[2]);
                ::fflush(stdout);

                if( midi_data[0] == 0x90/*note on*/)
                {
                    oscillator.trigger(midi_data[1], midi_data[2]);
                }
                else if( midi_data[0] == 0x80/*note off*/)
                {
                    oscillator.release(midi_data[1]);
                }
            }while(true);

            // key:control number / value:value
            std::map<unsigned char, unsigned char> ControlChangeMap;
            do{
                // read MIDI input
                std::array<unsigned char, 3> midi_data = {};
                ssize_t midi_read_count = ::snd_rawmidi_read(nanoKONTROL2, midi_data.data(), midi_data.size());

                if( midi_read_count != static_cast<int>(midi_data.size()) )
                {
                    // no more data
                    break;
                }

                ::printf("\rMIDI message:0x%02x 0x%02x 0x%02x", midi_data[0], midi_data[1], midi_data[2]);
                ::fflush(stdout);

                if( midi_data[0] == 0xb0/*control change*/){
                    // 最後の値のみを保持
                    ControlChangeMap[ midi_data[1] ] = midi_data[2];
                }

            }while(true);

            // update parameters
            for(const auto& key_value : ControlChangeMap)
            {
                unsigned char control_number = key_value.first;
                double value = static_cast<double>(key_value.second)/127;
                if( control_number == 0x10 /* knob1 */)
                {
                    volume = value;
                }
                else if( control_number == 0x11 /* knob2 */)
                {
                    cutoff = 40.0*(1.0-value) + 4000.0*value;
                }
                else if( control_number == 0x12 /* knob3 */)
                {
                    resonance = M_SQRT1_2*(1.0-value) + 20.0*value;
                }
                else if( control_number == 0x13 /* knob4 */)
                {
                    low = value;
                }
                else if( control_number == 0x14 /* knob5 */)
                {
                    band = value;
                }
                else if( control_number == 0x15 /* knob6 */)
                {
                    high = value;
                }
            }

            // フィルタの更新が必要か？
            auto isFilterUpdate = [](std::pair<unsigned char, unsigned char> key_value){
                unsigned char control_number = key_value.first;
                return control_number == 0x11 // knob2
                    || control_number == 0x12 // knob3
                    || control_number == 0x13 // knob4
                    || control_number == 0x14 // knob5
                    || control_number == 0x15;// knob6
            };
            if( std::any_of(ControlChangeMap.begin(), ControlChangeMap.end(), isFilterUpdate) )
            {
                // フィルタの更新(L/R)
                filter.update(cutoff, resonance, low, band, high);
            }

            //---------------
            // audio process
            //---------------
            for(short& s : buffer)
            {
                s = 1000.0 * filter.process( oscillator.process() );
            }

            //-------------
            // write audio
            //-------------
            if( ::snd_pcm_writei(pcm, buffer.data(), period_size) < 0 )
            {
                throw std::runtime_error("snd_pcm_writei error");
            }
        }while(true);

        std::cout << std::endl << "end" << std::endl;
    }
    catch(std::exception &ex)
    {
        std::cout << std::endl << "error:" << ex.what() << std::endl;
    }

    if(pcm != nullptr)
    {
        // close audio device
        ::snd_pcm_close(pcm);
    }

    if(nanoKEY2 != nullptr)
    {
        // close sound file
        ::snd_rawmidi_close(nanoKEY2);
    }

    if(nanoKONTROL2 != nullptr)
    {
        // close sound file
        ::snd_rawmidi_close(nanoKONTROL2);
    }

    return 0;
}
