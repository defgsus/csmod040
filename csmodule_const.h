/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Constant
 *
 * PURPOSE:
 * send a value, or scale/offset some input
 *
 * @version 2010/09/24 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_CONST_H_INCLUDED
#define CSMODULE_CONST_H_INCLUDED

#include "csmodule.h"


class CSmodule_Const: public CSmodule
{
	// pointers for easy value acces
	csfloat
		*_in,
		*_amp,
		*_off,
		*_out;

	public:
	char *docString();
	CSmodule_Const* newOne();
	void init();
	// void propertyCallback(CSmodProperty *prop)

	void step();

};

#endif // CSMODULE_CONST_H_INCLUDED
