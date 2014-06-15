#include <algorithm>
#include "math.h"
#include "csmodule_analyze.h"

char *CSmodule_ValueMeter::docString()
{
	const char *r = "\
the module displays fading meters for input signals.\n\n\
<in> are the different inputs of the module.\n\n\
the number of inputs can be set in properties.\n\
also set in the properies are <range min> and <range max>. they tell the meters \
the minimum/maximum expected input value.\n\
to use the absolute value of the input (without sign) set <take absolute value> to 1.\
";
	return const_cast<char*>(r);
}


CSmodule_ValueMeter* CSmodule_ValueMeter::newOne()
{
	return new CSmodule_ValueMeter();
}


void CSmodule_ValueMeter::init()
{
	gname = copyString("analyze");
	bname = copyString("value meter");
	name = copyString(bname);
	uname = copyString(bname);

	// -- look --

	width = 160;
	minWidth = 160;

	// --- inputs outputs ----

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		__in[i] = createInput(n,"in");
		_in[i] = &(__in[i]->value);
		free(n);

	}

	oldNrIn = 0;
	setNrIn(1);

	// -- internals --

	absolute = false;
	rangemin = 0.0;
	rangemax = 1.0;
	range = 1.0;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
		temp[i] = 0.0;

	// ---- properties ----

	createNameProperty();
	createPropertyFloat("min", "range min", rangemin);
	createPropertyFloat("max", "range max", rangemax);
	createPropertyInt("abs", "take absolute value", absolute, 0,1);
	createPropertyInt("nrchan", "nr of inputs", nrIn, 1,CSMOD_MAX_CHAN);

}


void CSmodule_ValueMeter::setNrIn(int newNrIn)
{
	if (newNrIn==oldNrIn) return;
	nrIn = newNrIn;
	oldNrIn = nrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
	}

	arrangeConnectors();
}


void CSmodule_ValueMeter::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrchan")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"min")==0)
	{
		rangemin = prop->fvalue;
        range = 1.0 / std::max((csfloat)0.0001, rangemax - rangemin);
	}
	else
	if (strcmp(prop->name,"max")==0)
	{
		rangemax = prop->fvalue;
        range = 1.0 / std::max((csfloat)0.0001, rangemax - rangemin);
	}
	else
	if (strcmp(prop->name,"abs")==0)
		absolute = (prop->ivalue==1);
	else

	CSmodule::propertyCallback(prop);
}



void CSmodule_ValueMeter::step()
{
	for (int i=0;i<nrIn;i++)
	{
		csfloat f = (absolute)?
            std::max((csfloat)0,std::min((csfloat)1, ((csfloat)fabs(*_in[i]) - rangemin) * range ))
			:
            std::max((csfloat)0,std::min((csfloat)1, (*_in[i] - rangemin) * range ));
		if (*_in[i]>temp[i])
			temp[i] += 100*isampleRate*( f - temp[i]);
		else
			temp[i] += 10*isampleRate*( f - temp[i]);
	}

}



void CSmodule_ValueMeter::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	int X = offx + (x+30)*zoom;
	int w = (width-35)*zoom;
	int h = (inputHeight>>1) * zoom;
	for (int i=0;i<nrIn;i++)
	{
		int Y = offy + (y+__in[i]->y-6)*zoom;
		int t = w*temp[i];
		fl_color(FL_GREEN);
		fl_rectf(X,Y, w,h);
		fl_color(FL_BLACK);
		fl_rectf(X+t,Y, w-t,h);
	}
}



























char *CSmodule_PitchDetect::docString()
{
	const char *r = "\
the module counts the time between zerocrossings in <input> and outputs the messured \
frequency in hertz.\n\
\n\
<follow> determines the speed with which the output follows the detected frequency. \
1.0 is immidiate, smaller than 1.0 is slower.\
";
	return const_cast<char*>(r);
}


CSmodule_PitchDetect* CSmodule_PitchDetect::newOne()
{
	return new CSmodule_PitchDetect();
}


void CSmodule_PitchDetect::init()
{
	gname = copyString("analyze");
	bname = copyString("pitch detector");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_follow = &(createInput("fol","follow",1.0)->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

	lin = 0.0;
	samples = 0;
	samplesc = sampleRate;

}


void CSmodule_PitchDetect::step()
{
	if ((lin<=0.0)&&(*_in>0.0))
	{
		samplesc = max((unsigned int)1, samples);
		samples = 0;
	}
	lin = *_in;

	samples ++;

	*_out += max((csfloat)0,min((csfloat)1, *_follow * *_follow * normSampleRate ))
		* ( sampleRate / samplesc - *_out)
		;
}


