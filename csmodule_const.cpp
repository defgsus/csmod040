#include "csmodule_const.h"

char *CSmodule_Const::docString()
{
	const char *r =
"this module can be used to supply a fixed value to other modules or to scale and/or offset it's input.\n\
<value> is the actual value that you want to set or modulate.\n\
<amplitude> is a multiplier for that value.\n\
<offset> is added to the multiplied value before passed to the output.";
	return const_cast<char*>(r);
}

CSmodule_Const* CSmodule_Const::newOne()
{
	return new CSmodule_Const();
}


void CSmodule_Const::init()
{
	gname = copyString("math");
	bname = copyString("constant");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_in = &(createInput("v","value")->value);
	_amp = &(createInput("a","amplitude",1.0)->value);
	_off = &(createInput("o","offset")->value);

	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

}


/*void CSmodule_Const::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}*/

void CSmodule_Const::step()
{
	*_out = (*_in) * (*_amp) + (*_off);
}
