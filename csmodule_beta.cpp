#include "math.h"
#include "csmodule_beta.h"

char *CSmodule_Beta::docString()
{
	const char *r = "\
	the beta term \"bends\" a half circle/sphere onto a linear function.\n\n\
	the number of dimensions can be set in the properties.\n\
	<radius> will be the radius of the slice/circle/sphere around zero.\
";
	return const_cast<char*>(r);
}


CSmodule_Beta* CSmodule_Beta::newOne()
{
	return new CSmodule_Beta();
}


void CSmodule_Beta::init()
{
	gname = copyString("math");
	bname = copyString("beta term");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_rad = &(createInput("rad","radius",1.0)->value);

	__X = createInput("x","X");
	_X = &(__X->value);
	__Y = createInput("y","Y");
	_Y = &(__Y->value);
	__Z = createInput("z","Z");
	_Z = &(__Z->value);

	__out = createOutput("oz","Z");
	_out = &(__out->value);

	setDim(2);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrdim", "nr of input dimensions", 2, 1,3);

}

void CSmodule_Beta::propertyCallback(CSmodProperty *prop)
{
	if (!strcmp("nrdim",prop->name))
		setDim(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_Beta::setDim(int nrDim)
{
	if (nrDim<2)
	{
		__out->uname[0]='Y';
		__Y->active = false;
		__Z->active = false;
		arrangeConnectors();
		return;
	}

	__Y->active = true;

	if (nrDim==2)
	{
		__out->uname[0]='Z';
		__Z->active = false;
		arrangeConnectors();
	return;
	}

	__out->uname[0]='4';
	__Z->active = true;
	arrangeConnectors();

}

void CSmodule_Beta::step()
{
	// radius square
	csfloat r = *_rad * *_rad;
	// exit if zero
	if (r==0.0) { *_out = 0.0; return; }
	// get beta
	csfloat beta = *_X * *_X;
	if (__Y->active) beta += *_Y * *_Y;
	if (__Z->active) beta += *_Z * *_Z;
	beta = 1.0 - beta / r;
	// exit if not squareroot-able
	if (beta<=0.0) { *_out = 0.0; return; }
	*_out = *_rad * sqrtf(beta);
}



