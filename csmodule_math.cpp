#include <math.h>
#include "csmodule_math.h"

char *CSmodule_Math::docString()
{
	const char *r = "\
the module adds, substracts, multiplies or divides a variable number of inputs to one output. \
the math operation can be changed in the module properties.\n\n\
division by zero is catched, x / 0 = 0 !!\
";
	return const_cast<char*>(r);
}

CSmodule_Math* CSmodule_Math::newOne()
{
	return new CSmodule_Math();
}


void CSmodule_Math::init()
{
	gname = copyString("math");
	bname = copyString("math");
	name = copyString("math add");
	uname = copyString(bname);

	// --- inputs outputs ----

	_out = &(createOutput("out","output")->value);

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		__in[i] = createInput(n,"in");
		free(n);
		_in[i] = &(__in[i]->value);
	}

	oldNrIn = 0;
	setNrIn(2);

	mop = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("mop", "operation (0-3,add,sub,mul,div)", mop, 0,3);
	createPropertyInt("nrin", "nr of inputs", nrIn, 2,CSMOD_MAX_CHAN);

	setMop(mop);
}

void CSmodule_Math::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"mop")==0)
		setMop(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Math::setNrIn(int newNrIn)
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

void CSmodule_Math::setMop(int newMop)
{
	mop = newMop;
	int k = getPropertyIndex("mop");
	if (k>=0) property[k]->ivalue = mop;

	if (con[0]->uname) free(con[0]->uname);
	switch (mop)
	{
		default: con[0]->uname = copyString("add"); break;
		case 1: con[0]->uname = copyString("sub"); break;
		case 2: con[0]->uname = copyString("mul"); break;
		case 3: con[0]->uname = copyString("div"); break;
	}
}


void CSmodule_Math::step()
{
	// start with first input
	*_out = *_in[0];

	// operate the rest
	switch (mop)
	{
		default: // add
			for (int i=1;i<nrIn;i++)
				*_out += *_in[i];
			break;
		case 1: // sub
			for (int i=1;i<nrIn;i++)
				*_out -= *_in[i];
			break;
		case 2: // mul
			for (int i=1;i<nrIn;i++)
				*_out *= *_in[i];
			break;
		case 3: // div
			//if (*_out==0.0) break;
			for (int i=1;i<nrIn;i++)
			{
				if (*_in[i]==0.0) { *_out = 0.0; break; }
				*_out /= *_in[i];
			}
			break;
	}
}












CSmodule_MathSub* CSmodule_MathSub::newOne()
{
	return new CSmodule_MathSub();
}


void CSmodule_MathSub::init()
{
	CSmodule_Math::init();
	if (name) free(name); name = copyString("math sub");
	setMop(1);
}



CSmodule_MathMul* CSmodule_MathMul::newOne()
{
	return new CSmodule_MathMul();
}


void CSmodule_MathMul::init()
{
	CSmodule_Math::init();
	if (name) free(name); name = copyString("math mul");
	setMop(2);
}



CSmodule_MathDiv* CSmodule_MathDiv::newOne()
{
	return new CSmodule_MathDiv();
}


void CSmodule_MathDiv::init()
{
	CSmodule_Math::init();
	if (name) free(name); name = copyString("math div");
	setMop(3);
}












char *CSmodule_Abs::docString()
{
	const char *r = "\
the module outputs the input without sign, f.e. -1.3 will be 1.3\
";
	return const_cast<char*>(r);
}

CSmodule_Abs* CSmodule_Abs::newOne()
{
	return new CSmodule_Abs();
}


void CSmodule_Abs::init()
{
	gname = copyString("math");
	bname = copyString("abs");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Abs::step()
{
	*_out = fabs( *_in );
}


















char *CSmodule_Sin2Cos::docString()
{
	const char *r = "\
the module adds, substracts, multiplies or divides a variable number of inputs to one output. \
the math operation can be changed in the module properties.\n\n\
division by zero is catched, x / 0 = 0 !!\
";
	return const_cast<char*>(r);
}

CSmodule_Sin2Cos::~CSmodule_Sin2Cos()
{
	if (_in) free(_in);
	if (_out) free(_out);
}

CSmodule_Sin2Cos* CSmodule_Sin2Cos::newOne()
{
	return new CSmodule_Sin2Cos();
}


void CSmodule_Sin2Cos::init()
{
	gname = copyString("math");
	bname = copyString("sin2cos");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_phase = &(createInput("p","phase")->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Sin2Cos::step()
{
	*_out = *_in - SQRT_2 * sin( (*_in + *_phase) * TWO_PI);
}














char *CSmodule_Modulo::docString()
{
	const char *r = "\
the module takes to input values <A> and <B> and outputs <A> modulo <B>, means A will always be \
<= (abs)B. works for negative A as well.\
";
	return const_cast<char*>(r);
}

CSmodule_Modulo* CSmodule_Modulo::newOne()
{
	return new CSmodule_Modulo();
}


void CSmodule_Modulo::init()
{
	gname = copyString("math");
	bname = copyString("modulo");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_A = &(createInput("a","A")->value);
	_B = &(createInput("b","B")->value);
	_out = &(createOutput("out","A mod B")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Modulo::step()
{
	*_out = MODULO(*_A, *_B);
}











char *CSmodule_Power::docString()
{
	const char *r = "\
the module calcultes <A> to the power of <B>.\
";
	return const_cast<char*>(r);
}

CSmodule_Power* CSmodule_Power::newOne()
{
	return new CSmodule_Power();
}


void CSmodule_Power::init()
{
	gname = copyString("math");
	bname = copyString("power");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_A = &(createInput("a","A")->value);
	_B = &(createInput("b","B")->value);
	_out = &(createOutput("out","A ^ B")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Power::step()
{
	*_out = powf(*_A, *_B);
	CSclipFloat(*_out);
}










char *CSmodule_Sin::docString()
{
	const char *r = "\
the module calcultes the sinus of <input> (normalized to 1.0).\
";
	return const_cast<char*>(r);
}

CSmodule_Sin* CSmodule_Sin::newOne()
{
	return new CSmodule_Sin();
}


void CSmodule_Sin::init()
{
	gname = copyString("math");
	bname = copyString("sin");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Sin::step()
{
	*_out = sin(*_in*TWO_PI);
}








char *CSmodule_Sin1::docString()
{
	const char *r = "\
the module calcultes the arcus sinus of <input> (normalized to 1.0).\
";
	return const_cast<char*>(r);
}

CSmodule_Sin1* CSmodule_Sin1::newOne()
{
	return new CSmodule_Sin1();
}


void CSmodule_Sin1::init()
{
	gname = copyString("math");
	bname = copyString("sin^-1");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Sin1::step()
{
	if ((*_in>1.0)||(*_in<-1.0))
		*_out = 0.0;
	else
		*_out = asin(*_in)/TWO_PI;
}




char *CSmodule_Dist::docString()
{
	const char *r = "\
the module calculates the distance between two vector sets. the number of dimensions is \
set in properties.\
";
	return const_cast<char*>(r);
}

CSmodule_Dist* CSmodule_Dist::newOne()
{
	return new CSmodule_Dist();
}


void CSmodule_Dist::init()
{
	gname = copyString("math");
	bname = copyString("distance");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_out = &(createOutput("out","distance")->value);

	char n[128],n1[128];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"ina%d",i);
		if (i<3)
			sprintf(n1,"%ca",i+88);
		else
			sprintf(n1,"V%da",i+1);
		__in1[i] = createInput(n,n1);
		_in1[i] = &(__in1[i]->value);
	}
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"inb%d",i);
		if (i<3)
			sprintf(n1,"%cb",i+88);
		else
			sprintf(n1,"V%db",i+1);
		__in2[i] = createInput(n,n1);
		_in2[i] = &(__in2[i]->value);
	}

	setNrIn(3);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of inputs", nrIn, 2,CSMOD_MAX_CHAN);

}

void CSmodule_Dist::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Dist::setNrIn(int newNrIn)
{
	nrIn = newNrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in1[i], (i<nrIn));
		setConnectorActive(__in2[i], (i<nrIn));
	}

	arrangeConnectors();
}



void CSmodule_Dist::step()
{
	*_out = 0.0;
	for (int i=0;i<nrIn;i++)
		*_out += (*_in2[i]-*_in1[i])*(*_in2[i]-*_in1[i]);
	if (*_out>0.0)
		*_out = sqrtf(*_out);
}











char *CSmodule_Mag::docString()
{
	const char *r = "\
the module calculates the magnitude of a set of vectors. the number of dimensions is \
set in properties.\
";
	return const_cast<char*>(r);
}

CSmodule_Mag* CSmodule_Mag::newOne()
{
	return new CSmodule_Mag();
}


void CSmodule_Mag::init()
{
	gname = copyString("math");
	bname = copyString("magnitude");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_out = &(createOutput("out","magnitude")->value);

	char n[128],n1[128];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"in%d",i);
		if (i<3)
			sprintf(n1,"%c",i+88);
		else
			sprintf(n1,"V%d",i+1);
		__in[i] = createInput(n,n1);
		_in[i] = &(__in[i]->value);
	}

	setNrIn(3);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of inputs", nrIn, 2,CSMOD_MAX_CHAN);

}

void CSmodule_Mag::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Mag::setNrIn(int newNrIn)
{
	nrIn = newNrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
	}

	arrangeConnectors();
}



void CSmodule_Mag::step()
{
	*_out = 0.0;
	for (int i=0;i<nrIn;i++)
		*_out += *_in[i] * *_in[i];
	if (*_out>0.0)
		*_out = sqrtf(*_out);
}


















char *CSmodule_DistFactor::docString()
{
	const char *r = "\
the module calculates the distance between two 3d vector sets and derives a nice reciprocal factor.\n\n\
the used formula is: amp * ( ( 1.0 / ( 1.0 + sqrt((x2-x1)^2 + (y2-y1)^2 + (z2-z1)^2) ) ) ^ porence )\n\n\
the first set of x,y,z positions are output unchanged by the module, just to introduce the one sample \
module delay that could be helpful when drawing very fast.\
";
	return const_cast<char*>(r);
}

CSmodule_DistFactor* CSmodule_DistFactor::newOne()
{
	return new CSmodule_DistFactor();
}


void CSmodule_DistFactor::init()
{
	gname = copyString("visual");
	bname = copyString("distance factor");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_x1 = &(createInput("x1","X1")->value);
	_y1 = &(createInput("y1","Y1")->value);
	_z1 = &(createInput("z1","Z1")->value);
	_x2 = &(createInput("x2","X2")->value);
	_y2 = &(createInput("y2","Y2")->value);
	_z2 = &(createInput("z2","Z2")->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);
	_pow = &(createInput("pow","potence",2.0)->value);

	_ox = &(createOutput("ox","X1")->value);
	_oy = &(createOutput("oy","Y1")->value);
	_oz = &(createOutput("oz","Z1")->value);
	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

}


void CSmodule_DistFactor::step()
{
	// pass through
	*_ox = *_x1;
	*_oy = *_y1;
	*_oz = *_z1;

	// get factor
	csfloat a = *_x2-*_x1;
	csfloat b = a*a;
	a = *_y2-*_y1;
	b += a*a;
	a = *_z2-*_z1;
	b += a*a;
	*_out = *_amp * pow( 1.0 / (1.0 + sqrt(b)), *_pow );
}


