/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Compare
 *
 * PURPOSE:
 * compare numbers / logics
 *
 * @version 2010/10/29 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_COMP_H_INCLUDED
#define CSMODULE_COMP_H_INCLUDED

#include "csmodule.h"

class CSmodule_Comp: public CSmodule
{
	csfloat
		*_A,
		*_B,
		*_oAeB,
		*_oAnB,
		*_oAgB,
		*_oAsB;

	public:
	char *docString();
	CSmodule_Comp *newOne();
	void init();

	void step();

};



#endif // CSMODULE_COMP_H_INCLUDED
