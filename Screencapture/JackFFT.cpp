#include "JackFFT.h"

//Jack audio variables
unsigned int nports;
jack_port_t *port;

//FFTW variables
double *input;
fftw_complex *output;
fftw_plan plan;
void slowfall(float nieuw);
void calc_fft();
void DrawTerminal();
double resolution;
float power[513];
float oldPower[513];
float maxPower[513];
UINT8* pointer;
UINT8 gamma[256] = { 0 };

//thread comm
int startTerminalDraw = 2;

int process(jack_nframes_t nframes, void *arg)
{


	jack_default_audio_sample_t *in = (jack_default_audio_sample_t *)jack_port_get_buffer(port, nframes);

	
	for (int i = 0; i < nframes; i++)
	{
		input[i] = in[i];
	}

	
	fftw_execute(plan);
	calc_fft();

	float red = 0;
	float green = 0;
	float blue = 0;
	static float redMax = 0.1;
	static float greenMax = 0.1;
	static float blueMax = 0.1;
	int i;

	/*
	for (i = 1; i < 6; i++)
	{
		red += power[i];
	}
	red = red / i;

	if (red>redMax)
		redMax = red;
	else
		redMax *= 0.99;
	
	for (i = i; i < 40; i++)
	{
		green += power[i];
	}
	green = green / (i - 6);

	if (green>greenMax)
		greenMax = green;
	else
		greenMax *= 0.99;

	for (i = i; i < 513; i++)
	{
		blue += power[i];
	}
	blue = blue / (i - 40);

	if (blue>blueMax)
		blueMax = blue;
	else
		blueMax *= 0.99;

	std::cout << "\t\t\t\t\t\t\t\r";
	std::cout << "RGB:" << (int)((red / redMax) * 255) << "," << (int)((green / greenMax) * 255) << "," << (int)((blue / blueMax) * 255) << "," << "\r";

	for (int i = 0; i < 439; i++)
	{
		*(pointer + (i*3)) = (UINT8)((red / redMax) * 255);
		*(pointer + (i*3) + 1) = (UINT8)((red / redMax) * 255);
		*(pointer + (i*3) + 2) = (UINT8)((red / redMax) * 255);
	}*/
	

	for (int i = 0; i < 439; i+=3)
	{
		if (power[i] > maxPower[i])
			maxPower[i] = power[i];
		else
			maxPower[i] *= 0.95;

		if (power[i] > 0 && power[i] > oldPower[i])
		{
			oldPower[i] = power[i];
		}
		else if (power[i] < oldPower[i])
		{
			oldPower[i] *= 0.9;
		}

		//*(pointer +1+ (i * 3)) = (oldPower[i]*255)/maxPower[i];
		int temp = (int)((oldPower[i] * 255) / maxPower[i]);
		if (temp > 255)
			temp = 255;
		else if (temp < 0)
			temp = 0;
		*(pointer + (i * 3)) = gamma[temp];
	}

	
	return 0;
}


void DrawTerminal()
{
	Sleep(1000);
	std::cout << "thread gestart " << std::endl;
	static HWND myconsole = GetConsoleWindow();
	static HDC mydc = GetDC(myconsole);
	static RECT myWindowRect;
	GetWindowRect(myconsole, &myWindowRect);

	static COLORREF WHITE = RGB(0, 0, 0);
	static COLORREF BLUE = RGB(255, 255, 255);
	while (startTerminalDraw !=0)
	{
		
		for (int y = 0; y < 100; y++)
		{
			for (int x = 0; x < 513; x += 3)
			{

				SetPixel(mydc, x, y, WHITE);
			}
		}
		while (startTerminalDraw == 1)
		{
			


			for (int y = 0; y < 100; y++)
			{
				for (int x = 0; x < 100; x++)
				{
					if (y > power[x])
					{
						SetPixel(mydc, x, y, WHITE);
					}
					else
						SetPixel(mydc, x, y, BLUE);

				}
			}
			
		}
	}
	ReleaseDC(myconsole, mydc);
	std::cout << "thread sluit " << std::endl;
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
	//register 1 port. multi channel will later be added
	setup_ports();
	jack_set_process_callback(client, process,0);
	for (int i = 0; i < 256; i++)
		gamma[i] = ((float)pow((float)((float)i / 255), (float)((float)1 / 0.6))) * (float)255;
}


JackFFT::~JackFFT()
{
	startTerminalDraw = 0;
}


void JackFFT::init(UINT8* tp)
{
	pointer = tp;
}

void JackFFT::start()
{
	if (startTerminalDraw != 1)
	{
		terminaldraw = new std::thread(DrawTerminal);
	}
	
	
	if (jack_activate(client)) {
		fprintf(stderr, "cannot activate client");
	}
	for (int i = 0; i < 439; i++)
	{
		*(pointer + (i * 3)) = 0;
		*(pointer + (i * 3) + 1) = 0;
		*(pointer + (i * 3) + 2) = 0;
	}
	startTerminalDraw = 1;
}

void JackFFT::stop()
{
	if (jack_deactivate(client)) {
		fprintf(stderr, "cannot deactivate client");
	}
	startTerminalDraw = 2;
}

void JackFFT::setup_ports()
{
	port = jack_port_register(client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

}


void JackFFT::setup_plan()
{
	input = static_cast<double*      >(fftw_malloc(nframes  * sizeof(double)));
	output = static_cast<fftw_complex*>(fftw_malloc((nframes / 2 + 1) * sizeof(fftw_complex)));

	resolution = (96000 / 2) / (nframes / 2 + 1);

	plan = fftw_plan_dft_r2c_1d(nframes, input, output, FFTW_ESTIMATE);
}

void calc_fft()
{
	for (int i = 0; i < 513; i++)
	{
		power[i] = 20*log10(sqrt(output[i][0] * output[i][0]) + (output[i][1] * output[i][1]));
	}
	
}

float slowfall(float nieuw,float oud)
{
	
	
	if (nieuw > oud)
	{
		oud = nieuw;
	}
	else
	{
		oud = oud* 0.97;
	}

	return oud;
}