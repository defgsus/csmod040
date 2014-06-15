/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Cellular Automaton
 *
 * PURPOSE:
 *  game of life
 *
 * @version 2011/02/23 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_CA_H_INCLUDED
#define CSMODULE_CA_H_INCLUDED

#include "csmodule.h"

#define CSMOD_MAX_CAWIDTH 128
#define CSMOD_MAX_CASIZE (CSMOD_MAX_CAWIDTH * CSMOD_MAX_CAWIDTH)
#define CSMOD_CA_SHIFT 7

class CSmodule_CA: public CSmodule
{
	csfloat
		*_writeGate,
		lwriteGate,
		*_in,
		*_inx,
		*_iny,
		*_resetMap,
		lresetMap,
		*_stepGate,
		lstepGate,
		*_outx,
		*_outy,

		*_b[9],
		*_s[9],

		*_out,
		*_outn,
		*_wi,
		*_he,

		map[CSMOD_MAX_CASIZE],
		buffer[CSMOD_MAX_CASIZE];

	int
		wi,
		he,
		sx,sy,sw,sh;

	public:
	char *docString();
	CSmodule_CA* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void step();

	void fillMap(csfloat value);
	void getNeighbours();

	void draw(int offx=0, int offy=0, float zoom=1.0);

};


#endif // CSMODULE_CA_H_INCLUDED
