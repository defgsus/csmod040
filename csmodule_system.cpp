#include "csmodule_system.h"


char *CSmodule_StartGate::docString()
{
	const char *r = "\
this module outputs a small gate on every start of the audio engine. this can be useful to \
always have your patches synchronized.\
";
	return const_cast<char*>(r);
}


CSmodule_StartGate* CSmodule_StartGate::newOne()
{
	return new CSmodule_StartGate();
}


void CSmodule_StartGate::init()
{
	gname = copyString("system");
	bname = copyString("start gate");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	width = minWidth = 84;

	_out = &(createOutput("o","output")->value);

	// ---- properties ----

	createNameProperty();

	started = false;

}


void CSmodule_StartGate::start()
{
	started = true;
	*_out = 1.0;
}

void CSmodule_StartGate::step()
{
	if (started)
		started = false;
	else
		*_out = 0.0;
}
























char *CSmodule_SampleGate::docString()
{
	const char *r = "\
this module outputs a small gate every other sample. means, f.e. if your samplerate is 44100 hz then \
the module will generate a gate signal at 22050 hz.\n\n\
right now there is no way of sending a gate signal on EVERY sample.\n\
";
	return const_cast<char*>(r);
}


CSmodule_SampleGate* CSmodule_SampleGate::newOne()
{
	return new CSmodule_SampleGate();
}


void CSmodule_SampleGate::init()
{
	gname = copyString("system");
	bname = copyString("sample gate");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	width = minWidth = 84;

	_out = &(createOutput("o","output")->value);

	// ---- properties ----

	createNameProperty();

}



void CSmodule_SampleGate::step()
{
	if (*_out!=0.0)
		*_out = 0.0;
	else
		*_out = 1.0;
}










char *CSmodule_SampleRate::docString()
{
	const char *r = "\
this module outputs the current sample rate of the container it is placed in.\
";
	return const_cast<char*>(r);
}


CSmodule_SampleRate* CSmodule_SampleRate::newOne()
{
	return new CSmodule_SampleRate();
}


void CSmodule_SampleRate::init()
{
	gname = copyString("system");
	bname = copyString("sample rate");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	width = minWidth = 84;

	_out = &(createOutput("o","output")->value);

	// ---- properties ----

	createNameProperty();

}


void CSmodule_SampleRate::step()
{
	*_out = sampleRate;
}










char *CSmodule_OnRender::docString()
{
	const char *r = "\
this module outputs 1.0 when disk rendering is active, 0.0 otherwise.\
";
	return const_cast<char*>(r);
}


CSmodule_OnRender* CSmodule_OnRender::newOne()
{
	return new CSmodule_OnRender();
}


void CSmodule_OnRender::init()
{
	gname = copyString("system");
	bname = copyString("on rendering");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	width = minWidth = 84;

	_out = &(createOutput("o","output")->value);

	// ---- properties ----

	createNameProperty();

}


void CSmodule_OnRender::step()
{
	if (parentRoot)
		*_out = parentRoot->isRendering;
	else
		*_out = 0.0;
}
