/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Clipping Modules
 *
 * PURPOSE:
 *	hard and soft clipping modules
 *
 * @version 2010/10/28 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_CLIP_H_INCLUDED
#define CSMODULE_CLIP_H_INCLUDED

#include "csmodule.h"

class CSmodule_ClipSoft: public CSmodule
{
	csfloat

		*_in,
		*_out;

	public:
	char *docString();
	CSmodule_ClipSoft *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};



class CSmodule_ClipHard: public CSmodule
{
	csfloat

		*_in,
		*_min,
		*_max,
		*_out;

	public:
	char *docString();
	CSmodule_ClipHard *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

};

#endif // CSMODULE_CLIP_H_INCLUDED
