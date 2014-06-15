/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: 2D SCOPE / TIMELINE
 *
 * PURPOSE:
 * display a number of input channels through time
 *
 * @version 2010/10/15 v0.1
 * @author def.gsus-
 *
 */

#ifndef CSMODULE_SCOPE2D_H_INCLUDED
#define CSMODULE_SCOPE2D_H_INCLUDED

#include "csmodule.h"

class CSmodule_Scope2d: public CSmodule
{
	// pointers for easy value acces
	csfloat
		*_amp,
		*_offset,
		*_width,
		*_rst,
		*_in[CSMOD_MAX_CHAN],

		lrst
		;

	CSmoduleConnector
		*__in[CSMOD_MAX_CHAN],
		*__rst;

	int
		offset,
		nrIn, oldNrIn;

	// list of timelines
	CStimeline
		**scope;

	int
		// scope rect
		sx,sy,sw,sh,
		// different colors
		scopeColor[CSMOD_MAX_CHAN];

	float
		// data write position
		dpos;

	public:
	char *docString();
	~CSmodule_Scope2d();
	CSmodule_Scope2d* newOne();
	void init();
	void setNrIn(int newNrIn);
	void propertyCallback(CSmodProperty *prop);

	void step();

	virtual void draw(int offx=0, int offy=0, float zoom=1.0);

};




#endif // CSMODULE_SCOPE2D_H_INCLUDED
