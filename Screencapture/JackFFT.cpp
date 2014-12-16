#include "JackFFT.h"

//Jack audio variables
unsigned int nports;
jack_port_t *port;

//FFTW variables
double *input;
fftw_complex *output;
fftw_plan plan;

int process(jack_nframes_t nframes, void *arg)
{


	jack_default_audio_sample_t *in = (jack_default_audio_sample_t *)jack_port_get_buffer(port, nframes);

	
	memcpy(input, in, sizeof(sample_t)*nframes);

	fftw_execute(plan);

	std::cout << sqrt(output[2][0] * output[2][0] + output[2][1] + output[2][1]) << "\r";
	return 0;
}


JackFFT::JackFFT()
{
	//init FFTW
	setup_plan();

	//init JACK
	//register the client
	if ((client = jack_client_open("Light Show", JackNoStartServer, NULL)) == 0) {
               fprintf(stderr, "jack server not running?\n");
	}
	//register 6 ports
	//I will take for granted that you use a 5.1 system.
	setup_ports();
	jack_set_process_callback(client, process,0);


	if (jack_activate(client)) {
		fprintf(stderr, "cannot activate client");
	}

	
}


JackFFT::~JackFFT()
{
}


void JackFFT::setup_ports()
{
	port = jack_port_register(client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

}

void JackFFT::setup_plan()
{
	input = static_cast<double*      >(fftw_malloc(nframes  * sizeof(double)));
	output = static_cast<fftw_complex*>(fftw_malloc((nframes / 2 + 1) * sizeof(fftw_complex)));

	plan = fftw_plan_dft_r2c_1d(nframes, input, output, FFTW_ESTIMATE);
}