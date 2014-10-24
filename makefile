raspi_synth_test: raspi_synth_test.o blit_saw_oscillator.o biquad_filter.o
	g++ raspi_synth_test.o blit_saw_oscillator.o biquad_filter.o -o raspi_synth_test -O3 -lasound

raspi_synth_test.o: raspi_synth_test.cpp
	g++ raspi_synth_test.cpp -c -std=c++0x

blit_saw_oscillator.o: blit_saw_oscillator.cpp blit_saw_oscillator.h
	g++ blit_saw_oscillator.cpp -c -std=c++0x
	
biquad_filter.o: biquad_filter.cpp biquad_filter.h
	g++ biquad_filter.cpp -c -std=c++0x
