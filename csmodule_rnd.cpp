#include "math.h"
#include "csmodule_rnd.h"

char *CSmodule_Rnd::docString()
{
	const char *r = "\
use the random module to produce a (pseudo-)random number, \
whenever a positive edge in <gate> occures.\n\n\
<offset> will be added to the output value (a random number in the range of -<amplitude> to <amplitude>).\n\
a positive value in <reset> will reset the random seed to the <seed> value, so the sequence repeats.\n\
the <pattern 1> and <pattern 2> inputs (integer, something like -1000000 to 1000000) \
modify the random generator to retrieve different patterns. especially modifying <pattern 2> \
can lead to pretty un-random behaviour.\
";
	return const_cast<char*>(r);
}


CSmodule_Rnd* CSmodule_Rnd::newOne()
{
	return new CSmodule_Rnd();
}


void CSmodule_Rnd::init()
{
	gname = copyString("random");
	bname = copyString("random number");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_gate = &(createInput("g","gate")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_rst = &(createInput("r","reset")->value);
	_seed = &(createInput("s0","seed",rand()%0xffff)->value);
	_seed1 = &(createInput("s1","pattern 1")->value);
	_seed2 = &(createInput("s2","pattern 2")->value);

	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

	lrst = 0.0;
	lgate = 0.0;
	seed = *_seed;
}

void CSmodule_Rnd::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_Rnd::step()
{
	// reset seed
	if ((lrst<=0.0)&&(*_rst>0.0)) seed = *_seed;
	lrst = *_rst;

	// get number
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		seed = (seed * (196314165 + (unsigned int)(*_seed2)))
					+ (907633515 + (unsigned int)(*_seed1));

		*_out = *_off + *_amp * 2.0 * ( (csfloat)seed / 0xffffffff - 0.5);
	}
	lgate = *_gate;

}











char *CSmodule_RndTrig::docString()
{
	const char *r = "\
use the random module to produce a short gate signal (one sample) whenever a positive edge in \
<gate> occures, and a random number (between 0.0 and 1.0) is less then or \
equal to <probability>.\n\n\
a positive edge in <reset> will reset the random seed to the <seed> value and start the random sequence again.\n\
the <seed 1> and <seed 2> inputs (integer, something like -1000000.0 to 1000000.0) \
modify the random generator to retrieve different patterns. especially modifying <seed 2> can \
lead to pretty un-random behaviour.\
";
	return const_cast<char*>(r);
}


CSmodule_RndTrig* CSmodule_RndTrig::newOne()
{
	return new CSmodule_RndTrig();
}


void CSmodule_RndTrig::init()
{
	gname = copyString("random");
	bname = copyString("random trigger");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_gate = &(createInput("g","gate")->value);
	_prob = &(createInput("pr","probability",0.5)->value);
	_rst = &(createInput("r","reset")->value);
	_seed = &(createInput("s0","seed",rand()%0xffff)->value);
	_seed1 = &(createInput("s1","pattern 1")->value);
	_seed2 = &(createInput("s2","pattern 2")->value);

	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

	lrst = 0.0;
	lgate = 0.0;
	seed = *_seed;
}

void CSmodule_RndTrig::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_RndTrig::step()
{
	// reset seed
	if ((lrst<=0.0)&&(*_rst>0.0)) seed = *_seed;
	lrst = *_rst;

	*_out = 0.0;
	// get number
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		seed = seed * (196314165 + (unsigned int)(*_seed2))
					+ (907633515 + (unsigned int)(*_seed1));

		if (((csfloat)seed/0xffffffff) <= *_prob ) *_out = 1.0;
	}
	lgate = *_gate;

}












char *CSmodule_NoiseWhite::docString()
{
	const char *r = "\
this module simply outputs rather pure white noise between -<amplitude> and <amplitude> (+<offset>). \
the random sequence can be reset with a positive edge in <reset>. the value in <seed> will be used as \
start seed when reset.\
";
	return const_cast<char*>(r);
}


CSmodule_NoiseWhite* CSmodule_NoiseWhite::newOne()
{
	return new CSmodule_NoiseWhite();
}


void CSmodule_NoiseWhite::init()
{
	gname = copyString("random");
	bname = copyString("noise white");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_rst = &(createInput("r","reset")->value);
	_seed = &(createInput("s0","seed",rand()%0xffff)->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

	seed = 1+*_seed;
	lrst = 0.0;
}

void CSmodule_NoiseWhite::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_NoiseWhite::step()
{
	// reset seed
	if ((lrst<=0.0)&&(*_rst>0.0)) seed = 1 + *_seed;
	lrst = *_rst;

	// wow, what a rnd generator...
	seed *= 16807;

	// set output
    *_out = *_off + *_amp * ( -1.0 + (csfloat)seed / 0x80000000 );
}









char *CSmodule_NoisePitch::docString()
{
	const char *r = "\
this module will output random numbers between -<amplitude> and <amplitude> (+<offset>) at a \
defined frequency, either stepped or linearly or 'smoothly' interpolated.\n\n\
<reset> will restart the random sequence.\n\
<frequency> sets the frequency in which new numbers are generated.\n\n\
<pattern> is interpreted as integer and modifies the random number generator. \
probably all even numbers give the same kind of white noise, while odd numbers will \
result in a small repeating pattern. once a while it is nescessary to reset the module \
to reinitialize the seed, when playing around with the pattern.\n\
\n\
the <output> provides fixed new random numbers with the given frequency.\n\
if <linear> is connected it will output a linear interpolated version.\n\
if <smooth> is connected, a nice smooth interpolation will be used instead.\n\
formula is : t = 5*t^2*(1-t) + t^5 , where t is the fade position (0.0 - 1.0). this is \
a relatively cheap function and certainly no cubic interpolation or something. \
only two data points are used and the result is ok.\
";
	return const_cast<char*>(r);
}


CSmodule_NoisePitch* CSmodule_NoisePitch::newOne()
{
	return new CSmodule_NoisePitch();
}


void CSmodule_NoisePitch::init()
{
	gname = copyString("random");
	bname = copyString("noise pitch");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_rst = &(createInput("r","reset")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);
	_freq = &(createInput("f","frequency",1.0)->value);
	_pattern = &(createInput("p","pattern")->value);

	_out = &(createOutput("out","output")->value);
	outL = createOutput("outl","linear"); _outL = &(outL->value);
	outS = createOutput("outs","smooth"); _outS = &(outS->value);

	// ---- properties ----

	createNameProperty();

	lrst = 0.0;
	seed = 0;
	frame = 0;
	n0 = 0.0;
	n1 = 0.0;
}

void CSmodule_NoisePitch::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_NoisePitch::step()
{
	// reset seed
	if ((lrst<=0.0)&&(*_rst>0.0)) seed = 1;
	lrst = *_rst;

	// determine sample length
	unsigned int frames = sampleRate / max((csfloat)0.001, *_freq);

	if (frame>=frames)
	{
		if (seed==0) seed = 1;
		// get new random number
		seed *= (16807 + (unsigned int)*_pattern);
		n1 = n0;
		n0 = *_off + *_amp * ( (csfloat)seed / 0x80000000 - 1.0 );
		frame = 0;
	}
	else
	{
		// else count
		frame++;
	}

	// --- output noise ---

	*_out = n0;

	// --- output interpolated noise ---

	// linear delta
	float f = (csfloat)frame/frames;

	if (outL->nrCon)
		*_outL = n0*f + (1-f)*n1;

	// smooth version
	if (outS->nrCon)
	{
		f = 5*f*f*(1-f) + f*f*f*f*f;
		*_outS = n0*f + (1-f)*n1;
	}
}
