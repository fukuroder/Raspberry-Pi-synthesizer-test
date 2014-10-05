raspi_synth_test: raspi_synth_test.cpp blit_saw_oscillator.cpp blit_saw_oscillator.h
	g++ raspi_synth_test.cpp blit_saw_oscillator.cpp -o raspi_synth_test -std=c++0x -O3 -lasound
