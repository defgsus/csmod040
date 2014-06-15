#include "math.h"

#include "csmodcontainer.h"
#include "csmodule_scope3d.h"

char *CSmodule_Scope3d::docString()
{
	const char *r = "\
this module displays a 3-dimensional oscilloscope on a fading screen. the image buffer is 8 bit rgb, and \
all values are clipped to fit into 8 bit. it's like a camera projection of a 3 dimensional space. \
it will draw it's graphics on an external window which can also be rendered to disk.\
\n\n\
<X>, <Y> and <Z> are the offsets of a light particle on each axis, where 0.0 is the center of screen.\n\
the resulting form can be rotated about it's origin with <rot X>, <rot Y> and <rot Z>, \
and the origin can be shifted away from the camera in z direction with a positive <distance> value.\n\
the scale of the form is controlled by <size> and the amount of perspective is changed with \
the <perspective> value ranging from 0.0 (no perspective) to 2.0 (very distorted), a 'naturalistic' \
value beeing something like 0.5. note that a pespective value of 0.0 removes completely the influence of \
the z vector, so <distance> has no effect at all.\n\
\n\
<bright> (between -1.0 and 1.0) sets the intensity of the particle, while <red>, <green> and <blue> \
determine the color (between -1.0 and 1.0). negativ brightness or color is subtracted from the image pixel.\n\
<opacity> can make the color value for the pixel absolute. normally the color (red/green/blue * brightness) is \
added to the image buffer. <opacity> defines the mix between the added pixel and the absolute color value \
(between 0.0 and 1.0).\n\
\n\
<radius> (integer) *currently* is half the side-length of a square that is drawn around the pixel position, if \
<radius> is greater than 1. (computational expensive)\n\
n\
the whole screen is dampened by the <fade out> value after every screen update. numbers greater than 0 \
fade the screen to black, numbers smaller than 0 fade to white. \
(f.e. 0.0 = no fade out, 1.0 = instantly black (after screen update), -0.01 = slowly fade to white). \
note that the actual time till complete fade-out depends on the frequency of the screen-update \
(as fast as possible in real-time and userdefined in disk-render mode).\n\
\n\
if <subsamples> >= 1 then this number of additional points of same color and brightness will be drawn \
to interpolate between the current and the last point. currently the interpolation method is linear. \
the last point will only be sampled when on screen (and in front of camera). there will be no subsample \
drawing for points that got outside screen, to avoid a mess.\n\
<subsamples (auto)> greater 0.0 will tell the module to determine the number of subsamples to draw per pixel \
automatically. if <subsamples (auto)> is 1.0 then the number of subsamples is equal to the distance-on-screen \
between the last and the current point. 2.0 will be twice as much, aso. the number of actual drawn subsamples \
is <subsamples> + the value determined by <subsamples (auto)>.\n\
NOTE: for efficiency reasons the subsamples are drawn between points-on-screen rather than points-in-space. \
that means that perspective distortions in fisheye-mode will NOT be calculated for the subsamples. currently \
subsamples connect points always in a straight line.\n\
<subsamples (delay)> defines, which lastly sampled point to connect with. 0 (or 1) means: use the last sample, \
2 means use the second-last sample, aso. that way it is possible to use the subsamples feature even if you \
draw multiple objects on one scope by switching very fast between the objects.\n\
\n\
if <interpolate> is greater than zero, then the screen will also be blurred/pixel interpolated before it is \
faded. the number in <interpolate> sets the interpolation intensity between 0.0 and 1.0. \
each color channel of each pixel is crossfaded between itself and the average of the sourrounding color channels. \
note that this will eat a good amount of additional computation time.\n\
<interpolate red/green/blue> are multipliers of the <interpolate> value for each individual color channel. \
so interpolation can be switched off or increased for a certain color.\n\
\n\
camera mode can be selected in the properties. it can be normal perspective camera (0) which simply puts \
x and y on its place while using the z distance as an perspective modifier. in fisheye mode (1) \
it will apply a 180 degrees pinhole/fisheye/spherical projection, which can be used to project on a dome. \
note that in fisheye mode the <perspective> value should be close to zero, since it will introduce a strong and \
probably unrealistic additional perspective distortion.\n\
\n\
CLIENT/SERVER modes:\n\
you can connect other scope3d modules to a server scope3d and thus display many light dots at a time. \
the scope3d which should act as the server should be given a name in the properties. type in some name in \
the <server name> field and create other scope3ds and type in the server name in their <connect to server> \
field. the client scope3ds will send any drawing request to the server scope3d. the client modules will turn \
red if you specify a <connect to server> name that does not exist in the patch. the external windows of the \
client scopes are not used and you can close them. inputs controlling the behaviour of the screen, like fade out \
and stuff will disappear. besides that, the client scope can be used just as the normal server scope, \
including normal/fisheye perspective and stuff.\n\n\
another client mode can be set in the properties with <send image buffer>. if the scope is already in \
client mode as described above, this setting will force the client scope to draw on it's own buffer but to add \
the buffer to the server image buffer on every screen update. that way you can mix different settings of fade-out \
and interpolation on one screen. (internally the each channel of the buffer is simply added to the server buffer. \
the added client buffer always starts in the top-left corner. it's size is independent of the server buffer.\n\
\n\
next thing is, a scope can be a server AND a client at the same time. a client can draw on a server scope, and \
this server scope can send it's image buffer over to another server. (so an image buffer of a server can be sent \
further to another server. BUT: drawing is always done on the requested server and are not piped further.)\
";
	return const_cast<char*>(r);
}

CSmodule_Scope3d* CSmodule_Scope3d::newOne()
{
	return new CSmodule_Scope3d();
}

void CSmodule_Scope3d::init()
{

	gname = copyString("visual");
	bname = copyString("scope 3d");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	headerHeight += 4;

	// --- inputs outputs ----

	_X = &(createInput("x","X")->value);
	_Y = &(createInput("y","Y")->value);
	_Z = &(createInput("z","Z")->value);
	_rotX = &(createInput("rx","rot X")->value);
	_rotY = &(createInput("ry","rot Y")->value);
	_rotZ = &(createInput("rz","rot Z")->value);
	_zdist = &(createInput("zd","distance",1.0)->value);
	_zoom = &(createInput("zoo","size",1.0)->value);
	_persp = &(createInput("per","perspective",0.5)->value);
	_red = &(createInput("r","red",1.0)->value);
	_green = &(createInput("g","green",1.0)->value);
	_blue = &(createInput("b","blue",1.0)->value);
	_bright = &(createInput("br","bright",0.5)->value);
	_opac = &(createInput("opc","opacity")->value);
	_rad = &(createInput("ra","radius",1.0)->value);
	__fade = createInput("f","fade out",0.1);
	_fade = &(__fade->value);
	_sub = &(createInput("sub","subsamples")->value);
	_suba = &(createInput("suba","subsamples (auto)")->value);
	_subsam = &(createInput("subsam","subsamples (delay)")->value);
	__smth = createInput("smt","interpolate");
	_smth = &(__smth->value);
	__smthr = createInput("smtr","interpolate red",1.0);
	_smthr = &(__smthr->value);
	__smthg = createInput("smtg","interpolate green",1.0);
	_smthg = &(__smthg->value);
	__smthb = createInput("smtb","interpolate blue",1.0);
	_smthb = &(__smthb->value);

	_outUpd = &(createOutput("outr","refresh")->value);
	_outx = &(createOutput("outx","X")->value);
	_outy = &(createOutput("outy","Y")->value);
	_outz = &(createOutput("outz","Z")->value);

	cameraMode = 0;
	interpolateMode = 0;
	sendTheBuffer = 0;

	// ---- properties ----

	CSmodule_AbstractDisplay::init();

	createPropertyInt("camera", "camera (persp/fisheye)", cameraMode, 0, 1);
	createPropertyInt("imode", "interpolate mode (0=mix/1=add)", interpolateMode, 0, 1);
	createPropertyInt("sendbuf", "send image buffer (0/1)?", sendTheBuffer, 0, 1);

	// ---- image and window ----

	createWindow();

	oldBodyColor = bodyColor;
	oldBodySelColor = bodySelColor;

	wasScreenUpdate = false;

}

void CSmodule_Scope3d::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"camera")==0)
		cameraMode = prop->ivalue;
	else
	if (strcmp(prop->name,"imode")==0)
		interpolateMode = prop->ivalue;
	else
	if (strcmp(prop->name,"sendbuf")==0)
	{
		sendTheBuffer = prop->ivalue;
		int i=getPropertyIndex("clientstr");
		if (i) connectToServer(property[i]->svalue);
	}
	else CSmodule_AbstractDisplay::propertyCallback(prop);
}


void CSmodule_Scope3d::connectToServer(const char *serverName)
{
	CSmodule_AbstractDisplay::connectToServer(serverName);

	bool act = (!isClient) || (sendTheBuffer!=0);
	setConnectorActive(__fade, act);
	setConnectorActive(__smth, act);
	setConnectorActive(__smthr, act);
	setConnectorActive(__smthg, act);
	setConnectorActive(__smthb, act);
	arrangeConnectors();

	if (!isClient)
	{
		doDrawWindow = true;
		doCallDrawWindow = false;
	}
	else
	{
		//window->hide(); FLTK only makes difficulties with window focus
		doDrawWindow = false;
		doCallDrawWindow = (sendTheBuffer!=0);
	}
}


void CSmodule_Scope3d::step()
{
	// change the server if nescessary
	CSmodule_AbstractDisplay::step();

	// send screen refresh gate
	*_outUpd = (csfloat)wasScreenUpdate;
	wasScreenUpdate = false;

	unsigned int obufpos = bufpos;
	// always increase bufpos, no matter pixel is drawn or not
	bufpos = (bufpos + 1) & (CSMOD_SCOPE3D_MAX_DELAY-1);

	// calculate subsample delay buffer position
	unsigned int bp =
		(obufpos+CSMOD_SCOPE3D_MAX_DELAY - max(1,min(CSMOD_SCOPE3D_MAX_DELAY-1, (int)*_subsam)) )
		& (CSMOD_SCOPE3D_MAX_DELAY-1);

	// first assume pixel gets not on screen
	lastPointSampled[obufpos] = false;

	csfloat ca,sa,x1,y1,z1,bu, R, S, phi, theta;

	// rotation around Z
	if (*_rotZ!=0.0)
	{
		ca = *_rotZ * TWO_PI;
		sa = sin(ca);
		ca = cos(ca);
		x1 = ca * *_X - sa * *_Y;
		y1 = sa * *_X + ca * *_Y;
	} else { x1 = *_X ; y1 = *_Y; }

	// rotation around Y
	if (*_rotY!=0.0)
	{
		ca = *_rotY * TWO_PI;
		sa = sin(ca);
		ca = cos(ca);
		bu = x1 * ca + *_Z * sa;
		z1 = x1 * sa - *_Z * ca;
		x1 = bu;
	} else { z1 = *_Z; }

	// rotation around X
	if (*_rotX!=0.0)
	{
		ca = *_rotX * TWO_PI;
		sa = sin(ca);
		ca = cos(ca);
		bu = y1 * ca - z1 * sa;
		z1 = y1 * sa + z1 * ca + *_zdist;
		y1 = bu;
	} else { z1 += *_zdist; }

	// feed output
	*_outz = z1;

	// position behind camera?
    if (z1<=0.0) { return; }


    // now -> x1, y1, z1 = position in space in front of camera

    switch (cameraMode)
    {
    	case 0: // normal perspective

			// perspective (hack), keep factor in z1
			bu = min((csfloat)2.0,max((csfloat)0.0, *_persp));
			z1 = 1.0/ ( z1*bu*(1.0 + max((csfloat)0.0, bu-0.5f)*2.0*z1) + (1.0-bu) );

			z1 *= *_zoom;
			x1 *= z1;
			y1 *= z1;

			break;

		case 1: // dome/fisheye perspective

			// perspective (hack)
			bu = min((csfloat)2.0,max((csfloat)0.0, *_persp));
			bu = 1.0/ ( z1*bu*(1.0 + max((csfloat)0.0, bu-0.5f)*2.0*z1) + (1.0-bu) );

			x1 *= bu * *_zoom;
			y1 *= bu * *_zoom;

			// cart->sphere
			S = x1*x1 + y1*y1;
			R = sqrtf(S + z1*z1);
			S = sqrtf(S);
			phi = sin( acosf(z1/R) );
			if (S!=0.0)
			{
				if (x1>=0)
					theta = asinf(y1/S);
				else
					theta = PI - asin(y1/S);
			}
			else
			{
				if (x1>=0)
					theta = asinf(y1);
				else
					theta = PI - asin(y1);
			}
			// sphere->cart
			x1 = phi * cos(theta);
			y1 = phi * sin(theta);

			break;
    }

	// feed output
	*_outx = x1;
	*_outy = y1;

	if ((!isClient)||(sendTheBuffer))
    drawPixel(
		// point on screen (-1,1)
		x1,
		y1,
		// remember last point on screen
		&lx[obufpos], &ly[obufpos],
		lx[bp], ly[bp],
		&lastPointSampled[obufpos],
		// sub-samples
		(lastPointSampled[bp])? *_sub:0,
		(lastPointSampled[bp])? *_suba:0,
		// colors in 8bit
		255 * (*_bright * *_red) ,
		255 * (*_bright * *_green) ,
		255 * (*_bright * *_blue) ,
		min((csfloat)50, *_rad),
		max(0,min(1<<8, (int)(*_opac*(1<<8)) ))
		);

	else
	if (server)
	{
		CSmodule_Scope3d *Server = dynamic_cast<CSmodule_Scope3d*>(server);
		if (Server)
		// draw on different scope
		Server->drawPixel(
				// point on screen (-1,1)
				x1,
				y1,
				// remember last point on screen
				&lx[obufpos], &ly[obufpos],
				lx[bp], ly[bp],
				&lastPointSampled[obufpos],
				// sub-samples
				(lastPointSampled[bp])? *_sub:0,
				(lastPointSampled[bp])? *_suba:0,
				// colors in 8bit
				255 * (*_bright * *_red) ,
				255 * (*_bright * *_green) ,
				255 * (*_bright * *_blue),
				min((csfloat)50, *_rad),
				max(0,min(1<<8, (int)(*_opac*(1<<8)) ))
			);
	}

}

// 2654/1800
void CSmodule_Scope3d::drawPixel(csfloat x1, csfloat y1,
					unsigned int *lx, unsigned int *ly, unsigned int ulx, unsigned int uly,
					bool *lastPointSampled,
					csfloat sub, csfloat suba,
					short int red, short int green, short int blue,
					short unsigned int rad, int opac)
{
	/*
	// send further if this server is a client
	if (clientMode)
		return drawPixel(x1,y1,lx,ly,ulx,uly,sub,suba,red,green,blue,rad,opac);
	*/

	if (!ready) return;

	bool oldIsStep = isStep;
	isStep = true;

	// get on-screen coordinates (-1,-1 is bottom-left)
	unsigned int x = (unsigned int)(w + x1 * h) >> 1;
	if (x>=w) return;
	unsigned int y = (unsigned int)(h - y1 * h) >> 1;
	if (y>=h) return;


	// --- set the point(s) ---

	uchar *p,*pp;
	int
		op=(1<<8)-opac,
		opacr = opac * red,
		opacg = opac * green,
		opacb = opac * blue;
	// set a pixel (bounded)
	#define setPp(x,y) \
	if ((x<w)&&(y<h)) { \
		p = &img[((y)*w+(x))<<2]; \
		pp = p; \
		*p = max(0, min(255, (short int)*p + red)); p++; \
		*p = max(0, min(255, (short int)*p + green)); p++; \
		*p = max(0, min(255, (short int)*p + blue)); \
		if (opac>0) \
		{ 	p = pp;\
			*p = max(0, min(255, ( (int)*p * op + opacr )>>8 )); p++; \
			*p = max(0, min(255, ( (int)*p * op + opacg )>>8 )); p++; \
			*p = max(0, min(255, ( (int)*p * op + opacb )>>8 )); \
		} \
	}

	// set a square / or pixel
	#define setP(x,y) \
	if (rad>1) \
	{ \
		for (int jj=-rad;jj<=rad;jj++) \
			for (int ii=-rad;ii<=rad;ii++) \
				{ setPp(x+ii,y+jj); } \
	} \
	else \
	{ \
		p = &img[((y)*w+(x))<<2]; \
		pp = p; \
		*p = max(0, min(255, (short int)*p + red)); p++; \
		*p = max(0, min(255, (short int)*p + green)); p++; \
		*p = max(0, min(255, (short int)*p + blue)); \
		if (opac>0) \
		{ 	p = pp;\
			*p = max(0, min(255, ( (int)*p * op + opacr )>>8 )); p++; \
			*p = max(0, min(255, ( (int)*p * op + opacg )>>8 )); p++; \
			*p = max(0, min(255, ( (int)*p * op + opacb )>>8 )); \
		} \
	}

	setP(x,y);


	// sample last point

	if (suba>0)
		sub += suba * sqrt((*lx-x)*(*lx-x) + (*ly-y)*(*ly-y));

	// sample this point
	*lx = x;
	*ly = y;
	*lastPointSampled = true;

	// draw interpolated points
	if ((sub<1.0)) return;

	csfloat
		dx = (csfloat)ulx-x,
		dy = (csfloat)uly-y;

	x1 = x;
	y1 = y;
	csfloat sa = sub+1.0;
	csfloat ca = dx/sa;
	sa = dy/sa;
	for (int i=0;i<sub;i++)
	{
		x1 += ca;
		y1 += sa;
		x = x1; y = y1;
		if ((x<w)&&(y<h))
		{
			setP(x,y);
		}
	}

	return;

	isStep = oldIsStep;
}




void CSmodule_Scope3d::processBuffer()
{
	wasScreenUpdate = true;

	uchar *p,*pp;
	short int fade;
	unsigned int k;

	// interpolate
	if (*_smth>0.0)
	{
		int
			sr = max(0,min(255, (int)( *_smth * *_smthr * 255) )),
			sr1 = 255-sr,
			sg = max(0,min(255, (int)( *_smth * *_smthg * 255) )),
			sg1 = 255-sg,
			sb = max(0,min(255, (int)( *_smth * *_smthb * 255) )),
			sb1 = 255-sb;

			/** !!TODO!! make it fast */

			#define getpix(off)   \
				((img[p1-4+off] \
				+img[p1+off]   \
				+img[p1+4+off] \
				+img[p2-4+off] \
				+(img[p2+off]<<3)\
				+img[p2+4+off] \
				+img[p3-4+off] \
				+img[p3+off]   \
				+img[p3+4+off])>>4)

		switch (interpolateMode)
		{
			case 0: // mix
				for (unsigned int j=1;j<h-1;j++)
				for (unsigned int i=1;i<w-1;i++)
				{
					unsigned int i1 = i<<2;
					unsigned int p1 = i1 + ( ((j-1)*w) << 2 );
					unsigned int p2 = i1 + ( (j*w) << 2 );
					unsigned int p3 = i1 + ( ((j+1)*w) << 2 );

					img[p2] = ( img[p2]*sr1 + sr*getpix(0) ) >> 8; p2++;
					img[p2]	= ( img[p2]*sg1 + sg*getpix(1) ) >> 8; p2++;
					img[p2]	= ( img[p2]*sb1 + sb*getpix(2) ) >> 8;
				}

				// fade out
				p = img;
				fade = ( 255 * *_fade );
				k = (w*h)<<2;
				for (unsigned int i=0;i<k;i++)
				{
					*p = max(0, min(255, *p - fade ));
					p++;
				}
				/** !TODO!
					fades out the edge pixels cause they wont be interpolated right now */
				p = img; // first line
				pp = img + ((w*(h-1))<<2); // last line
				for (unsigned int i=0;i<w<<2;i++)
				{
					*p = max(0, *p - 5); p++;
					*pp = max(0, *pp - 5); pp++;
				}
				p = img; // first column
				pp = img + ((w-1)<<2); // last column
				for (unsigned int i=0;i<h;i++)
				{
					*p = max(0, *p - 5); p++;
					*p = max(0, *p - 5); p++;
					*p = max(0, *p - 5); p+=(w<<2)-2;
					*pp = max(0, *pp - 5); pp++;
					*pp = max(0, *pp - 5); pp++;
					*pp = max(0, *pp - 5); pp+=(w<<2)-2;
				}
			break;

			case 1: // add
				fade = ( 255 * *_fade );
				for (unsigned int j=1;j<h-1;j++)
				for (unsigned int i=1;i<w-1;i++)
				{
					unsigned int i1 = i<<2;
					unsigned int p1 = i1 + ( ((j-1)*w) << 2 );
					unsigned int p2 = i1 + ( (j*w) << 2 );
					unsigned int p3 = i1 + ( ((j+1)*w) << 2 );

					img[p2]		= max(0,min(255, (int)(img[p2]   -fade+ ((sr*getpix(0))>>8) )));
					img[p2+1]	= max(0,min(255, (int)(img[p2+1] -fade+ ((sg*getpix(1))>>8) )));
					img[p2+2]	= max(0,min(255, (int)(img[p2+2] -fade+ ((sb*getpix(2))>>8) )));
				}
			break;
		}
	}

	else // simply fade out
	{
		p = img;
		fade = ( 255 * *_fade );
		k = (w*h)<<2;
		for (unsigned int i=0;i<k;i++)
		{
			*p = max(0, min(255, *p - fade ));
			p++;
		}
	}

}


void CSmodule_Scope3d::drawWindow()
{
	if (isClient && server && sendTheBuffer && (!requestServerChange))
	{
		processBuffer();
		CSmodule_Scope3d *Server = dynamic_cast<CSmodule_Scope3d*>(server);
		if (Server) Server->addBuffer(img, w, h);
		return;
	}

	// dont draw clients
	if (isClient) return;

	// be sure user don't changes the buffer-size
	// due window resizing during this function
	isStep = true;

	fl_draw_image(img, 0,0,w,h,4);
	processBuffer();

	isStep = false;
}

void CSmodule_Scope3d::drawAndSaveWindow(const char *filename)
{
	if (isClient && server && sendTheBuffer && (!requestServerChange))
	{
		processBuffer();
		CSmodule_Scope3d *Server = dynamic_cast<CSmodule_Scope3d*>(server);
		if (Server) Server->addBuffer(img, w, h);
		return;
	}

	// dont save client buffers
	if (isClient) return;

	// be sure user don't changes the buffer-size
	// due window resizing during this function
	isStep = true;

	storePNG(filename, img, w,h, 4);
	processBuffer();

	isStep = false;
}


void CSmodule_Scope3d::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);
	if (isServer)
	{
		fl_font(FL_HELVETICA_ITALIC, 9*zoom);
		fl_color(FL_GREEN);
		fl_draw("server", offx+zoom*(x+width-30),offy+zoom*(y+9));
	}
	if (isClient)
	{
		fl_font(FL_HELVETICA_ITALIC, 9*zoom);
		fl_color(FL_RED);
		fl_draw("client", offx+zoom*(x+width-25),offy+zoom*(y+headerHeight-4));
	}
}
