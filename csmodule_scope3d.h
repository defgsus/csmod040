/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Scope3D
 *
 * PURPOSE:
 * three dimension scope & visualization magic!
 *
 * @version 2010/10/18 v0.1
 * @version 2011/02/13 v0.2 server-client connections via strings
 * @version 2011/03/18 v0.3 opacity / subsamples delay
 * @version 2011/04/01 v0.4 _AbstractDisplay to manage buffers
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_SCOPE3D_H_INCLUDED
#define CSMODULE_SCOPE3D_H_INCLUDED

#include "csmodule.h"
#include "csmodule_display.h"

#define CSMOD_SCOPE3D_MAX_DELAY_SHIFT 15
#define CSMOD_SCOPE3D_MAX_DELAY (1<<CSMOD_SCOPE3D_MAX_DELAY_SHIFT)

class CSmodule_Scope3d: public CSmodule_AbstractDisplay
{
	csfloat
		*_X,
		*_Y,
		*_Z,
		*_rotX,
		*_rotY,
		*_rotZ,
		*_zdist,
		*_zoom,
		*_persp,
		*_sub,
		*_suba,
		*_subsam,
		*_red,
		*_green,
		*_blue,
		*_bright,
		*_opac,
		*_rad,
		*_fade,
		*_smth,
		*_smthr,
		*_smthg,
		*_smthb,

		*_outUpd,
		*_outx,
		*_outy,
		*_outz;

	CSmoduleConnector
		*__fade,
		*__smth,
		*__smthr,
		*__smthg,
		*__smthb;

	int
		cameraMode,
		interpolateMode,
		sendTheBuffer;
	unsigned int
		/** last sampled pixel position */
		lx[CSMOD_SCOPE3D_MAX_DELAY],
		ly[CSMOD_SCOPE3D_MAX_DELAY],
		bufpos;
	bool
		/** true when this pixel was samppled */
		lastPointSampled[CSMOD_SCOPE3D_MAX_DELAY];
	bool
		wasScreenUpdate;

	public:
	char *docString();
	CSmodule_Scope3d *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	/** overrides AbstractDisplay's to update the module inputs */
	void connectToServer(const char *serverName);

	void step();

	void drawPixel(	csfloat x1, csfloat y1,
					unsigned int *lx, unsigned int *ly, unsigned int ulx, unsigned int uly,
					bool *lastPointSampled,
					csfloat sub, csfloat suba,
					short int red, short int green, short int blue,
					short unsigned int rad, int opac);

	/** apply fadeout / interpolation */
	void processBuffer();

	void drawWindow();
	void drawAndSaveWindow(const char *filename);

	void draw(int offx=0, int offy=0, float zoom=1.0);

};


#endif // CSMODULE_SCOPE3D_H_INCLUDED
