#pragma once

#include "jack\weakjack.h"
#include "jack\jack.h"

#include "fftw\fftw3.h"
#include <iostream>

typedef jack_default_audio_sample_t sample_t;

class JackFFT
{
public:
	JackFFT();
	~JackFFT();

	void init(UINT8*);
	void start();
	void stop();

	
	UINT32 *pBits;

private:
	void setup_ports();

	/* FFT DATA */

	void setup_plan();
	void JackFFT::slowfall(float);

	/* JACK data */
	
	jack_nframes_t nframes = 1024;
	const size_t sample_size = sizeof(jack_default_audio_sample_t);


	jack_client_t *client;
};

