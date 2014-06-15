/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: ScopeGL
 *
 * PURPOSE:
 *
 * @version 2011/02/26 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SCOPEGL_H_INCLUDED
#define CSMODULE_SCOPEGL_H_INCLUDED

#include "csmodule.h"

#include <FL/gl.H>
#include <GL/glu.h>

#define CSMOD_SCOPEGL_MAX 0x10000

class CSmodule_ScopeGl: public CSmodule
{
	csfloat
		*_resetInput,	lresetInput,
		*_setInput,		lsetInput,
		*_X,
		*_Y,
		*_Z,
		*_red,
		*_green,
		*_blue,

		*_nrObj,
		*_dist,
		*_rotX,
		*_rotY,
		*_rotZ,
		*_zdist,
		*_zoom;

	uchar
		*img;

	float
		coord[CSMOD_SCOPEGL_MAX*3],
		color[CSMOD_SCOPEGL_MAX*3];

	unsigned int
		inputPos,
		w,h,
		lx,ly;
	bool
		wasScreenUpdate;

	public:
	char *docString();
	~CSmodule_ScopeGl();
	CSmodule_ScopeGl *newOne();

	void init();
	void propertyCallback(CSmodProperty *prop);
	void setWindowCaption();
	void setImage(int w,int h);

	void step();
	void onResizeWindow(int newW, int newH);
	void drawWindow();

	void drawAndSaveWindow(const char *filename);

};



#endif // CSMODULE_SCOPEGL_H_INCLUDED
