#ifdef CSMOD_USE_FFTW

#include <math.h>
#include "csmodule_fft.h"
#include "csmodcontainer.h"

char *CSmodule_FFT::docString()
{
	const char *r = "\
this module makes use of the wellknown FFTW library and calculates a fast fourier transform for \
it's <input>. the values of the frequency bands in the spectrum can be reused through the output \
connectors.\n\n\
the size of the input buffer is set in the module's property window. for efficient processing \
a buffer size which is a power of two should be used. the fourier transform will be calculated \
as soon as the input buffer is filled. so keep in mind that the size of the buffer introduces \
a certain latency. this might change in the future.\n\n\
<amplitude> scales the input value. the spectrum will always be in the range of 0.0 to 1.0.\n\
<fade up> and <fade down> sets the fade time of the spectrum in the range of 0.0 to 1.0, \
where higher is faster.\n\
<range low> and <range high> let you select a viewport within the calculated spectrum. \
0.0 means the lowest frequency and 1.0 means the maximum frequency (half of the samplerate). \
so for example, if your samplerate is 44100 hertz and <range low> and <range high> is set to 0.2 and 0.5 \
respectively, then the displayed (and output) range will be about 4410 to 11025 hertz. \
for small ranges however the resolution will be not enough. in this case you can change the \
buffer size in the property window. BUT: a large buffer will be computationally intense. \
";
	return const_cast<char*>(r);
}


CSmodule_FFT::~CSmodule_FFT()
{
	if (plan) fftwf_destroy_plan(plan);
	if (inp) fftwf_free(inp);
	if (outp) fftwf_free(outp);
}

CSmodule_FFT* CSmodule_FFT::newOne()
{
	return new CSmodule_FFT();
}


void CSmodule_FFT::init()
{
	gname = copyString("analyze");
	bname = copyString("fourier transform");
	name = copyString(bname);
	uname = copyString(bname);

	// --- look ----

	sx = 80;
	sy = headerHeight+3;
	sw = 360;

	minWidth = sx+sw+3;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_fadeUp = &(createInput("fu","fade up",0.8)->value);
	_fadeDown = &(createInput("fd","fade down",0.2)->value);
	_rangeStart = &(createInput("str","range low")->value);
	_rangeEnd = &(createInput("end","range high",1.0)->value);

	minHeight = minh = height;
	sh = height - headerHeight-8;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("out",i);
		char *n1 = int2char("out ",i+1);
		__out[i] = createOutput(n,n1);
		__out[i]->autoArrange = false;
		__out[i]->showLabel = false;
		_out[i] = &(__out[i]->value);
		free(n1); free(n);
	}

	oldNrOut = 0;
	setNrOut(16);

	// -- internals --

	plan = 0;
	inp = 0;
	outp = 0;
	setBufferSize(1024);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("buflen", "length of fft buffer", bufferSize, 16, 44100);
	createPropertyInt("nrchan", "nr of outputs", nrOut, 2,64);

}


void CSmodule_FFT::setNrOut(int newNrOut)
{
	if (newNrOut==oldNrOut) return;
	nrOut = newNrOut;
	oldNrOut = nrOut;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		bool b = (i<nrOut);
		setConnectorActive(__out[i], b);
	}

	arrangeConnectors();

	// arrange the outputs
	for (int i=0;i<nrOut;i++)
	{
		__out[i]->y = height;
		__out[i]->x = sx + (float)i/(nrOut-1)*sw;
	}

}


void CSmodule_FFT::setBufferSize(int newBufferSize)
{
	CSwait();
	ready = false;

	// free the old structures
	if (plan) fftwf_destroy_plan(plan);
	if (inp) fftwf_free(inp);
	if (outp) fftwf_free(outp);

	// set new
	bufferSize = newBufferSize;
	bufferPos = 0;
	inp = (float*) fftwf_malloc( sizeof(float) * bufferSize);
	outp = (float*) fftwf_malloc( sizeof(float) * bufferSize);
	dispSize = bufferSize >> 1;
	disp = (float*) calloc(dispSize, sizeof(float));

	// set up the plan to make the transform
	plan = fftwf_plan_r2r_1d(
		bufferSize, inp, outp,
		// kind of transform -> discrete hartley transform
		FFTW_DHT,
		// maybe performance plus
		FFTW_DESTROY_INPUT
		);

	ready = true;
}


void CSmodule_FFT::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrchan")==0)
		setNrOut(prop->ivalue);
	else
	if (strcmp(prop->name,"buflen")==0)
		setBufferSize(prop->ivalue);

	else CSmodule::propertyCallback(prop);
}


void CSmodule_FFT::step()
{
	if (!inp) return;

	// fill the buffer
	inp[bufferPos++] = *_amp * *_in;

	// make the transform if buffer full
	if (bufferPos>=bufferSize)
	{
		// get range
		*_rangeStart = max(0.0f,min(1.0f, *_rangeStart ));
		*_rangeEnd = max(0.0f,min(1.0f, *_rangeEnd ));
		float rangeDelta = *_rangeEnd - *_rangeStart;

		// make the transform
		fftwf_execute(plan);

		// -- transform *outp to display buffer --

		float *d = disp; // write pointer
		for (int i=0;i<dispSize;i++)
		{
			// get read index within range
			int p = ( ((float)i/dispSize)*rangeDelta+ *_rangeStart) * dispSize;
			// read
			float f = min(1.0f, (float)fabs(outp[p]) / bufferSize);
			// fade
			if (f> *d) *d += *_fadeUp * (f- *d);
			else *d += *_fadeDown * (f- *d);
			d++;
		}

		// -- feed the output connectors --

		d = disp;
		int st = dispSize / nrOut;
		for (int i=0;i<nrOut;i++)
		{
			csfloat *f = &__out[i]->value;
			for (int j=0;j<st;j++) *f += *d++;
		}
		// scale
		for (int i=0;i<nrOut;i++) __out[i]->value /= st;

		// reset buffer
		bufferPos = 0;
	}
}


void CSmodule_FFT::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	// draw spectrum background
	fl_color(FL_BLACK);
	fl_rectf(offx+zoom*(x+sx),offy+zoom*(y+sy),zoom*sw,zoom*sh);

	// draw spectrum
	int x1,y1,x2=0,y2=0;
	fl_color(FL_GREEN);
	for (int i=0;i<dispSize;i++)
	{
		x1 = offx+zoom*(x+sx+i*sw/dispSize);
		y1 = offy+zoom*(y+sy+sh-disp[i]*sh);
		if (i>0) fl_line(x2,y2,x1,y1);
		x2 = x1; y2=y1;
	}
}

#endif // #ifdef CSMOD_USE_FFTW
