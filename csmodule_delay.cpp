#include <algorithm>
#include "math.h"
#include "csmodule_delay.h"

char *CSmodule_Delay::docString()
{
	const char *r = "\
this module implements a sub-sample accurate delay line.\n\n\
the sample in <input> will be stored in the delay buffer with the given <amplitude> (so changing \
the amplitude affects the input buffer FIRST, and changes to the output of the delay will be \
noticeable only AFTER the delay time).\n\
the delay time can be given in <samples> or in <seconds>. both values are added together internally.\n\n\
the maximum possible delay time can be set in the property window.\n\n\
currently sub-sample buffer read-outs are interpolated linearly.\
";
	return const_cast<char*>(r);
}

CSmodule_Delay::~CSmodule_Delay()
{
	if (buf) free(buf);
}

CSmodule_Delay* CSmodule_Delay::newOne()
{
	return new CSmodule_Delay();
}


void CSmodule_Delay::init()
{
	gname = copyString("delay");

	bname = copyString("delay");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);
	_sam = &(createInput("sam","samples")->value);
	_sec = &(createInput("sec","seconds",0.25)->value);
	_rst = &(createInput("rst","reset buffer")->value);
	lrst = 0;

	_out = &(createOutput("out","output")->value);

	// -- intern --

	buf = 0;
	setBuf(10.0);

	quality = 1;

	// ---- properties ----

	createNameProperty();
	createPropertyFloat("nrsec","max. delay length (sec)", buflensec, 0.1, 100.0);
	createPropertyInt("qual","quality (0,1)", quality, 0,1);

}

void CSmodule_Delay::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrsec")==0)
		setBuf(prop->fvalue);
	else
	if (strcmp(prop->name,"qual")==0)
		quality = prop->ivalue;
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Delay::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);
	setBuf(buflensec);
}


void CSmodule_Delay::setBuf(float sec)
{
	CSwait();
	ready = false;

	buflensec = sec;

	if (buf) free(buf);
    buflen = std::max(4, (int)(sec*sampleRate));
	buf = (csfloat*) calloc(buflen, sizeof(csfloat));
	bufpos = 0;

	ready = true;
}


void CSmodule_Delay::step()
{
	// store
	buf[bufpos] = *_amp * *_in;

	// clear buffer
	if ((lrst<=0)&&(*_rst>0.0))
		memset(buf, 0, buflen*sizeof(float));
	lrst=*_rst;

	// calc buf position
	csfloat d = min((csfloat)buflen-1, max((csfloat)1, *_sam + *_sec * sampleRate ));
	int dt = (int)d;

	// read
	*_out = buf[ (bufpos-dt+buflen) % buflen ];

	if (quality==1)
	{
		// linear interpolate
		d -= dt;
		*_out = *_out * d + (1-d) *
			buf[ (bufpos-dt+1+buflen) % buflen ];
	}

	// inc write pointer
	bufpos++; if (bufpos>=buflen) bufpos = 0;

}











char *CSmodule_DelaySam::docString()
{
	const char *r = "\
this module serves as a simple non-interpolated sample delay, especially to introduce a sample accurate \
latency to sync with other modules in the module chain.\n\n\
the thing in <input> will be delayed for the number of samples as given in <samples>. however, as any module \
the delay (sample) module in itself introduces a one sample delay.\
";
	return const_cast<char*>(r);
}

CSmodule_DelaySam::~CSmodule_DelaySam()
{
	if (buf) free(buf);
}

CSmodule_DelaySam* CSmodule_DelaySam::newOne()
{
	return new CSmodule_DelaySam();
}


void CSmodule_DelaySam::init()
{
	gname = copyString("delay");

	bname = copyString("delay (sample)");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_sam = &(createInput("sam","samples",1)->value);

	_out = &(createOutput("out","output")->value);

	// -- intern --

	buf = 0;
	setBuf(sampleRate);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrsam","max. delay length (sampl)", sampleRate, 1, 44100*16);

}

void CSmodule_DelaySam::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrsam")==0)
		setBuf(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_DelaySam::setBuf(int sam)
{
	CSwait();
	ready = false;

	if (buf) free(buf);
	buflen = max(4, sam);
	//printf("%s buflen: %d\n",name,buflen);
	buf = (csfloat*) calloc(buflen, sizeof(csfloat));
	bufpos = 0;

	ready = true;
}
//kerstin.vater@verdi.de
//1010509782

void CSmodule_DelaySam::step()
{
	// store input
	buf[bufpos] = *_in;

	// get pos
	int dt = min(buflen-1, max(0, (int)*_sam ));

	// read
	*_out = buf[ (bufpos-dt+buflen) % buflen ];

	// inc pointer
	bufpos++; if (bufpos>=buflen) bufpos = 0;

}


















char *CSmodule_DelaySpread::docString()
{
	const char *r = "\
this module implements a sub-sample accurate multi-tap delay line. it can be used for hall or reverb effects.\n\n\
the sample in <input> will be stored in the delay buffer with the given <amplitude>.\n\
the delay time can be given in <samples> or in <seconds>. both values are added together internally.\n\n\
the maximum possible delay time can be set in the property window. \
currently sub-sample buffer read-outs are interpolated linearly.\n\n\
<spread nr> sets the number of delays that will be put through to the output. 20 should be enough in most \
cases, 100 gets computationally quite intense.\n\
<spread mul> is multiplied with the delay time after each multi-tap read-out.\n\
<spread add> is added to the delay time after each read-out. together with <spread mul> you can define \
the character of the delay/reverb.\n\
as an example: <seconds> = 0.1, <spread mul> = 2.0, <spread add> = 0.1, that means the first delay read-out will \
be 0.1 sec later than the input, the second read-out will take place at 0.2 seconds, since the <spread add> value \
is added to the delay pointer which has started with 0.0. the third read-out is at 0.5, since the position which \
was 0.2 is multiplied with <spread mul> and <spread add> is added.\n\
note that the amplitude of the output is divided by the <spread nr> value.\n\n\
the spread delay is not the way to build high quality reverbs but with the right settings it can sound quite nice \
and is not too time consuming.\
";
	return const_cast<char*>(r);
}

CSmodule_DelaySpread* CSmodule_DelaySpread::newOne()
{
	return new CSmodule_DelaySpread();
}


void CSmodule_DelaySpread::init()
{
	CSmodule_Delay::init();

	free(bname); bname = copyString("delay spread");
	free(name); name = copyString(bname);
	rename(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_spra = &(createInput("snr","spread nr",10.0)->value);
	_spr = &(createInput("spr","spread time mul",1.1)->value);
	_sprl = &(createInput("spl","spread time add",0.01)->value);

}


void CSmodule_DelaySpread::step()
{
	// keep track of buffer data
	buf[bufpos] = *_amp * *_in;

	csfloat
		o,d,p;

	// compose output
	*_out = 0;

	*_spra = max((csfloat)1, *_spra);
	p = 0.0;
	int nr=*_spra;
	for (int i=0;i<nr;i++)
	{
		// determine read position
		d = min((csfloat)buflen-1, max((csfloat)1,
			*_sam + (*_sec + p) * sampleRate ));
		int dt = (int)d;

		// read
		o = buf[ (bufpos-dt+buflen) % buflen ];

		// interpolate/read
		d -= dt;
		o = o * d + (1-d) *
			buf[ (bufpos-dt+1+buflen) % buflen ];

		// add to output
		*_out += o;

		p = p * *_spr + *_sprl;
	}
	// normalize
	*_out /= *_spra;


	// inc write pointer
	bufpos++; if (bufpos>=buflen) bufpos = 0;

}

















char *CSmodule_DelayOneSample::docString()
{
	const char *r = "\
this module does nothing, it simply passes the input through to the output. it's used \
as a cheap 1-sample delay.\
";
	return const_cast<char*>(r);
}

CSmodule_DelayOneSample* CSmodule_DelayOneSample::newOne()
{
	return new CSmodule_DelayOneSample();
}


void CSmodule_DelayOneSample::init()
{
	gname = copyString("delay");
	bname = copyString("delay1 (1 sample)");
	name = copyString("delay (1 sample)");
	uname = copyString(name);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

}


void CSmodule_DelayOneSample::step()
{
	*_out = *_in;
}















char *CSmodule_RoomDelay::docString()
{
	const char *r = "\
this module implements a sub-sample accurate delay line.\n\n\
the sample in <input> will be stored in the delay buffer with the given <amplitude> (so changing \
the amplitude affects the input buffer FIRST, and changes to the output of the delay will be \
noticeable only AFTER the delay time).\n\
the delay time can be given in <samples> or in <seconds>. both values are added together internally.\n\n\
the maximum possible delay time can be set in the property window.\n\n\
currently sub-sample buffer read-outs are interpolated linearly.\
";
	return const_cast<char*>(r);
}

CSmodule_RoomDelay::~CSmodule_RoomDelay()
{
	if (buf) free(buf);
}

CSmodule_RoomDelay* CSmodule_RoomDelay::newOne()
{
	return new CSmodule_RoomDelay();
}


void CSmodule_RoomDelay::init()
{
	gname = copyString("delay");

	bname = copyString("delay");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);
	_X = &(createInput("x","X",0.0)->value);
	_Y = &(createInput("y","Y",0.0)->value);

	_refl = &(createInput("r","reflection",0.1)->value);
	_sizeX = &(createInput("sx","room size X",2.0)->value);
	_sizeY = &(createInput("sy","room size Y",2.0)->value);
	_sos = &(createInput("sos","speed of sound")->value);

	_A = &(createOutput("a","rear left")->value);
	_B = &(createOutput("b","rear right")->value);
	_C = &(createOutput("c","front right")->value);
	_D = &(createOutput("d","front left")->value);

	// -- intern --

	buf = 0;
	setBuf(10.0);

	// ---- properties ----

	createNameProperty();
	createPropertyFloat("nrsec","max. delay length (sec)", 10.0, 0.1, 100.0);

}

void CSmodule_RoomDelay::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrsec")==0)
		setBuf(prop->fvalue);
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_RoomDelay::setBuf(float sec)
{
	CSwait();
	ready = false;

	if (buf) free(buf);
    buflen = std::max(4, (int)(sec*sampleRate));
	buf = (csfloat*) calloc(buflen, sizeof(csfloat));
	bufpos = 0;

	ready = true;
}


void CSmodule_RoomDelay::step()
{



	/*
	// store
	buf[bufpos] = *_amp * *_in;

	// calc buf position
	csfloat d = min((csfloat)buflen-1, max((csfloat)1, *_sam + *_sec * sampleRate ));
	int dt = (int)d;

	// read
	*_out = buf[ (bufpos-dt+buflen) % buflen ];

	// linear interpolate
	d -= dt;
	*_out = *_out * d + (1-d) *
		buf[ (bufpos-dt+1+buflen) % buflen ];

	// inc write pointer
	bufpos++; if (bufpos>=buflen) bufpos = 0;
	*/
}












char *CSmodule_Reverb::docString()
{
	const char *r = "\
this is a reverb module (in it's *experimental* stage).\n\n\
the <input> is delayed for <predelay> seconds and then reverberated with a reverb time span of <length> seconds.\n\n\
the mix between original and reverbed signal is set in <wet> (limited to 0.0 to 1.0).\n\
the <amplitude> multiplies the amplitude of the reverbed signal.\n\n\
<lowpass freq> is in hz and applies a low pass filter on the reverbed signal, IF the \
value in <lowpass freq> is >= 0.0. else the signal remains unfiltered.\n\
<feedback> is a multiplier to send back the filtered reverb signal into the delay line.\n\n\
a gate in <reset buffer> empties the whole delay buffer.\n\n\
properties:\n\
<max. delay> sets the maximum possible delay time in 2^n samples. so f.e. 16 would mean a delay of 16385 samples.\n\
<nr of reflections> sets the number of different delay positions to add to the reverb output. \
this number shouldn't be too high to save computation time (about 50-80 is feasable).\n\n\
<time curve> is the exponent of the time spread. 1.0 is completely linear and above 1.0 is exponential.\n\
<am curve> is the same as <time curve> only for the amplitude of each delay position. \n\
both values are best set by try and error.\n\n\
<time offset curve> is a modulation on the individual delay positions. 0.0 = no modulation, everything else \
modulates the positions to form small clusters. ( *experimental value*, probably subject to change )\
";
	return const_cast<char*>(r);
}

CSmodule_Reverb::~CSmodule_Reverb()
{
	if (buf) free(buf);
	if (revamp) free(revamp);
	if (revpos) free(revpos);
}

CSmodule_Reverb* CSmodule_Reverb::newOne()
{
	return new CSmodule_Reverb();
}


void CSmodule_Reverb::init()
{
	gname = copyString("delay");

	bname = copyString("reverb");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_sec = &(createInput("sec","predelay (sec)",0.04)->value);
	_len = &(createInput("len","length (sec)",1.0)->value);
	_wet = &(createInput("wet","dry/wet",0.5)->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);
	_freq = &(createInput("f","lowpass freq",10000.0)->value);
	_fb = &(createInput("fb","feedback",0.2)->value);
	_rst = &(createInput("rst","reset buffer")->value);

	_out = &(createOutput("out","output")->value);

	// -- intern --

	buf = 0;
	setBuf(18);
	revamp = revpos = 0;
	revtfac = 2.0;
	revafac = 3.0;
	revofac = 0.0;
	setRev(48);

	lrst = 0;
	lfreq = -1;
	sam = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("dlylen","max. delay (2^n samples)", 18, 1, 24.0);
	createPropertyInt("revlen","nr of reflections", revlen, 1, 1000.0);
	createPropertyFloat("revtf","time curve (t^n)", revtfac, 0.0, 100.0);
	createPropertyFloat("revof","time offset curve (sin(t*n))", revofac, 0.0, 100.0);
	createPropertyFloat("revaf","amp curve (t^n)", revafac, 0.0, 100.0);
}

void CSmodule_Reverb::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"dlylen")==0)
		setBuf(prop->ivalue);
	else
	if (strcmp(prop->name,"revlen")==0)
		setRev(prop->ivalue);
	else
	if (strcmp(prop->name,"revtf")==0)
	{
		revtfac = prop->fvalue;
		setRev(revlen);
	}
	else
	if (strcmp(prop->name,"revaf")==0)
	{
		revafac = prop->fvalue;
		setRev(revlen);
	}
	else
	if (strcmp(prop->name,"revof")==0)
	{
		revofac = prop->fvalue;
		setRev(revlen);
	}
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_Reverb::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);
	revamp = revpos = 0;
	revtfac = 2.0;
	revafac = 3.0;
	revofac = 0.0;
	setRev(48);

	lrst = 0;
	lfreq = -1;
	sam = coeff1 = 0.0;
	//*_out = 0.0; // TODO outs von allen modulen

}

void CSmodule_Reverb::setBuf(int po)
{
	CSwait();
	ready = false;

	if (buf) free(buf);
    buflen = std::max(4, (int)pow(2, po));
	buflenand = buflen-1;
	buf = (csfloat*) calloc(buflen, sizeof(csfloat));
	bufpos = 0;

	ready = true;
}

void CSmodule_Reverb::setRev(int nr)
{
	CSwait();
	bool oReady = ready;
	ready = false;

	revlen = max(1, nr);
	if (revamp) free(revamp);
	revamp = (csfloat*) calloc(revlen, sizeof(csfloat));
	if (revpos) free(revpos);
	revpos = (csfloat*) calloc(revlen, sizeof(csfloat));

	csfloat
		*ra = revamp,
		*rp = revpos;
	for (int i=0;i<revlen;i++)
	{
		*rp = (csfloat)i / revlen;
		*rp += sinf((csfloat)i/revlen*PI*revofac) / revlen;
		*rp = pow(*rp, revtfac);

		*ra = (csfloat)i / revlen;
		*ra = pow(1.0 - *ra, revafac);
		if (i&&1) *ra = -*ra;

		rp++; ra++;
	}

	// normalize
	ra = revamp;
	csfloat ma = 0;
	for (int i=0;i<revlen;i++)
	{
		ma += fabs(*ra);
		ra++;
	}
	ma = (csfloat)1.0 / max((csfloat)0.00001,ma);
	ra = revamp;
	for (int i=0;i<revlen;i++)
		*ra++ *= ma;

	ready = oReady;
}


void CSmodule_Reverb::step()
{
	// store input & feedback
	buf[bufpos] = *_in + *_fb * sam;

	// calc delay buf position
	int dt = min(buflen-1, max(0, (int)(*_sec * sampleRate) ));

	// cutoff coefficient
	if (lfreq!=*_freq)
	{
		if (*_freq<0)
			coeff1 = 1.0;
		else
			coeff1 =
                std::max((csfloat)0, min((csfloat)2,
                    (csfloat)2 * *_freq * isampleRate
				));
	}
	lfreq = *_freq;


	// --- gather delays ---

	csfloat samb = 0.0;

	int po = bufpos+(buflen<<4)-dt;
	csfloat
		*ra = revamp,
		*rp = revpos;
	for (int i=0;i<revlen;i++)
	{
		samb += *ra * buf[
			(int)(po - *_len * *rp * sampleRate ) & buflenand
			];
		ra++;
		rp++;
	}

	// filter
	sam += coeff1*(samb-sam);

	// mix reverb
	*_wet = max((csfloat)0, min((csfloat)1, *_wet ));
	*_out = (1.0 - *_wet) * *_in + *_wet * *_amp * sam;


	// inc write pointer
	bufpos++; if (bufpos>=buflen) bufpos = 0;


	// clear buffer
	if ((lrst<=0)&&(*_rst>0.0))
	{
		memset(buf, 0, buflen*sizeof(float));
		*_out = 0.0;
		coeff1 = sam = samb = 0.0;
		lfreq = *_freq-1; // force recalc
	}
	lrst=*_rst;

}
