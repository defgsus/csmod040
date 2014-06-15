#include <algorithm>
#include "math.h"
#include "csmodule_sh.h"

char *CSmodule_SH::docString()
{
	const char *r = "\
on every positive edge in <gate> the module passes the value at <input> through to the <output> \
and holds it until the next positive edge.\
";
	return const_cast<char*>(r);
}


CSmodule_SH* CSmodule_SH::newOne()
{
	return new CSmodule_SH();
}


void CSmodule_SH::init()
{
	gname = copyString("logic");
	bname = copyString("sample & hold");
	name = copyString("sample and hold");
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_rst = &(createInput("rst","gate")->value);

	_out = &(createOutput("o","output")->value);

	// ---- properties ----

	createNameProperty();

	lrst = 0.0;
}



void CSmodule_SH::step()
{
	if ((lrst<=0.0) && (*_rst>0.0)) *_out = *_in;
	lrst = *_rst;
}







char *CSmodule_SHfollow::docString()
{
	const char *r = "\
on every positive edge in <gate> the module starts to follow the value in <input> and passing \
this through to the <output>. the time the module takes to follow is given in seconds with the \
<time (sec)> input.\
";
	return const_cast<char*>(r);
}


CSmodule_SHfollow* CSmodule_SHfollow::newOne()
{
	return new CSmodule_SHfollow();
}


void CSmodule_SHfollow::init()
{
	gname = copyString("logic");
	bname = copyString("s & h (follow)");
	name = copyString("s and h (follow)");
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_rst = &(createInput("rst","gate")->value);
	_time = &(createInput("t","time (sec)",1.0)->value);

	_out = &(createOutput("o","output")->value);
	__outS = createOutput("os","smooth");
	_outS = &__outS->value;

	// ---- properties ----

	createNameProperty();

	mframe = 1;
	frame = 0;
	lrst = 0.0;
	sam = 0.0;
	samo = 0.0;
}



void CSmodule_SHfollow::step()
{
	if ((lrst<=0.0) && (*_rst>0.0)) { sam = *_in; samo = *_out; frame = 0; }
	lrst = *_rst;

    mframe = max((csfloat)1, *_time * sampleRate);
	csfloat t = min((csfloat)1, (csfloat)frame / mframe);

	*_out = samo * (1-t) + t * sam;
	if (__outS->nrCon)
	{
		t = 5*t*t*(1-t) + t*t*t*t*t;
		*_outS = samo * (1-t) + t * sam;
	}

	if (frame<mframe) frame++;
}











char *CSmodule_SHmulti::docString()
{
	const char *r = "\
on every positive edge in <gate> the module passes the value at <input> through to the chain of \
sequential outputs <out x>.\n\n\
the number of outputs is selected in properties.\
";
	return const_cast<char*>(r);
}


CSmodule_SHmulti* CSmodule_SHmulti::newOne()
{
	return new CSmodule_SHmulti();
}


void CSmodule_SHmulti::init()
{
	gname = copyString("logic");
	bname = copyString("s & h (multi)");
	name = copyString("s and h (multi)");
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_rst = &(createInput("rst","gate")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("out",i);
		char *n1 = int2char("out ",i+1);
		__out[i] = createOutput(n,n1);
		free(n); free(n1);
		_out[i] = &(__out[i]->value);

	}

	oldNrOut = 0;
	setNrOut(2);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrout", "nr of outputs", nrOut, 1,CSMOD_MAX_CHAN);

	lrst = 0.0;
}

void CSmodule_SHmulti::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_SHmulti::setNrOut(int newNrOut)
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



void CSmodule_SHmulti::step()
{
	if ((lrst<=0.0) && (*_rst>0.0))
	{
		for (int i=nrOut-1;i>0;i--)
			*_out[i] = *_out[i-1];
		*_out[0] = *_in;
	}
	lrst = *_rst;
}















char *CSmodule_SHpoly::docString()
{
	const char *r = "\
on every positive edge in <gate> the module passes the value at <input> through to one of the \
outputs <out x>. the number of outputs is selected in properties.\n\n\
if the internal counter (output in <chan nr>) reaches the set nr of outputs or the value \
in <nr voices> it goes back to 0.\n\n\
a gate in <reset> will return the counter to 0. to clear the outputs, send some gates (f.e. with the \
gate repeat module) into <gate> and the reset value into <input>.\
\
";
	return const_cast<char*>(r);
}


CSmodule_SHpoly* CSmodule_SHpoly::newOne()
{
	return new CSmodule_SHpoly();
}


void CSmodule_SHpoly::init()
{
	gname = copyString("logic");
	bname = copyString("s & h (poly)");
	name = copyString("s and h (poly)");
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_gate = &(createInput("g","gate")->value);
	_rst = &(createInput("rst","reset")->value);
	_nrVoices = &(createInput("nrv","nr voices",2)->value);

	_outnr = &(createOutput("nr","chan nr")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("out",i);
		char *n1 = int2char("out ",i);
		__out[i] = createOutput(n,n1);
		free(n); free(n1);
		_out[i] = &(__out[i]->value);

	}

	oldNrOut = 0;
	setNrOut(2);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrout", "nr of outputs", nrOut, 1,CSMOD_MAX_CHAN);

	chan = 0;
	lgate = 0.0;
	lrst = 0.0;
}

void CSmodule_SHpoly::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_SHpoly::setNrOut(int newNrOut)
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



void CSmodule_SHpoly::step()
{
	// reset channel
	if ((lrst<=0.0) && (*_rst>0.0))
	{
		chan = 0;
		*_outnr = 0;
	}
	lrst = *_rst;

	// gate action
	if ((lgate<=0.0) && (*_gate>0.0))
	{
		*_out[chan] = *_in;
		chan = chan + 1;
		if ((chan>=nrOut)||(chan>=*_nrVoices)) chan = 0;
		*_outnr = chan;
	}
	lgate = *_gate;
}












char *CSmodule_Counter::docString()
{
	const char *r = "\
this module counts the positive edges in <gate>.\n\n\
it resets itself when <output> <= <min> or when <output> >= <max> (if <max> is not 0). \
when one of this happens <gate (max)>) is set to 1 for the time <output> stays 0.\n\n\
a gate in <reset> will reset the counter to the value in <load>.\n\n\
<direction> is interpreted as integer at sets the direction and stepwidth of the counter, \
so it can be negative as well.\
";
	return const_cast<char*>(r);
}


CSmodule_Counter* CSmodule_Counter::newOne()
{
	return new CSmodule_Counter();
}


void CSmodule_Counter::init()
{
	gname = copyString("logic");
	bname = copyString("counter");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_gate = &(createInput("gate","gate")->value);
	_min = &(createInput("min","min",-1.0)->value);
	_max = &(createInput("max","max")->value);
	_rst = &(createInput("rst","reset")->value);
	_load = &(createInput("load","load")->value);
	_dir = &(createInput("dir","direction",1.0)->value);

	_out = &(createOutput("o","output")->value);
	_outM = &(createOutput("om","gate (max)")->value);

	// ---- properties ----

	createNameProperty();

	lrst = 0.0;
	lgate = 0.0;
}



void CSmodule_Counter::step()
{
	// count gates
	if ((lgate<=0.0) && (*_gate>0.0)) *_out = (int)(*_out) + (int)*_dir;
	lgate = *_gate;;

	// reset thing on sync
	if ((lrst<=0.0) && (*_rst>0.0)) { *_out = *_load; }
	lrst = *_rst;

	// reset when to low
	if ((*_out <= *_min)) { *_out = 0.0; *_outM = 1.0; };

	// reset when to high
	if ((*_max!=0.0) && (*_out >= *_max)) { *_out = 0.0; *_outM = 1.0; };

	// keep <gate> as long as output stays 0
	if (*_out!=0.0) *_outM = 0.0;
}





char *CSmodule_SampleCounter::docString()
{
	const char *r = "\
this module counts samples, means: it counts upwards (or downwards) with each sample step.\n\n\
it resets itself when <output> <= <min> or when <output> >= <max> (if <max> is not 0). \
when one of this happens <gate (max)>) is set to 1 for the time <output> stays 0.\n\
a gate in <reset> will reset the counter to the value in <load>.\n\n\
<direction> is interpreted as integer at sets the direction and stepwidth of the counter, \
so it can be negative as well.\
";
	return const_cast<char*>(r);
}


CSmodule_SampleCounter* CSmodule_SampleCounter::newOne()
{
	return new CSmodule_SampleCounter();
}


void CSmodule_SampleCounter::init()
{
	gname = copyString("logic");
	bname = copyString("counter (sample)");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_min = &(createInput("min","min",-1.0)->value);
	_max = &(createInput("max","max")->value);
	_rst = &(createInput("rst","reset")->value);
	_load = &(createInput("load","load")->value);
	_dir = &(createInput("dir","direction",1.0)->value);

	_out = &(createOutput("o","output")->value);
	_outM = &(createOutput("om","gate (max)")->value);

	// ---- properties ----

	createNameProperty();

	lrst = 0.0;
	lgate = 0.0;
}



void CSmodule_SampleCounter::step()
{
	// count samples
	*_out = (int)(*_out) + (int)*_dir;

	// reset thing on sync
	if ((lrst<=0.0) && (*_rst>0.0)) *_out = *_load;
	lrst = *_rst;

	// reset when to low
	if ((*_out <= *_min)) { *_out = 0.0; *_outM = 1.0; };

	// reset when to high
	if ((*_max!=0.0) && (*_out >= *_max)) { *_out = 0.0; *_outM = 1.0; };

	// keep <gate> as long as output stays 0
	if (*_out!=0.0) *_outM = 0.0;
}













char *CSmodule_CounterSec::docString()
{
	const char *r = "\
this module counts the time since a positive edge in <gate>. \
it resets itself when <output> = <max> (if <max> is greater than 0) sets <gate (max)>) to 1 for one sample.\
";
	return const_cast<char*>(r);
}


CSmodule_CounterSec* CSmodule_CounterSec::newOne()
{
	return new CSmodule_CounterSec();
}


void CSmodule_CounterSec::init()
{
	gname = copyString("logic");
	bname = copyString("counter sec");
	name = copyString("counter seconds");
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_gate = &(createInput("gate","gate")->value);
	_max = &(createInput("max","max")->value);

	_out = &(createOutput("o","output")->value);
	_outM = &(createOutput("om","gate (max)")->value);

	// ---- properties ----

	createNameProperty();

	lgate = 0.0;

	samples = 0;
	seconds = 0;
}



void CSmodule_CounterSec::step()
{
	*_out = seconds + (csfloat)samples * isampleRate;

	#define Reset() { samples=0; seconds=0; *_out=0; }

	// reset on gate
	if ((lgate<=0.0) && (*_gate>0.0)) Reset();
	lgate = *_gate;

	samples++;
	if (samples>=sampleRate) { seconds++; samples = 0; }

	// reset when to high
	if ((*_max>0.0) && (*_out >= *_max)) { Reset(); *_outM = 1.0; }
		else *_outM = 0.0;
}
