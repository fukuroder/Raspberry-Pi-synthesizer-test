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
#include<set>
#include<algorithm>
#include<stdexcept>

#include"blit_saw_oscillator.h"

int main()
{
    snd_pcm_t *pcm = nullptr;
    snd_rawmidi_t *nanoKEY2 = nullptr;

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

            //---------------
            // audio process
            //---------------
            for(short& s : buffer)
            {
                s = 1000.0*oscillator.process();
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

    return 0;
}
