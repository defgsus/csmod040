/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: some useful objects as modules
 *
 * PURPOSE: calculation speed
 *
 * @version 2011/03/15 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_OBJECTS_H_INCLUDED
#define CSMODULE_OBJECTS_H_INCLUDED

#include "csmodule.h"


#define CSMOD_STARS_MAX 10000

class CSmodule_StarSphere: public CSmodule
{
	public:
	csfloat
		*_nrStars,
		*_rad,
		*_varirad,
		*_bright,
		*_rst, lrst,

		*_x,
		*_y,
		*_z,
		*_br,

		pos[CSMOD_STARS_MAX<<2];

	unsigned int count;

	char *docString();
	CSmodule_StarSphere *newOne();

	void init();

	void step();

};


class CSmodule_PointMatrix: public CSmodule
{
	public:
	csfloat
		*_nrX,
		*_nrY,
		*_nrZ,
		*_width,
		*_rst, lrst,
		*_gate, lgate,

		*_x,
		*_y,
		*_z;

	CSmoduleConnector
		*__gate;

	int
		countx,
		county,
		countz;

	char *docString();
	CSmodule_PointMatrix *newOne();

	void init();

	void step();

};



#define CSMOD_GHM_MAX 32000

class CSmodule_GHM: public CSmodule
{
	public:
	csfloat
		*_scale,
		*_rst, lrst,
		*_gate, lgate,

		*_x,
		*_y,
		*_z,
		*_q,
		*_nr,

		pos[CSMOD_GHM_MAX<<2];

	CSmoduleConnector
		*__gate;

	unsigned int
		nr,
		count,
		x1,y1,z1,x2,y2,z2;

	char *docString();
	CSmodule_GHM *newOne();

	void propertyCallback(CSmodProperty *prop);
	void calcPos(int x1, int y1, int z1, int x2, int y2, int z2);

	void init();

	void step();

};




class CSmodule_Cube: public CSmodule
{
	public:
	csfloat
		*_phase,
		*_scaleX,
		*_scaleY,
		*_scaleZ,

		*_x,
		*_y,
		*_z;

	char *docString();
	CSmodule_Cube *newOne();

	void init();

	void step();

};


#endif // CSMODULE_OBJECTS_H_INCLUDED
