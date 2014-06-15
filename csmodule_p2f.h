/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE:
 *		Pitch (Note) to Frequency
 *		Quantizer
 *
 * PURPOSE:
 * render a note value into a frequency value for variable notes per oktave
 *
 * @version 2010/10/27 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_P2F_H_INCLUDED
#define CSMODULE_P2F_H_INCLUDED

#include "csmodule.h"

class CSmodule_P2F: public CSmodule
{
	csfloat
		*_note,
		*_quant,
		*_noteoct,
		*_rootfreq,
		*_modulo,

		*_out,

		base,
		lnoteoct;

	public:
	char *docString();
	CSmodule_P2F *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};


class CSmodule_Quant: public CSmodule
{
	csfloat
		*_in,
		*_quant,

		*_out,
		*_gate,

		lout;

	public:
	char *docString();
	CSmodule_Quant *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};

#endif // CSMODULE_P2F_H_INCLUDED
