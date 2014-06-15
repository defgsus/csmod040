/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Rotate
 *
 * PURPOSE:
 * rotate an X,Y pair around Z
 *
 * @version 2010/10/14 v0.1
 * @version 2011/03/10 _View7
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_ROTATE_H_INCLUDED
#define CSMODULE_ROTATE_H_INCLUDED

#include "csmodule.h"


class CSmodule_Rotate: public CSmodule
{
	csfloat
		*_X,
		*_Y,
		*_rotZ,
		*_outX,
		*_outY;

	public:
	char *docString();
	CSmodule_Rotate* newOne();
	void init();

	void step();

};




class CSmodule_RotateAxis: public CSmodule
{
	csfloat
		*_X,
		*_Y,
		*_Z,
		*_aX,
		*_aY,
		*_aZ,
		*_ang,

		*_outX,
		*_outY,
		*_outZ;

	public:
	char *docString();
	CSmodule_RotateAxis* newOne();
	void init();

	void step();

};



class CSmodule_View7: public CSmodule
{
	public:

	csfloat
		*_rotZ,
		*_rotY,
		*_rotX,
		*_dist,
		*_rotZ1,
		*_rotY1,
		*_rotX1,
		*_X[CSMOD_MAX_CHAN],
		*_Y[CSMOD_MAX_CHAN],
		*_Z[CSMOD_MAX_CHAN],
		*_outX[CSMOD_MAX_CHAN],
		*_outY[CSMOD_MAX_CHAN],
		*_outZ[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__X[CSMOD_MAX_CHAN],
		*__Y[CSMOD_MAX_CHAN],
		*__Z[CSMOD_MAX_CHAN],
		*__outX[CSMOD_MAX_CHAN],
		*__outY[CSMOD_MAX_CHAN],
		*__outZ[CSMOD_MAX_CHAN];

	int nrIn;

	char *docString();
	CSmodule_View7* newOne();
	void init();

	void propertyCallback(CSmodProperty *prop);
	void setNrIn(int newNrIn);

	void step();

};

#endif // CSMODULE_ROTATE_H_INCLUDED
