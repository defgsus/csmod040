
#include "csmodule_mixer.h"

char *CSmodule_Mixer::docString()
{
	const char *r = "\
the module is used to mix a number of channels together.\n\
each channel has it's own amplitude value/input, and the sum of all \
channels is scaled by the main amplitude value.\n\
use the property window ('p' or double-click) to set the number of channels.";
	return const_cast<char*>(r);
}


CSmodule_Mixer* CSmodule_Mixer::newOne()
{
	return new CSmodule_Mixer();
}


void CSmodule_Mixer::init()
{
	gname = copyString("signal");
	bname = copyString("mixer");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_mainAmp = &(createInput("mamp","main amp.",1.0)->value);
	_out = &(createOutput("out","output")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		__in[i] = createInput(n,"in");
		free(n);
		n = int2char("amp",i);
		__amp[i] = createInput(n,"amp",1.0);
		free(n);

		_in[i] = &(__in[i]->value);
		_amp[i] = &(__amp[i]->value);

	}

	oldNrIn = 0;
	setNrIn(2);

	// ---- properties ----

	createNameProperty();

	createPropertyInt("nrin", "nr of inputs", nrIn, 1,CSMOD_MAX_CHAN);

}


void CSmodule_Mixer::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);

	CSmodule::propertyCallback(prop);
}

void CSmodule_Mixer::setNrIn(int newNrIn)
{
	if (newNrIn==oldNrIn) return;
	nrIn = newNrIn;
	oldNrIn = nrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		bool b = (i<nrIn);
		setConnectorActive(__in[i], b);
		setConnectorActive(__amp[i], b);
	}

	arrangeConnectors();
}


void CSmodule_Mixer::step()
{
	// sum inputs
	*_out = 0.0;
	for (int i=0;i<nrIn;i++) *_out += *_in[i] * *_amp[i];
	// scale
	*_out *= *_mainAmp;
}
