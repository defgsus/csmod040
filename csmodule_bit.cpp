#include "csmodule_bit.h"

char *CSmodule_BitIn::docString()
{
	const char *r = "\
";
	return const_cast<char*>(r);
}


CSmodule_BitIn* CSmodule_BitIn::newOne()
{
	return new CSmodule_BitIn();
}


void CSmodule_BitIn::init()
{
	gname = copyString("logic");
	bname = copyString("bit in");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_out = &(createOutput("out","output")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		char *n1 = int2char("bit ",i);
		__in[i] = createInput(n,n1);
		free(n); free(n1);
		_in[i] = &(__in[i]->value);
	}

	oldNrIn = 0;
	setNrIn(4);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of bits", nrIn, 2,CSMOD_MAX_CHAN);
}


void CSmodule_BitIn::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_BitIn::setNrIn(int newNrIn)
{
	if (newNrIn==oldNrIn) return;
	nrIn = newNrIn;
	oldNrIn = nrIn;

	for (int i=0;i<CSMOD_BIT_MAX;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
	}

	arrangeConnectors();
}


void CSmodule_BitIn::step()
{
	unsigned int k = 1, o = 0;
	for (int i=0;i<nrIn;i++)
	{
		if ((*_in[i]>=0.5)||(*_in[i]<=-0.5)) o = o | k;
		k = k << 1;
	}
	*_out = o;
}


























char *CSmodule_BitOut::docString()
{
	const char *r = "\
binary decomposer\
";
	return const_cast<char*>(r);
}


CSmodule_BitOut* CSmodule_BitOut::newOne()
{
	return new CSmodule_BitOut();
}


void CSmodule_BitOut::init()
{
	gname = copyString("logic");
	bname = copyString("bit out");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("out",i);
		char *n1 = int2char("bit ",i);
		__out[i] = createOutput(n,n1);
		free(n); free(n1);
		_out[i] = &(__out[i]->value);
	}

	oldNrOut = 0;
	setNrOut(4);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrout", "nr of bits", nrOut, 2,CSMOD_BIT_MAX);
}


void CSmodule_BitOut::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_BitOut::setNrOut(int newNrOut)
{
	if (newNrOut==oldNrOut) return;
	nrOut = newNrOut;
	oldNrOut = nrOut;

	for (int i=0;i<CSMOD_BIT_MAX;i++)
	{
		setConnectorActive(__out[i], (i<nrOut));
	}

	arrangeConnectors();
}


void CSmodule_BitOut::step()
{
	int k=1;
	for (int i=0;i<nrOut;i++)
	{
		*_out[i] = (((unsigned int)*_in & k) != 0);
		k = k << 1;
	}
}
