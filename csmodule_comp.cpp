#include "math.h"
#include "csmodule_comp.h"

char *CSmodule_Comp::docString()
{
	const char *r = "\
the module compares two inputs, <A> and <B> and outputs 0.0 (false) or 1.0 (true) for the following \
comparisons:\n\n\
A equals B\n\
A not equals B\n\
A is greater than B\n\
A is less than B\
";
	return const_cast<char*>(r);
}


CSmodule_Comp* CSmodule_Comp::newOne()
{
	return new CSmodule_Comp();
}


void CSmodule_Comp::init()
{
	gname = copyString("logic");
	bname = copyString("compare");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_A = &(createInput("a","A")->value);
	_B = &(createInput("b","B")->value);

	_oAeB = &(createOutput("oe","A = B")->value);
	_oAnB = &(createOutput("on","A != B")->value);
	_oAgB = &(createOutput("og","A > B")->value);
	_oAsB = &(createOutput("os","A < B")->value);

	// ---- properties ----

	createNameProperty();

}



void CSmodule_Comp::step()
{
	if (*_A > *_B)
	{
		*_oAeB = 0.0;
		*_oAnB = 1.0;
		*_oAgB = 1.0;
		*_oAsB = 0.0;
		return;
	}
	else
	if (*_A < *_B)
	{
		*_oAeB = 0.0;
		*_oAnB = 1.0;
		*_oAgB = 0.0;
		*_oAsB = 1.0;
		return;
	}
	else
	{
		*_oAeB = 1.0;
		*_oAnB = 0.0;
		*_oAgB = 0.0;
		*_oAsB = 0.0;
	}
}




