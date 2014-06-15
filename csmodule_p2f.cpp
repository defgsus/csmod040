#include "math.h"
#include "csmodule_p2f.h"

char *CSmodule_P2F::docString()
{
	const char *r = "\
this module converts a note number to it's corresponding frequency.\n\n\
<note> is the input number which is quantized (stepped) by the <quantize> value.\n\
the output frequency is <root freq> for <note> = 0, and double as high every <notes per oct>ave.\n\
<notes per oct> has a lower limit of 0.1.\
the input <note>''s sign is ignored and it's range is limited by the <modulo> value.\
";
	return const_cast<char*>(r);
}


CSmodule_P2F* CSmodule_P2F::newOne()
{
	return new CSmodule_P2F();
}


void CSmodule_P2F::init()
{
	gname = copyString("midi");
	bname = copyString("note to freq");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_note = &(createInput("n","note")->value);
	_quant = &(createInput("q","quantize",1.0)->value);
	_noteoct = &(createInput("no","notes per oct",12.0)->value);
	_modulo = &(createInput("mo","modulo", 128.0)->value);
	_rootfreq = &(createInput("ro","root freq (C-0)",16.0)->value);

	_out = &(createOutput("out","frequency")->value);

	// ---- properties ----

	createNameProperty();

	lnoteoct = -1;

}

void CSmodule_P2F::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}


// 292
// 125-145
void CSmodule_P2F::step()
{
	// quantize
	csfloat n =	(*_quant!=0.0)?
		(csfloat)((int)( fabs(*_note) / *_quant )) * *_quant
		:
		fabs(*_note);

	// get base for potence
	if (lnoteoct!=*_noteoct)
	{
		base = pow(2.0, 1.0 / max((csfloat)0.01, *_noteoct));
	}
	lnoteoct = *_noteoct;

	// get frequency
	*_out = *_rootfreq * pow( base, MODULO(n, max((csfloat)1, fabs(*_modulo)) ) );

}













char *CSmodule_Quant::docString()
{
	const char *r = "\
the input signal is quantized (or stepped) by the <quantize> value. can be also thought of as a bit reduction.\n\n\
<gate> will be 1.0 shortly, when the output changes it's value.\
";
	return const_cast<char*>(r);
}


CSmodule_Quant* CSmodule_Quant::newOne()
{
	return new CSmodule_Quant();
}


void CSmodule_Quant::init()
{
	gname = copyString("signal");
	bname = copyString("quantizer");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_quant = &(createInput("q","quantize",0.5)->value);

	_out = &(createOutput("out","output")->value);
	_gate = &(createOutput("gt","gate")->value);

	// ---- properties ----

	createNameProperty();

	lout = 0;
}

void CSmodule_Quant::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}


void CSmodule_Quant::step()
{
	// quantize
	*_out = (*_quant!=0.0)?
		(csfloat)((int)( *_in / *_quant )) * *_quant
		:
		*_in;

	if (*_out!=lout) *_gate = 1.0; else *_gate = 0.0;
	lout = *_out;
}
