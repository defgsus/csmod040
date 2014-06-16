/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: FAST FOURIER TRANSFORM
 *
 * PURPOSE:
 * analyze data and supply the results
 *
 * @note
 *		uses FFTW library
 *
 * @version 2010/10/17 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_FFT_H_INCLUDED
#define CSMODULE_FFT_H_INCLUDED

#ifdef CSMOD_USE_FFTW

#include "fftw3.h"
#include "csmodule.h"


class CSmodule_FFT: public CSmodule
{
	// pointers for easy value access
	csfloat
		*_in,
		*_amp,
		*_fadeUp,
		*_fadeDown,
		*_rangeStart,
		*_rangeEnd,
		*_out[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__out[CSMOD_MAX_CHAN];

	int
		nrOut,
		oldNrOut,
		bufferSize,
		bufferPos,
		dispSize,
		// view port
		sx,sy,sw,sh,
		minh;

	float
		/** fft input */
		*inp,
		/** fft output */
		*outp,
		/** display buffer */
		*disp;

	fftwf_plan
		plan;

	public:
	char *docString();
	~CSmodule_FFT();
	CSmodule_FFT* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrIn);
	void setBufferSize(int newBufferSize);

	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);

};

#endif // #ifdef CSMOD_USE_FFTW

#endif // CSMODULE_FFT_H_INCLUDED

