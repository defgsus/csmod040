#include <algorithm>
#include "math.h"
#include "csmodule_osc.h"

char *CSmodule_Osc::docString()
{
	const char *r = "\
the module supports the common oscillator types SINE, SQUARE, SAW (or RAMP) and TRIANGLE wave.\n\n\
<freq> will set the frequency in hertz. it can also be negative which will reverse the \
wave-forms in time. note that only the outputs which are connected will be calculated. \
the scope display of the module shows the first connected output waveform.\n\n\
<phase> is the phase offset (between 0.0 and 1.0 for one period).\n\
<amplitude> sets the maximum amplitude of the output (from -amplitude to amplitude).\n\
<offset> will be added to the output.\n\
a positive edge on <sync> will restart the phase/period (hard sync).\n\
<pulsewidth> is used only by the SQUARE and TRIANGLE wave generator and determines \
the length of the positive edge between 0.0 and 1.0.\n\n\
the SAW wave will travel from -1.0 to 1.0 as with the other wave-forms too. to get a \
standard RAMP wave (between 0.0 and 1.0), set <amplitude> and <offset> to 0.5 each.\
";	return const_cast<char*>(r);
}

CSmodule_Osc::CSmodule_Osc()
    :   CSmodule(), scope(0)
{

}

CSmodule_Osc::~CSmodule_Osc()
{
    if (scope)
        releaseTimeline(scope);
}

CSmodule_Osc* CSmodule_Osc::newOne()
{
	return new CSmodule_Osc();
}


void CSmodule_Osc::init()
{
	gname = copyString("oscillator");

	bname = copyString("oscillator");
	name = copyString(bname);
	uname = copyString(bname);

	// ---- look ----

	headerHeight = 70;
	// scope rectangle
	sx = 3; sy = 17;
	sw = width-6; sh = headerHeight - sy - 4;

	// --- inputs outputs ----

	_freq = &(createInput("f","freq (hz)",1.0)->value);
	_phase = &(createInput("p","phase")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_rst = &(createInput("rst","sync")->value);
	_pulse = &(createInput("pw","pulsewidth",0.5)->value);

	o_sine = createOutput("osin","sine"); _outSine = &(o_sine->value);
	o_cosine = createOutput("ocos","cosine"); _outCosine = &(o_cosine->value);
	o_square = createOutput("osqr","square"); _outSquare = &(o_square->value);
	o_saw = createOutput("osaw","saw"); _outSaw = &(o_saw->value);
	o_tri = createOutput("otri","triangle"); _outTri = &(o_tri->value);

	// ---- properties ----

	createNameProperty();

	// --- internals ---

	scope = createTimeline(sw);
	phase = 0.0;
	lrst = 0.0;

}


void CSmodule_Osc::step()
{
	bool rec=false;

	// phase reset
	if ( (lrst<=0) && ( *_rst>0 ) ) phase = 0.0;
	lrst = *_rst;

	CSclipFloat(phase);

	// phase modulation
	csfloat p = phase + *_phase;

	// sine output
	if (o_sine->nrCon)
	{
		*_outSine = *_off + *_amp * sin(p * TWO_PI);
		// store sine value for scope display if output active
		recordTimeline(scope, *_outSine);
		rec = true;
	}
	// cosine output
	if (o_cosine->nrCon)
	{
		*_outCosine = *_off + *_amp * cos(p * TWO_PI);
		// store cosine value for scope display if output active
		if (!rec) recordTimeline(scope, *_outCosine);
		rec = true;
	}

	if ((o_square->nrCon||o_saw->nrCon||o_tri->nrCon))
{

	// on positive phase
	if (p>=0)
	{
		p = MODULO(p,1.0);

		// square output
		if (o_square->nrCon)
		{
			*_outSquare = *_off + ( (p > *_pulse)? -*_amp : *_amp );
			if (!rec) { recordTimeline(scope, *_outSquare); rec=true; }
		}

		// saw output
		if (o_saw->nrCon)
		{
			*_outSaw = *_off + *_amp * 2.0 * ( p - 0.5 );
			if (!rec) { recordTimeline(scope, *_outSaw); rec=true; }
		}

		// tri output
		if (o_tri->nrCon)
		{
			csfloat pw = max((csfloat)0.00001,min((csfloat)0.99999, *_pulse ));
			if (p<pw)
				*_outTri = *_off + *_amp * 2.0 * ( 0.5 - (pw-p)/pw );
			else
				*_outTri = *_off + *_amp * 2.0 * ( (1.0-p)/(1.0-pw) - 0.5 );

			if (!rec) recordTimeline(scope, *_outTri);
		}
	}

	// on negative phase (on neg. phase offset or neg. frequency)
	else
	{
		p = MODULO(-p,1.0);
		// square output
		if (o_square->nrCon)
		{
			*_outSquare = *_off + ( (p > 1.0 - *_pulse)? *_amp : -*_amp );
			if (!rec) { recordTimeline(scope, *_outSquare); rec=true; }
		}

		// saw output
		if (o_saw->nrCon)
		{
			*_outSaw = *_off - *_amp * 2.0 * ( p - 0.5 );
			if (!rec) { recordTimeline(scope, *_outSaw); rec=true; }
		}

		// tri output
		if (o_tri->nrCon)
		{
			csfloat pw = 1.0 - max((csfloat)0.00001,min((csfloat)0.99999, *_pulse ));
			if (p<pw)
				*_outTri = *_off + *_amp * 2.0 * ( 0.5 - (pw-p)/pw );
			else
				*_outTri = *_off + *_amp * 2.0 * ( (1.0-p)/(1.0-pw) - 0.5 );

			if (!rec) recordTimeline(scope, *_outTri);
		}

	}
}// finish do other osc outputs

	// phase increment
	phase += *_freq * isampleRate;
	// wrap phase around to avoid overflow
	if (phase>1.0) phase-=1.0;
	else
	if (phase<-1.0) phase+=1.0;
}










CSmodule_OscPOut* CSmodule_OscPOut::newOne()
{
	return new CSmodule_OscPOut();
}


void CSmodule_OscPOut::init()
{
	CSmodule_Osc::init();
	gname = copyString("oscillator");

	free(bname); bname = copyString("osc phaseout");
	free(name); name = copyString(bname);
	rename(bname);

	// --- inputs outputs ----

	_outPhase = &createOutput("ophase","phase")->value;

}


void CSmodule_OscPOut::step()
{
	bool rec=false;

	// phase reset
	if ( (lrst<=0) && ( *_rst>0 ) ) phase = 0.0;
	lrst = *_rst;

	CSclipFloat(phase);

	// phase modulation
	csfloat p = phase + *_phase;

	// sine output
	if (o_sine->nrCon)
	{
		*_outSine = *_off + *_amp * sin(p * TWO_PI);
		// store sine value for scope display if output active
		recordTimeline(scope, *_outSine);
		rec = true;
	}
	// cosine output
	if (o_cosine->nrCon)
	{
		*_outCosine = *_off + *_amp * cos(p * TWO_PI);
		// store cosine value for scope display if output active
		if (!rec) recordTimeline(scope, *_outCosine);
		rec = true;
	}

	if ((o_square->nrCon||o_saw->nrCon||o_tri->nrCon))
{

	// on positive phase
	if (p>=0)
	{
		p = MODULO(p,1.0);

		// square output
		if (o_square->nrCon)
		{
			*_outSquare = *_off + ( (p > *_pulse)? -*_amp : *_amp );
			if (!rec) { recordTimeline(scope, *_outSquare); rec=true; }
		}

		// saw output
		if (o_saw->nrCon)
		{
			*_outSaw = *_off + *_amp * 2.0 * ( p - 0.5 );
			if (!rec) { recordTimeline(scope, *_outSaw); rec=true; }
		}

		// tri output
		if (o_tri->nrCon)
		{
			csfloat pw = max((csfloat)0.00001,min((csfloat)0.99999, *_pulse ));
			if (p<pw)
				*_outTri = *_off + *_amp * 2.0 * ( 0.5 - (pw-p)/pw );
			else
				*_outTri = *_off + *_amp * 2.0 * ( (1.0-p)/(1.0-pw) - 0.5 );

			if (!rec) recordTimeline(scope, *_outTri);
		}
	}

	// on negative phase (on neg. phase offset or neg. frequency)
	else
	{
		p = MODULO(-p,1.0);
		// square output
		if (o_square->nrCon)
		{
			*_outSquare = *_off + ( (p > 1.0 - *_pulse)? *_amp : -*_amp );
			if (!rec) { recordTimeline(scope, *_outSquare); rec=true; }
		}

		// saw output
		if (o_saw->nrCon)
		{
			*_outSaw = *_off - *_amp * 2.0 * ( p - 0.5 );
			if (!rec) { recordTimeline(scope, *_outSaw); rec=true; }
		}

		// tri output
		if (o_tri->nrCon)
		{
			csfloat pw = 1.0 - max((csfloat)0.00001,min((csfloat)0.99999, *_pulse ));
			if (p<pw)
				*_outTri = *_off + *_amp * 2.0 * ( 0.5 - (pw-p)/pw );
			else
				*_outTri = *_off + *_amp * 2.0 * ( (1.0-p)/(1.0-pw) - 0.5 );

			if (!rec) recordTimeline(scope, *_outTri);
		}

	}
}// finish do other osc outputs

	*_outPhase = phase;

	// phase increment
	phase += *_freq * isampleRate;
	// wrap phase around to avoid overflow
	if (phase>1.0) phase-=1.0;
	else
	if (phase<-1.0) phase+=1.0;
}











char *CSmodule_OscP::docString()
{
	const char *r = "\
the module supports the common oscillator types SINE, SQUARE, SAW (or RAMP) and TRIANGLE wave. \
it is the same module as the original oscillator, except that it runs for a certain number of periods.\n\n\
<freq> will set the frequency in hertz. it can also be negative which will reverse the \
wave-forms in time. note that only the outputs which are connected will be calculated. \
the scope display of the module shows the first connected output waveform.\n\n\
<phase> is the phase offset (between 0.0 and 1.0 for one period).\n\
<amplitude> sets the maximum amplitude of the output (from -amplitude to amplitude).\n\
<offset> will be added to the output.\n\
a positive edge on <sync> will restart the phase/period (hard sync).\n\
<pulsewidth> is used only by the SQUARE and TRIANGLE wave generator and determines \
the length of the positive edge between 0.0 and 1.0.\n\n\
the SAW wave will travel from -1.0 to 1.0 as with the other wave-forms too. to get a \
standard RAMP wave (between 0.0 and 1.0), set <amplitude> and <offset> to 0.5 each.\n\n\
<nr periods> sets the number of periods of the oscillator before it stops. \
a value of 0 means unlimited, a value greater 0 means: run for the number of periods and then \
stay there until the next gate in <sync>.\
";	return const_cast<char*>(r);
}

CSmodule_OscP* CSmodule_OscP::newOne()
{
	return new CSmodule_OscP();
}


void CSmodule_OscP::init()
{
	CSmodule_Osc::init();
	free(bname); bname = copyString("oscillator (period)");
	free(name); name = copyString(bname);
	rename(bname);

	// --- inputs outputs ----

	_nrPer = &(createInput("nrp","nr periods",1.0)->value);
	nrPer = 0;
}


void CSmodule_OscP::step()
{
	bool rec=false;

	// phase reset
	if ( (lrst<=0) && ( *_rst>0 ) ) { phase = 0.0; nrPer = 0; }
	lrst = *_rst;

	CSclipFloat(phase);

	// phase modulation
	csfloat p = phase + *_phase;

	// sine output
	if (o_sine->nrCon)
	{
		*_outSine = *_off + *_amp * sin(p * TWO_PI);
		// store sine value for scope display if output active
		recordTimeline(scope, *_outSine);
		rec = true;
	}
	// cosine output
	if (o_cosine->nrCon)
	{
		*_outCosine = *_off + *_amp * cos(p * TWO_PI);
		// store cosine value for scope display if output active
		if (!rec) recordTimeline(scope, *_outCosine);
		rec = true;
	}

	if ((o_square->nrCon||o_saw->nrCon||o_tri->nrCon))
{

	// on positive phase
	if (p>=0)
	{
		p = MODULO(p,1.0);

		// square output
		if (o_square->nrCon)
		{
			*_outSquare = *_off + ( (p > *_pulse)? -*_amp : *_amp );
			if (!rec) { recordTimeline(scope, *_outSquare); rec=true; }
		}

		// saw output
		if (o_saw->nrCon)
		{
			*_outSaw = *_off + *_amp * 2.0 * ( p - 0.5 );
			if (!rec) { recordTimeline(scope, *_outSaw); rec=true; }
		}

		// tri output
		if (o_tri->nrCon)
		{
			csfloat pw = max((csfloat)0.00001,min((csfloat)0.99999, *_pulse ));
			if (p<pw)
				*_outTri = *_off + *_amp * 2.0 * ( 0.5 - (pw-p)/pw );
			else
				*_outTri = *_off + *_amp * 2.0 * ( (1.0-p)/(1.0-pw) - 0.5 );

			if (!rec) recordTimeline(scope, *_outTri);
		}
	}

	// on negative phase (on neg. phase offset or neg. frequency)
	else
	{
		p = MODULO(-p,1.0);
		// square output
		if (o_square->nrCon)
		{
			*_outSquare = *_off + ( (p > 1.0 - *_pulse)? *_amp : -*_amp );
			if (!rec) { recordTimeline(scope, *_outSquare); rec=true; }
		}

		// saw output
		if (o_saw->nrCon)
		{
			*_outSaw = *_off - *_amp * 2.0 * ( p - 0.5 );
			if (!rec) { recordTimeline(scope, *_outSaw); rec=true; }
		}

		// tri output
		if (o_tri->nrCon)
		{
			csfloat pw = 1.0 - max((csfloat)0.00001,min((csfloat)0.99999, *_pulse ));
			if (p<pw)
				*_outTri = *_off + *_amp * 2.0 * ( 0.5 - (pw-p)/pw );
			else
				*_outTri = *_off + *_amp * 2.0 * ( (1.0-p)/(1.0-pw) - 0.5 );

			if (!rec) recordTimeline(scope, *_outTri);
		}

	}
}// finish do other osc outputs

	// phase increment
	if ((*_nrPer==0)||(nrPer<*_nrPer))
	{
		phase += *_freq * isampleRate;
		// wrap phase around to avoid overflow
		if (phase>1.0)
		{
			phase-=1.0;
			nrPer++;
		}
		else
		if (phase<-1.0)
		{
			phase+=1.0;
			nrPer++;
		}
	}
}












/* 94ms per sam */
char *CSmodule_OscLU::docString()
{
	const char *r = "\
the module supports the common oscillator types SINE, SQUARE, SAW (or RAMP) and TRIANGLE wave. \
the wave forms are precalculated in a lookup table, so this module is a bit faster as the \
default oscillator (especially the sine function) but the bandwidth is not optimal.\n\n\
<freq> will set the frequency in hertz. it can also be negative which will reverse the \
wave-forms in time.\n\
<phase> is the phase offset (between 0.0 and 1.0 for one period).\
<amplitude> sets the maximum amplitude of the output (from -amplitude to amplitude).\n\
<offset> will be added to the output.\n\
a positive edge on <sync> will restart the phase/period (hard sync).\n\
the SAW wave will travel from -1.0 to 1.0 as with the other wave-forms too. to get a \
standard RAMP wave (between 0.0 and 1.0), set <amplitude> and <offset> to 0.5 each.\
";	return const_cast<char*>(r);
}


CSmodule_OscLU* CSmodule_OscLU::newOne()
{
	return new CSmodule_OscLU();
}


void CSmodule_OscLU::init()
{
	gname = copyString("oscillator");

	bname = copyString("oscillator table");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_freq = &(createInput("f","freq (hz)",1.0)->value);
	_phase = &(createInput("p","phase")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_rst = &(createInput("rst","sync")->value);

	o_sine = createOutput("osin","sine"); _outSine = &(o_sine->value);
	o_square = createOutput("osqr","square"); _outSquare = &(o_square->value);
	o_saw = createOutput("osaw","saw"); _outSaw = &(o_saw->value);
	o_tri = createOutput("otri","triangle"); _outTri = &(o_tri->value);

	// ---- properties ----

	createNameProperty();

	// --- internals ---

	phase = 0.0;
	lrst = 0.0;

	// calc tables
	for (int i=0;i<CSMOD_OSC_TABLE;i++)
	{
		csfloat t = (csfloat)i/CSMOD_OSC_TABLE;
		w_sin[i] = sin(t*TWO_PI);
		w_sqr[i] = (t<0.5)? -1.0 : 1.0;
		w_saw[i] = (t-0.5)*2.0;
		w_tri[i] = (t<0.5)? t*4.0-1.0 : (0.75-t)*4.0;
		/*
		int n = 4;
		w_saw[i] = 0.0;
		for (int j=0;j<n;j++)
		{
			csfloat a = (csfloat)j/n;
			w_saw[i] += (1.0-a)*sin(t*pow(2,j)*TWO_PI);
		}
		*/
	}

}


void CSmodule_OscLU::step()
{
	// phase reset
	if ( (lrst<=0) && ( *_rst>0 ) ) phase = 0.0;
	lrst = *_rst;

	CSclipFloat(phase);

	// phase modulation
	csfloat p = phase + *_phase;

	// integer phase
	unsigned int po = (unsigned int)(p*CSMOD_OSC_TABLE) & (CSMOD_OSC_TABLE-1);

	// outputs
	*_outSine = *_off + *_amp * w_sin[po];
	*_outSquare = *_off + *_amp * w_sqr[po];
	*_outSaw = *_off + *_amp * w_saw[po];
	*_outTri = *_off + *_amp * w_tri[po];

	// phase increment
	phase += *_freq * isampleRate;
	// wrap phase around to avoid overflow
	if (phase>1.0) phase-=1.0;
	else
	if (phase<-1.0) phase+=1.0;
}










void CSmodule_Osc::draw(int offx, int offy, float zoom)
{
	// draw original module
	CSmodule::draw(offx,offy,zoom);

	// draw scope
	fl_color(headerFocusColorB);
	drawTimeline(scope, offx+zoom*(x+sx),offy+zoom*(y+sy),zoom*sw,zoom*sh, true, headerColorD);
}

















char *CSmodule_OscSpec::docString()
{
	const char *r = "\
the module outputs an additive sine type waveform composed of several sine waves (up to %d).\n\n\
<freq> will set the base frequency in hertz. it can also be negative which will reverse the \
wave-forms in time.\n\
<phase> is the phase offset (between 0.0 and 1.0 for one period).\
<amplitude> sets the maximum amplitude of each single sinewave (from -amplitude to amplitude). \
note that the output amplitude is the sum of all single voices.\n\
<offset> will be added to the output.\n\
a positive edge on <sync> will restart the phase/period (hard sync).\n\
<nr voices> sets the number of spectral voices (sine waves) and is interpreted as float. \
note that while 115 voices mind sound nice, it is computationally quite expensive.\n\
<freq. mul>is the multiplier for the frequency for each voice. 2.0 means, for example, that \
every voice should have twice the frequency of the last voice.\n\
<freq. add> is in hertz and will be added to each new voice. this way you can have non-exponential \
rise (or fall) of frequency. example: <freq> = 500.0, <freq. mul> = 0.0, <freq add.> = 1.000, will \
generate a sine wave every 1000 hz, starting at 500 hz.\n\
finally <amp. mul> is the fade out factor for the amplitude of each voice. every voice's \
amplitude will be <amp. mul> times the last voice's amplitude. note that silent voices, where the \
amplitude reached zero, will still be calculated.\
";	return const_cast<char*>(r);
}

CSmodule_OscSpec::CSmodule_OscSpec()
      : scope(0) { }

CSmodule_OscSpec::~CSmodule_OscSpec()
{
	releaseTimeline(scope);
}

CSmodule_OscSpec* CSmodule_OscSpec::newOne()
{
	return new CSmodule_OscSpec();
}


void CSmodule_OscSpec::init()
{
	gname = copyString("oscillator");

	bname = copyString("oscillator spectral");
	name = copyString(bname);
	uname = copyString(bname);

	// ---- look ----

	headerHeight = 70;
	// scope rectangle
	sx = 3; sy = 17;
	sw = width-6; sh = headerHeight - sy - 4;

	// --- inputs outputs ----

	_freq = &(createInput("f","freq (hz)",1.0)->value);
	_phase = &(createInput("p","phase")->value);
	_amp = &(createInput("a","amplitude",0.2)->value);
	_off = &(createInput("o","offset")->value);
	_rst = &(createInput("rst","sync")->value);
	_nr = &(createInput("nr","nr voices",5.0)->value);
	_mul = &(createInput("mul","freq. mul",2.0)->value);
	_add = &(createInput("add","freq. add",0.0)->value);
	_fade = &(createInput("fad","amp. mul",0.9)->value);

	_outSine = &(createOutput("osin","output")->value);

	// ---- properties ----

	createNameProperty();

	// --- internals ---

	scope = createTimeline(sw);
	for (int i=0;i<CSMOD_OSC_MAXVOICE;i++) phase[i] = 0.0;
	lrst = 0.0;

}


void CSmodule_OscSpec::step()
{
	// phase reset
	if ( (lrst<=0) && ( *_rst>0 ) )
		memset(phase, 0, CSMOD_OSC_MAXVOICE*sizeof(csfloat));
	lrst = *_rst;

	CSclipFloat(phase);

	// sine additive output
	*_outSine = 0.0;
	int nr = (int)*_nr+1;
	csfloat f = *_freq;
	csfloat a = 1.0;
	for (int i=0;i<nr;i++)
	{
		csfloat t = max((csfloat)0,min((csfloat)1, *_nr - (csfloat)i ));

		// phase modulation
		csfloat p = (phase[i] + *_phase) * TWO_PI;

		*_outSine += a * t * sin(p);

		// phase increment
		phase[i] += f * isampleRate;
		// wrap phase around to avoid overflow
		if (phase[i]>1.0) phase[i]-=1.0;
		else
		if (phase[i]<-1.0) phase[i]+=1.0;

		// spectral parameters
		f *= *_mul;
		f += *_add;
		a *= *_fade;
	}
	*_outSine = *_off + *_amp * *_outSine;

	// store sine value for scope display if output active
	recordTimeline(scope, *_outSine);


}


void CSmodule_OscSpec::draw(int offx, int offy, float zoom)
{
	// draw original module
	CSmodule::draw(offx,offy,zoom);

	// draw scope
	fl_color(headerFocusColorB);
	drawTimeline(scope, offx+zoom*(x+sx),offy+zoom*(y+sy),zoom*sw,zoom*sh, true, headerColorD);
}



















char *CSmodule_Clock::docString()
{
	const char *r = "\
this module outputs a short gate signal (one sample) the specified times per minute.\n\n\
<sync> will reset the counter.\n\
<bpm> is beats per minute. f.e. the musically 'default' 120 bpm will output two gate signals per second.\n\
<ticks> is the number of ticks per beat. so for every full beat, the <tick> output will be sending a gate \
the specified number of times.\n\n\
the number of tick tracks - that means different channels for different number of ticks per beat - can be \
set in the property window.\
";	return const_cast<char*>(r);
}


CSmodule_Clock* CSmodule_Clock::newOne()
{
	return new CSmodule_Clock();
}


void CSmodule_Clock::init()
{
	gname = copyString("oscillator");

	bname = copyString("clock");
	name = copyString("clock (bpm)");
	uname = copyString(bname);

	// --- inputs outputs ----

	_rst = &(createInput("r","sync")->value);
	_bpm = &(createInput("bpm","bpm",120.0)->value);
	//_ticks = &(createInput("t","ticks",16.0)->value);
	_out = &(createOutput("o","beat")->value);
	//_otick = &(createOutput("ot","tick")->value);

	oldNrIn = 0;
	setNrIn(1);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrtrk", "nr of tracks", nrIn, 1,CSMOD_MAX_CHAN);


	// --- internals ---

	lrst = 0.0;
	count = 0.0;
	memset(count0,0,CSMOD_MAX_CHAN*sizeof(csfloat));
}

void CSmodule_Clock::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrtrk")==0)
		setNrIn(prop->ivalue);

	CSmodule::propertyCallback(prop);
}

void CSmodule_Clock::setNrIn(int newNrIn)
{
	if (newNrIn==oldNrIn) return;

	nrIn = newNrIn;

	// delete some
	if (nrIn<oldNrIn)
	{
		while (nrCon>nrIn*2+3)
			deleteConnector(nrCon-1);
	}
	else
	// create some
	if (nrIn>oldNrIn)
	{
		for (int i=oldNrIn;i<nrIn;i++)
		{
			char *n = int2char("t",i);
			char *n1 = int2char("ticks",i+1);
			_tick[i] = &(createInput(n,n1,4)->value);
			free(n); free(n1);
			n = int2char("to",i);
			n1 = int2char("tick",i+1);
			_otick[i] = &(createOutput(n,n1)->value);
			free(n); free(n1);
		}

	}

	minHeight = headerHeight;
	height = minHeight;
	arrangeConnectors();
	oldNrIn = nrIn;
}


void CSmodule_Clock::step()
{
	// reset beat counter on sync
	if ((lrst<=0.0)&&(*_rst>0.0)) {
		count = 0.0;
		memset(count0,0,CSMOD_MAX_CHAN*sizeof(csfloat));
	}
	lrst = *_rst;

	// output beat counter
	if (count == 0.0) *_out = 1.0; else *_out = 0.0;

	// output tick counters
	for (int i=0;i<nrIn;i++)
	{
		if (count0[i] == 0.0) *_otick[i] = 1.0; else *_otick[i] = 0.0;
	}

	// inc and reset tick counters
	for (int i=0;i<nrIn;i++)
	{
		count0[i] += 1.0;
		if (count0[i] >= 60.0/max((csfloat)0, *_bpm * *_tick[i])*sampleRate) count0[i] = 0.0;
	}

	// inc and reset beat counter
	count += 1.0;
	if (count >= 60.0/max((csfloat)0, *_bpm)*sampleRate) {
		count = 0.0;
		memset(count0,0,CSMOD_MAX_CHAN*sizeof(csfloat));
		}

}













char *CSmodule_MultiSin::docString()
{
	const char *r = "\
the module outputs an adjustable number of sine waveforms with frequencies that are multiples of \
the base frequency. number of outputs can be choosen in the property view.\n\n\
this is the cheap version. there is only ONE free running phase internally, which means that all \
additional oscillators use simply multiples of that one phase, so they will all (phase-)reset \
when the original phase goes above 1.0 (or below -1.0). for all <freq mul> values which are not \
integer, this will lead to a hard cut in the sine waveform.\n\n\
<freq> will set the base frequency in hertz. it can also be negative which will reverse the \
wave-forms in time.\n\
<phase> is the phase offset (between 0.0 and 1.0 for one period).\
<amplitude> sets the maximum amplitude of the outputs (from -amplitude to amplitude).\n\
<offset> will be added to the outputs.\n\
a positive edge in <sync> will restart the phase/period (hard sync) of all oscillators.\n\n\
the factor in <freq mul> will be multiplied with the base frequency for each additional \
oscillator output. so, f.e., if <freq> = 1.0 and <freq mul> = 2.0 then the frequencies of \
the oscillators will be 1.0, 2.0, 4.0, 8.0, aso...\n\
<phase add> will be added to the phase of each additional oscillator output.\
";
	return const_cast<char*>(r);
}


CSmodule_MultiSin* CSmodule_MultiSin::newOne()
{
	return new CSmodule_MultiSin();
}


void CSmodule_MultiSin::init()
{
	gname = copyString("oscillator");
	bname = copyString("multi sine");
	name = copyString("multi sine (cheap)");
	uname = copyString(bname);

	// --- inputs outputs ----

	_freq = &(createInput("f","freq (hz)",1.0)->value);
	_phase = &(createInput("p","phase")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_rst = &(createInput("rst","sync")->value);

	_mul = &(createInput("fm","freq mul",2.0)->value);
	_add = &(createInput("fa","phase add")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("sin",i);
		char *n1 = int2char("sine ",i);
		__out[i] = createOutput(n,n1);
		free(n); free(n1);
		_out[i] = &(__out[i]->value);
	}

	oldNrOut = 0;
	setNrOut(2);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrout", "nr of outputs", nrOut, 2,CSMOD_MAX_CHAN);

	phase = 0.0;
	lrst = 0.0;
}


void CSmodule_MultiSin::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_MultiSin::setNrOut(int newNrOut)
{
	if (newNrOut==oldNrOut) return;
	nrOut = newNrOut;
	oldNrOut = nrOut;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__out[i], (i<nrOut));
	}

	arrangeConnectors();
}


void CSmodule_MultiSin::step()
{
	// phase reset
	if ( (lrst<=0) && ( *_rst>0 ) ) phase = 0.0;
	lrst = *_rst;

	CSclipFloat(phase);

	// phase modulation
	csfloat p = TWO_PI * (phase + *_phase);

	// sine output
	for (int i=0;i<nrOut;i++)
	{
		*_out[i] = *_off + *_amp * sin(p);
		p = p * *_mul + TWO_PI * *_add;
	}

	// phase increment
	phase += *_freq * isampleRate;
	// wrap phase around to avoid overflow
	if (phase>1.0) phase-=1.0;
	else
	if (phase<-1.0) phase+=1.0;
}















char *CSmodule_PolyOsc::docString()
{
	const char *r = "\
the module outputs an adjustable number of SIN,COS,SQR,SAW or TRI waveforms with \
frequencies that are multiples of the base frequency. number of outputs and waveform \
is set in the property view.\n\n\
<freq> will set the base frequency in hertz. it can also be negative which will reverse the \
wave-forms in time.\n\
<phase> is the phase offset (between 0.0 and 1.0 for one period).\
<amplitude> sets the maximum amplitude of the outputs (from -amplitude to amplitude).\n\
<offset> will be added to the outputs.\n\
a positive edge in <sync> will restart the phase/period (hard sync) of all oscillators.\n\
<pulsewidth> is used only by the SQUARE and TRIANGLE wave generator and determines \
the length of the positive edge between 0.0 and 1.0.\n\
\n\
the factor in <freq mul> will be multiplied with the base frequency for each additional \
oscillator output. so, f.e., if <freq> = 1.0 and <freq mul> = 2.0 then the frequencies of \
the oscillators will be 1.0, 2.0, 4.0, 8.0, aso...\n\
<freq add> will be added to the frequency of each additional oscillator (after the multiplication).\
<phase add> is a phase offset of each oscillator to the previous.\
";
	return const_cast<char*>(r);
}


CSmodule_PolyOsc* CSmodule_PolyOsc::newOne()
{
	return new CSmodule_PolyOsc();
}


void CSmodule_PolyOsc::init()
{
	gname = copyString("oscillator");
	bname = copyString("poly osc");
	name = copyString("poly osc");
	uname = copyString(bname);

	// --- inputs outputs ----

	_freq = &(createInput("f","freq (hz)",1.0)->value);
	_phase = &(createInput("p","phase")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_rst = &(createInput("rst","sync")->value);
	_pulse = &(createInput("pw","pulsewidth",0.5)->value);

	_mul = &(createInput("fm","freq mul",2.0)->value);
	_add = &(createInput("fa","freq add")->value);
	_padd = &(createInput("pa","phase add")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("out",i);
		char *n1 = int2char("    ",i+1);
		__out[i] = createOutput(n,n1);
		free(n); free(n1);
		_out[i] = &(__out[i]->value);
	}

	oldNrOut = 0;
	setNrOut(2);

	wave = -1;
	setWave(0);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrout", "nr of outputs", nrOut, 2,CSMOD_MAX_CHAN);
	createPropertyInt("wave", "0=sin 1=cos 2=sqr 3=saw 4=tri", 0, 0,4);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
		phase[i] = 0.0;
	lrst = 0.0;
}


void CSmodule_PolyOsc::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	if (strcmp(prop->name,"wave")==0)
		setWave(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_PolyOsc::setNrOut(int newNrOut)
{
	if (newNrOut==oldNrOut) return;
	nrOut = newNrOut;
	oldNrOut = nrOut;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__out[i], (i<nrOut));
	}

	arrangeConnectors();
}

void CSmodule_PolyOsc::setWave(int newWave)
{
	if (wave==newWave) return;
	wave=newWave;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		switch (wave)
		{
			case 1: __out[i]->uname[0] = 'c'; __out[i]->uname[1] = 'o'; __out[i]->uname[2] = 's'; break;
			case 2: __out[i]->uname[0] = 's'; __out[i]->uname[1] = 'q'; __out[i]->uname[2] = 'r'; break;
			case 3: __out[i]->uname[0] = 's'; __out[i]->uname[1] = 'a'; __out[i]->uname[2] = 'w'; break;
			case 4: __out[i]->uname[0] = 't'; __out[i]->uname[1] = 'r'; __out[i]->uname[2] = 'i'; break;
			default: __out[i]->uname[0] = 's'; __out[i]->uname[1] = 'i'; __out[i]->uname[2] = 'n'; break;
		}
	}

	arrangeConnectors();
}


void CSmodule_PolyOsc::step()
{
	// phase reset
	if ( (lrst<=0) && ( *_rst>0 ) )
		for (int i=0;i<CSMOD_MAX_CHAN;i++)
			phase[i] = 0.0;
	lrst = *_rst;

	// keep phase intact
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		csfloat p = phase[i];
		CSclipFloat(p);
		phase[i] = p;
	}

	// base freq
	csfloat
		f = *_freq,
		pa = 0; // phase add

	// sine outputs
	#define outloopstart() \
	for (int i=0;i<nrOut;i++) { \
		csfloat p = (phase[i] + *_phase + pa);

	#define outloopend() \
		phase[i] += f * isampleRate; \
		if (phase[i]>1.0) phase[i]-=1.0; \
		else \
		if (phase[i]<-1.0) phase[i]+=1.0; \
		f = f * *_mul + *_add; \
		pa += *_padd; \
		} \

	switch (wave)
	{
		case 1:
			outloopstart();
			// cos
			*_out[i] = *_off + *_amp * cos(TWO_PI * p);

			outloopend();
			break;

		case 2:
			outloopstart();

			// --- sqr ---
			if (p>=0) // positive phase
			{
				p = MODULO(p,1.0);
				*_out[i] = *_off + ( (p>=0.5)? -*_amp : *_amp );
			}
			else // negative phase
			{
				p = MODULO(-p,1.0);
				*_out[i] = *_off + ( (p>=1.0-*_pulse)? -*_amp : *_amp );
			}

			outloopend();
			break;

		case 3:
			outloopstart();

			// --- saw ---
			if (p>=0) // positive phase
			{
				p = MODULO(p,1.0);
				*_out[i] = *_off + *_amp * 2.0 * ( p - 0.5 );
			}
			else // negative phase
			{
				p = MODULO(-p,1.0);
				*_out[i] = *_off - *_amp * 2.0 * ( p - 0.5 );
			}

			outloopend();
			break;

		case 4:
			outloopstart();

			// --- tri ---
			if (p>=0) // positive phase
			{
				p = MODULO(p,1.0);
				csfloat pw = max((csfloat)0.00001,min((csfloat)0.99999, *_pulse ));
				if (p<pw)
					*_out[i] = *_off + *_amp * 2.0 * ( 0.5 - (pw-p)/pw );
				else
					*_out[i] = *_off + *_amp * 2.0 * ( (1.0-p)/(1.0-pw) - 0.5 );
			}
			else // negative phase
			{
				p = MODULO(-p,1.0);
				csfloat pw = 1.0 - max((csfloat)0.00001,min((csfloat)0.99999, *_pulse ));
				if (p<pw)
					*_out[i] = *_off + *_amp * 2.0 * ( 0.5 - (pw-p)/pw );
				else
					*_out[i] = *_off + *_amp * 2.0 * ( (1.0-p)/(1.0-pw) - 0.5 );
			}

			outloopend();
			break;

		default:
			outloopstart();
			// sine
			*_out[i] = *_off + *_amp * sin(TWO_PI * p);

			outloopend();
			break;
	}
}

















char *CSmodule_Osc3::docString()
{
	const char *r = "\
the module consists of three sine oscillators which are synced at the same time.\n\n\
amplitude, offset, frequency, phase and sync work exactly as in the normal oscillator module.\n\n\
<freq base> is a multiplier for each oscillator's frequency. so if the frequency inputs would be 1, 2 and \
3, and <freq base> would be 10, then the actual used frequencies would be 10, 20 and 30.\n\n\
the single <phase> input is directly added to each oscillator's phase to modulate them all at once.\
";	return const_cast<char*>(r);
}


CSmodule_Osc3* CSmodule_Osc3::newOne()
{
	return new CSmodule_Osc3();
}


void CSmodule_Osc3::init()
{
	gname = copyString("oscillator");

	bname = copyString("oscillator set 3");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_freq = &(createInput("f","freq base",1.0)->value);
	_phase = &(createInput("p","phase")->value);
	_sync = &(createInput("rst","sync")->value);
	//_softsync = &(createInput("srst","soft sync")->value);

	_f1 = &(createInput("f1","freq X",1.0)->value);
	_f2 = &(createInput("f2","freq Y",1.0)->value);
	_f3 = &(createInput("f3","freq Z",0.0)->value);
	_p1 = &(createInput("p1","phase X",0.00)->value);
	_p2 = &(createInput("p2","phase Y",0.25)->value);
	_p3 = &(createInput("p3","phase Z",0.00)->value);
	_a1 = &(createInput("a1","amp X",1.0)->value);
	_a2 = &(createInput("a2","amp Y",1.0)->value);
	_a3 = &(createInput("a3","amp Z",1.0)->value);
	_o1 = &(createInput("of1","offset X",0.0)->value);
	_o2 = &(createInput("of2","offset Y",0.0)->value);
	_o3 = &(createInput("of3","offset Z",0.0)->value);

	_out1 = &(createOutput("o1","X")->value);
	_out2 = &(createOutput("o2","Y")->value);
	_out3 = &(createOutput("o3","Z")->value);

	// ---- properties ----

	createNameProperty();

	// --- internals ---

	phase1 = phase2 = phase3 =
	phase1a = phase2a = phase3a =
	phase1b = phase2b = phase3b = 0.0;
	lsync = lsoftsync = 0.0;
	coeff1 = 0.0;

}


void CSmodule_Osc3::step()
{
	// phase reset
	if ( (lsync<=0) && ( *_sync>0 ) ) {
		phase1 = phase2 = phase3 =
		phase1a = phase2a = phase3a = 0.0;
		coeff1 = 0.0;
	}
	lsync = *_sync;

	CSclipFloat(phase1);
	CSclipFloatAgain(phase2);
	CSclipFloatAgain(phase3);

	// soft sync
	/*
	if ((lsoftsync<=0.0)&&(*_softsync>0.0))
    {	/ *
		if (phase1>=0.5) phase1b = 1.0-phase1; else
			if (phase1<=-0.5) phase1b = -1.0-phase1; else
				phase1b	= -phase1;
		if (phase2>=0.5) phase2b = 1.0-phase2; else
			if (phase2<=-0.5) phase2b = -1.0-phase2; else
				phase2b	= -phase2;
				*
		phase1b = 0.0;
		phase2b = phase1-phase2;
		coeff1 = 8.0 / max((csfloat)8, *_softsync * sampleRate );
	}
	lsoftsync = *_softsync;
	*/

	if (coeff1>0.0)
	{
		phase1a += coeff1*(phase1b-phase1a);
		phase2a += coeff1*(phase2b-phase2a);
		phase3a += coeff1*(phase3b-phase3a);
	}

	// three outputs
	*_out1 = *_off + *_o1 + *_amp * *_a1 * sin(TWO_PI * (*_phase + phase1+phase1a + *_p1));
	*_out2 = *_off + *_o2 + *_amp * *_a2 * sin(TWO_PI * (*_phase + phase2+phase2a + *_p2));
	*_out3 = *_off + *_o3 + *_amp * *_a3 * sin(TWO_PI * (*_phase + phase3+phase3a + *_p3));

	// phase increment
	csfloat f = *_freq * isampleRate;
	phase1 += *_f1 * f;
	phase2 += *_f2 * f;
	phase3 += *_f3 * f;

	// wrap phases around to avoid overflow
	if (phase1>1.0) phase1-=1.0; else if (phase1<-1.0) phase1+=1.0;
	if (phase2>1.0) phase2-=1.0; else if (phase2<-1.0) phase2+=1.0;
	if (phase3>1.0) phase3-=1.0; else if (phase3<-1.0) phase3+=1.0;
}



