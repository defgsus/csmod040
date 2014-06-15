#include "math.h"
#include "csmodule_clip.h"

char *CSmodule_ClipSoft::docString()
{
	const char *r = "\
a soft clipper for input signals. the value of the output will always be between -1.0 and 1.0.\n\n\
the module uses a fast tangens hyperbolicus approximation. a value of 1.0 will be output as ~ 0.777, \
a value of 3.0 or more will output 1.0.\
";
	return const_cast<char*>(r);
}


CSmodule_ClipSoft* CSmodule_ClipSoft::newOne()
{
	return new CSmodule_ClipSoft();
}


void CSmodule_ClipSoft::init()
{
	gname = copyString("signal");
	bname = copyString("clipper soft");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);

	_out = &(createOutput("o","output")->value);

	// ---- properties ----

	createNameProperty();

}

void CSmodule_ClipSoft::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_ClipSoft::step()
{
	if (*_in<-3) *_out = -1.0;
	else
	if (*_in>3) *_out = 1.0;
	else
	{
		csfloat in2 = *_in * *_in;
		*_out = *_in * ( 27 + in2 ) / ( 27 + 9 * in2 );
	}

}





char *CSmodule_ClipHard::docString()
{
	const char *r = "\
a hard clipper to keep an input within a certain range.\n\n\
the value of the output will always be between <min> and <max>.\
";
	return const_cast<char*>(r);
}


CSmodule_ClipHard* CSmodule_ClipHard::newOne()
{
	return new CSmodule_ClipHard();
}


void CSmodule_ClipHard::init()
{
	gname = copyString("signal");
	bname = copyString("clipper hard");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_min = &(createInput("mi","min",-1.0)->value);
	_max = &(createInput("ma","max",1.0)->value);

	_out = &(createOutput("o","output")->value);

	// ---- properties ----

	createNameProperty();

}

void CSmodule_ClipHard::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_ClipHard::step()
{
	if (*_in < *_min) *_out = *_min;
	else
	if (*_in > *_max) *_out = *_max;
	else
	*_out = *_in;
}

