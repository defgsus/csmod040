#include "math.h"
#include "csmodule_clockdiv.h"

char *CSmodule_ClockDiv::docString()
{
	const char *r = "\
the clock divider counts the number of positive inputs at <clock> and outputs \
every <x>th signal, defined by <divider>\n\n\
<reset> resets the counter\n\
the output will be a square wave with pulse width defined by the <pulse width> input.\n\
this module makes it easy to octavize any input signal down to a square wave, or to lower the tempo \
of a gate/sequencer signal.\
";
	return const_cast<char*>(r);
}


CSmodule_ClockDiv* CSmodule_ClockDiv::newOne()
{
	return new CSmodule_ClockDiv();
}


void CSmodule_ClockDiv::init()
{
	gname = copyString("logic");
	bname = copyString("clock divider");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_rst = &(createInput("rst","reset")->value);
	_clock = &(createInput("clk","clock")->value);
	_div = &(createInput("div","divider", 2.0)->value);
	_pulse = &(createInput("pls","pulsewidth",0.5)->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

	// -- intern ---

	lrst = 0;
	lclock = 0;
	frame = 0;
	oframe = 0;
	spc = 0;
}

void CSmodule_ClockDiv::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_ClockDiv::step()
{
	// reset counter
	if ((lrst<=0.0)&&(*_rst>0.0)) { count = 0; frame = 0; oframe = 0; }
	lrst = *_rst;

	// inc counter
	if ((lclock<=0.0) && (*_clock>0)) count++;
	lclock = *_clock;

	frame++;


	// start clock output ?
	if (count >= *_div)
	{
		spc = frame;
		frame = 0;
		oframe = 0;
		*_out = 1.0;
		count = 0;
	}

	oframe++;


	// square phase
	if (oframe > spc* *_pulse)
	{
		*_out = 0.0;
		oframe = 0;
	}

}

