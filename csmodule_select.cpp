#include <math.h>
#include "csmodule_select.h"

char *CSmodule_Select::docString()
{
	const char *r = "\
this module cross-fades between several input channels.\n\n\
number of channels can be selected in properties\n\n\
<sel> selects the input channel. integer values will select the single input, starting at 0. \
any fractional part will mix the channel and the next one. the number in sel will be wrapped around \
the number of channels.\
";
	return const_cast<char*>(r);
}


CSmodule_Select* CSmodule_Select::newOne()
{
	return new CSmodule_Select();
}


void CSmodule_Select::init()
{
	gname = copyString("signal");
	bname = copyString("select");
	name = copyString("multiplex in");
	uname = copyString(name);

	// --- inputs outputs ----

	_sel = &(createInput("sel","sel")->value);
	_out = &(createOutput("out","output")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		char *n1 = int2char("in ",i);
		__in[i] = createInput(n,n1);
		free(n); free(n1);
		_in[i] = &(__in[i]->value);
	}

	oldNrIn = 0;
	setNrIn(2);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of inputs", nrIn, 2,CSMOD_MAX_CHAN);
}


void CSmodule_Select::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Select::setNrIn(int newNrIn)
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


void CSmodule_Select::step()
{
	// select channel
	csfloat f = fabs(*_sel);
	f = MODULO(f, nrIn);
	unsigned int c = (unsigned int)f;
	// transmit
	*_out = *_in[c];

	// fade fractional part
	f = f-c;
	if (f>0.0)
	{
		c = (c+1)% nrIn;

		*_out = *_out * (1.0-f) + f* *_in[c];
	}
}
















char *CSmodule_SelectOut::docString()
{
	const char *r = "\
this module passes it's input to one of several output channels.\n\n\
the number of channels can be selected in properties.\n\n\
<sel> selects the output channel. integer values will select the single output, starting at 0. \
any fractional part will mix the channel and the next one. the number in sel will be wrapped around \
the number of channels.\
";
	return const_cast<char*>(r);
}


CSmodule_SelectOut* CSmodule_SelectOut::newOne()
{
	return new CSmodule_SelectOut();
}


void CSmodule_SelectOut::init()
{
	gname = copyString("signal");
	bname = copyString("select out");
	name = copyString("multiplex out");
	uname = copyString(name);

	// --- inputs outputs ----

	_sel = &(createInput("sel","sel")->value);
	_in = &(createInput("in","input")->value);

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
	createPropertyInt("nrout", "nr of outputs", nrOut, 2,CSMOD_MAX_CHAN);
}


void CSmodule_SelectOut::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_SelectOut::setNrOut(int newNrOut)
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


void CSmodule_SelectOut::step()
{
	// clear outputs
	for (int i=0;i<nrOut;i++) *_out[i] = 0;

	// select channel
	csfloat f = fabs(*_sel);
	f = MODULO(f, nrOut);
	unsigned int c = (unsigned int)f;

	// transmit
	*_out[c] = *_in;

	// fade fractional part
	f = f-c;
	if (f>0.0)
	{
		*_out[c] = (1-f) * *_in;
		c = (c+1)% nrOut;
		*_out[c] = f * *_in;
	}
}


















char *CSmodule_SelectInNN::docString()
{
	const char *r = "\
this module cross-fades between several inputs on a number channels.\n\n\
number of inputs and channels can be selected in properties\n\n\
<sel> selects the input channel. integer values will select the single input, starting at 0. \
any fractional part will mix the channel and the next one. the number in sel will be wrapped around \
to the number of inputs.\n\n\
<in x y> is the yth input of channel x.\n\
<out x> is the output of channel x.\
";
	return const_cast<char*>(r);
}


CSmodule_SelectInNN* CSmodule_SelectInNN::newOne()
{
	return new CSmodule_SelectInNN();
}


void CSmodule_SelectInNN::init()
{
	gname = copyString("signal");
	bname = copyString("multiplex in (NxN)");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_sel = &(createInput("sel","sel")->value);

	char n[128],n1[128];
	for (int i=0;i<CSMOD_MAX_CHAN*CSMOD_MAX_CHAN;i++)
	{
		sprintf(n, "in%d",i);
		sprintf(n1, "in%d %d",i/CSMOD_MAX_CHAN, i%CSMOD_MAX_CHAN);
		__in[i] = createInput(n,n1);
		_in[i] = &(__in[i]->value);
	}
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n, "out%d",i);
		sprintf(n1, "out%d",i);
		__out[i] = createOutput(n,n1);
		_out[i] = &(__out[i]->value);
	}

	setNrIn(2,2);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrchan", "nr of channels", nrChan, 1,CSMOD_MAX_CHAN);
	createPropertyInt("nrin", "nr of inputs", nrIn, 2,CSMOD_MAX_CHAN);
}


void CSmodule_SelectInNN::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue,nrChan);
	else
	if (strcmp(prop->name,"nrchan")==0)
		setNrIn(nrIn, prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_SelectInNN::setNrIn(int newNrIn, int newNrChan)
{
	nrIn = newNrIn;
	nrChan = newNrChan;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__out[i], (i<nrChan));
	}
	for (int i=0;i<CSMOD_MAX_CHAN*CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i],
			(i/CSMOD_MAX_CHAN<nrChan) && (i%CSMOD_MAX_CHAN<nrIn));
	}

	arrangeConnectors();
}


void CSmodule_SelectInNN::step()
{
	// select channel
	csfloat f = fabs(*_sel);
	f = MODULO(f, nrIn);
	unsigned int c = (unsigned int)f;

	// transmit
	for (int j=0;j<nrChan;j++)
		*_out[j] = *_in[j*CSMOD_MAX_CHAN+c];


	// fade fractional part
	f = f-c;
	if (f>0.0)
	{
		c = (c+1)% nrIn;

		for (int j=0;j<nrChan;j++)
			*_out[j] = *_out[j] * (1.0-f) + f * *_in[j*CSMOD_MAX_CHAN+c];
	}
}
