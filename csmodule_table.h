/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Table
 *
 * PURPOSE:
 * output painted sequence (interpolated)
 *
 * @version 2010/11/07 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_TABLE_H_INCLUDED
#define CSMODULE_TABLE_H_INCLUDED

#include "csmodule_seq.h"


class CSmodule_Table: public CSmodule_Seq
{
	csfloat
		*_pos,

		*_outL,
		*_outS,
		*_outC,

		lpos;

	CSmoduleConnector
		*outL,
		*outS,
		*outC;

	int bankEdit;

	public:
	char *docString();
	CSmodule_Table *newOne();

	void init();

	void getValue(int bank, csfloat *_out, csfloat *_outL, csfloat *_outS, csfloat *_outC);
	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);
};


#endif // CSMODULE_TABLE_H_INCLUDED
