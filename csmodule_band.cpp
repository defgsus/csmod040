#include "math.h"
#include "csmodule_band.h"

char *CSmodule_Band::docString()
{
	const char *r = "\
this module lets you track the amplitude of a certain frequency band of an input signal.\n\n\
the signal is provided in <input>.\n\
<freq> sets the frequency of the band to analyze.\n\n\
internally it's a goertzel 2nd order filter to calculate one DFT band of an input signal. a small \
buffer is used to catch enough input data. note that the outputs of two band detectors of the same frequency band \
might variate a little bit for the same input signal due to probably different buffer window positions \
which leads to different input signal phases.\n\n\
this module is quite cheap in computational effort and can be used a lot of times.\
";
	return const_cast<char*>(r);
}

CSmodule_Band::CSmodule_Band()
    :   buf(0)
{ }

CSmodule_Band::~CSmodule_Band()
{
	if (buf) free(buf);
}

CSmodule_Band* CSmodule_Band::newOne()
{
	return new CSmodule_Band();
}


void CSmodule_Band::init()
{
	gname = copyString("analyze");
	bname = copyString("band detector");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_freq = &(createInput("f","freq",100.0)->value);

	_out = &(createOutput("o","output")->value);

	// buffer

	bufpos = 0;
	buflen = 512;
	buf = (csfloat*) calloc(buflen, sizeof(csfloat));

	// ---- properties ----

	createNameProperty();

}

void CSmodule_Band::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_Band::step()
{
	// fill buffer
    if (bufpos<buflen)
    {
    	buf[bufpos] = *_in;
    	bufpos++;
    	return;
    }
    bufpos = 0;

	// get the band info
    /** FROM: http://www.musicdsp.org/archive.php?classid=2#107 */
    csfloat Skn, Skn1, Skn2;
    Skn = Skn1 = Skn2 = 0;
    csfloat ph = TWO_PI * *_freq*isampleRate;
    csfloat co = 2.0*cos( ph );

    for (int i=0; i<buflen; i++) {
		Skn2 = Skn1;
		Skn1 = Skn;
		Skn = co*Skn1 - Skn2 + buf[i];
    }

    // fade, scale and abs
    *_out += 0.1*(fabs(Skn - exp(-ph)*Skn1)/buflen*2.0 - *_out);


}

