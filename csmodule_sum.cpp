#include "math.h"
#include "csmodule_sum.h"

char *CSmodule_Sum::docString()
{
	const char *r = "\
to be described by his ten-ness...\
";
	return const_cast<char*>(r);
}


CSmodule_Sum* CSmodule_Sum::newOne()
{
	return new CSmodule_Sum();
}


void CSmodule_Sum::init()
{
	gname = copyString("math");
	bname = copyString("sum matrix");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_amp = &(createInput("amp","amplitude",1.0)->value);
	_off = &(createInput("off","offset")->value);
	_X = &(createInput("x","X")->value);
	_Y = &(createInput("y","Y")->value);
	_A = &(createOutput("a","-X -Y")->value);
	_B = &(createOutput("b","+X -Y")->value);
	_C = &(createOutput("c","+X +Y")->value);
	_D = &(createOutput("d","-X +Y")->value);

	// ---- properties ----

	createNameProperty();

}

void CSmodule_Sum::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_Sum::step()
{
	*_A = *_off + *_amp * ( -*_X -*_Y );
	*_B = *_off + *_amp * ( +*_X -*_Y );
	*_C = *_off + *_amp * ( +*_X +*_Y );
	*_D = *_off + *_amp * ( -*_X +*_Y );
}

















char *CSmodule_QuadPos::docString()
{
	const char *r = "\
the module outputs four amplitudes for a given input angle on a circle.\n\
\n\
<position> is the angle on a circle between 0.0 and 1.0, where 0.0 is front, 0.25 is right, 0.5 is \
rear and 0.75 is left. the value is continous, so f.e. -1.2 is the same as 2.8 or 50.8 ...\n\
<amplitude> is multiplied to each output which normally is in the range of 0.0 and 1.0.\n\
<offset> is added to each output.\n\
\n\n\
the outputs are amplitudes for a quadro speaker setup.\
";
	return const_cast<char*>(r);
}


CSmodule_QuadPos* CSmodule_QuadPos::newOne()
{
	return new CSmodule_QuadPos();
}


void CSmodule_QuadPos::init()
{
	gname = copyString("math");
	bname = copyString("quadro circle");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_ang = &(createInput("a","position")->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);
	_off = &(createInput("off","offset",0.0)->value);
	_A = &(createOutput("a","rear left")->value);
	_B = &(createOutput("b","rear right")->value);
	_C = &(createOutput("c","front right")->value);
	_D = &(createOutput("d","front left")->value);

	// ---- properties ----

	createNameProperty();

}

void CSmodule_QuadPos::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_QuadPos::step()
{
	csfloat a = (*_ang + 0.625) * TWO_PI;

	*_A = *_off + *_amp * (0.5 + 0.5 * sin( a ));
	*_B = *_off + *_amp * (0.5 + 0.5 * sin( a + HALF_PI));
	*_C = *_off + *_amp * (0.5 + 0.5 * sin( a + PI ));
	*_D = *_off + *_amp * (0.5 + 0.5 * sin( a + PI + HALF_PI ));
}














char *CSmodule_QuadMix::docString()
{
	const char *r = "\
this module pans an input signal for a rectangular speaker setup for a given input angle on a circle. \
it can have several input/angle pairs which are mixed together.\n\
\n\
<amplitude> is multiplied to each output.\n\
<offset> is added to each output.\n\
<input x> is the signal to be panned\n\
<position x> is the angle on a circle between 0.0 and 1.0, where 0.0 is front, 0.25 is right, 0.5 is \
rear and 0.75 is left. the value is continous, so f.e. -1.2 is the same as 2.8 or 50.8 ...\n\
\n\n\
the number of inputs can be set in the properties view.\
";
	return const_cast<char*>(r);
}


CSmodule_QuadMix* CSmodule_QuadMix::newOne()
{
	return new CSmodule_QuadMix();
}


void CSmodule_QuadMix::init()
{
	gname = copyString("math");
	bname = copyString("quadro circle mix");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_amp = &(createInput("amp","amplitude",1.0)->value);
	_off = &(createInput("off","offset",0.0)->value);
	_A = &(createOutput("a","rear left")->value);
	_B = &(createOutput("b","rear right")->value);
	_C = &(createOutput("c","front right")->value);
	_D = &(createOutput("d","front left")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		char *n1 = int2char("input ",i+1);
		__in[i] = createInput(n,n1);
		free(n); free(n1);
		_in[i] = &(__in[i]->value);

		n = int2char("ang",i);
		n1 = int2char("position ",i+1);
		__ang[i] = createInput(n,n1);
		free(n); free(n1);
		_ang[i] = &(__ang[i]->value);
	}

	oldNrIn = 0;
	setNrIn(1);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of inputs", nrIn, 1,CSMOD_MAX_CHAN);

}

void CSmodule_QuadMix::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_QuadMix::setNrIn(int newNrIn)
{
	if (newNrIn==oldNrIn) return;
	nrIn = newNrIn;
	oldNrIn = nrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
		setConnectorActive(__ang[i], (i<nrIn));
	}

	arrangeConnectors();
}



void CSmodule_QuadMix::step()
{
	// init outputs
	*_A = *_B = *_C = *_D = 0.0;


	for (int i=0;i<nrIn;i++)
	{
		csfloat a = (*_ang[i] + 0.625) * TWO_PI;

		*_A += *_in[i] * ( *_off + *_amp * (0.5 + 0.5 * sin( a )) );
		*_B += *_in[i] * ( *_off + *_amp * (0.5 + 0.5 * sin( a + HALF_PI)) );
		*_C += *_in[i] * ( *_off + *_amp * (0.5 + 0.5 * sin( a + PI )) );
		*_D += *_in[i] * ( *_off + *_amp * (0.5 + 0.5 * sin( a + PI + HALF_PI )) );
	}
}





















char *CSmodule_QuadMixMod::docString()
{
	const char *r = "\
this module pans an input signal for a rectangular speaker setup for a given input angle on a circle. \
it can have several input/angle pairs which are mixed together.\n\
\n\
<amplitude> is multiplied to each output.\n\
<offset> is added to each output.\n\
<modulation amt> defines the influence of the panning. 0.0 to 1.0 crossfade between no panning and full \
panning. above 1.0 it serves as the potence of the panning curve which is sinusoidal at 1.0.\n\
\n\
<input x> is the signal to be panned\n\
<position x> is the angle on a circle between 0.0 and 1.0, where 0.0 is front, 0.25 is right, 0.5 is \
rear and 0.75 is left. the value is continous, so f.e. -1.2 is the same as 2.8 or 50.8 ...\n\
\n\
the number of inputs can be set in the properties view.\
";
	return const_cast<char*>(r);
}


CSmodule_QuadMixMod* CSmodule_QuadMixMod::newOne()
{
	return new CSmodule_QuadMixMod();
}


void CSmodule_QuadMixMod::init()
{
	CSmodule_QuadMix::init();

	free(bname); bname = copyString("quadro circle mod mix");
	free(name); name = copyString(bname);
	rename(bname);

	// --- inputs outputs ----

	_amt = &(insertInput(2,"amt","modulation amt",1.0)->value);

}

void CSmodule_QuadMixMod::step()
{
	// init outputs
	*_A = *_B = *_C = *_D = 0.0;

	*_amt = max(0, *_amt);

	#define shape(exp) ( (*_amt<1.0)? ( 1.0 - *_amt + *_amt * (exp) ) : ( pow(exp, *_amt) ) )


	for (int i=0;i<nrIn;i++)
	{
		csfloat a = (*_ang[i] + 0.625) * TWO_PI;

		*_A += *_in[i] * ( *_off + *_amp * shape(0.5 + 0.5 * sin( a )) );
		*_B += *_in[i] * ( *_off + *_amp * shape(0.5 + 0.5 * sin( a + HALF_PI)) );
		*_C += *_in[i] * ( *_off + *_amp * shape(0.5 + 0.5 * sin( a + PI )) );
		*_D += *_in[i] * ( *_off + *_amp * shape(0.5 + 0.5 * sin( a + PI + HALF_PI )) );
	}
}
