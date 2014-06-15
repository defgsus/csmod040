#include "csmodule_audioout.h"
#include "csmodcontainer.h"

char *CSmodule_AudioOut::docString()
{
	const char *r = "\
this module provides the audio out interface to the system audio device.\n\
the device is choosen in the audio preferences (F10). each audio out module \
regardless of where it is placed within the patch will mix it's inputs to \
the audio outputs. you can add as many audio out modules as you may need \
they all mix together.\n\
<amplitude> will scale the volume of any input to the module.\n\
<mono in> will pass the input to every channel of the audio out.\n\
each channel has an <in x> input to feed them separately.\n\
you can set the number of output channels for each audio out device in it's \
property window ('p' or double-click). however, only the number of channels \
supported by the selected audio device are used.\
";
	return const_cast<char*>(r);
}


CSmodule_AudioOut* CSmodule_AudioOut::newOne()
{
	return new CSmodule_AudioOut();
}


void CSmodule_AudioOut::init()
{
	gname = copyString("IO");
	bname = copyString("audio out");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_amp = &(createInput("a","amplitude",1.0)->value);
	_inmono = &(createInput("inm","mono in")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		__in[i] = createInput(n,"in");
		free(n);
		_in[i] = &(__in[i]->value);
	}

	oldNrIn = 0;
	setNrIn(1);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrchan", "nr of channels", nrIn, 1,CSMOD_MAX_CHAN);

	// --- look ----

	minWidth = 80;
	width = 80;

	CSsetColor(headerColor, 90,50,50);
	CSsetColor(headerFocusColor, 160,110,110);
	CSsetColor(bodyColor, 120,80,80);
	updateColors();

	// -- internals --


}


void CSmodule_AudioOut::setNrIn(int newNrIn)
{
	if (newNrIn==oldNrIn) return;
	nrIn = newNrIn;
	oldNrIn = nrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
	}

	arrangeConnectors();

	/* also set the property here, because the container can call
	   this function to set the appropiate number of channels for
	   the audio out/in module */
	for (int i=0;i<nrProp;i++)
		if (!strcmp(property[i]->name, "nrchan"))
			property[i]->ivalue = nrIn;
}


void CSmodule_AudioOut::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrchan")==0)
		setNrIn(prop->ivalue);
	else
		CSmodule::propertyCallback(prop);
}


void CSmodule_AudioOut::step()
{
	if (parentRoot)
	{
		// mix the inputs to the rootContainer's sample buffer

		float *s = parentRoot->outputSample;

		for (int i=0;i<nrIn;i++)
			*s++ += *_amp * (*_inmono + *_in[i]);
	}

}
