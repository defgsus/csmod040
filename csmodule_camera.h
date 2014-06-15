/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Camera
 *
 * PURPOSE:
 * translate X,Y,Z coordinates in a meaningful way
 *
 * @version 2010/11/05 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_CAMERA_H_INCLUDED
#define CSMODULE_CAMERA_H_INCLUDED

#include "csmodule.h"

class CSmodule_Camera: public CSmodule
{
	csfloat
		*_x,
		*_y,
		*_z,
		*_posx,
		*_posy,
		*_posz,
		*_lookx,
		*_looky,
		*_lookz,
		*_upx,
		*_upy,
		*_upz,
		*_ox,
		*_oy,
		*_oz;

	public:
	char *docString();
	CSmodule_Camera *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void vnormalize(csfloat *x,csfloat *y,csfloat *z);

	void step();

};






class CSmodule_Fisheye: public CSmodule
{
	csfloat
		*_x,
		*_y,
		*_z,
		*_ox,
		*_oy;

	public:
	char *docString();
	CSmodule_Fisheye *newOne();
	void init();

	void step();

};






class CSmodule_Hsb2Rgb: public CSmodule
{
	csfloat
		*_H,
		*_S,
		*_B,
		*_r,
		*_g,
		*_b;

	public:
	char *docString();
	CSmodule_Hsb2Rgb *newOne();
	void init();

	void step();

};


#endif // CSMODULE_CAMERA_H_INCLUDED
