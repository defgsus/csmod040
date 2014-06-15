#ifdef CSMOD_USE_GL

#include "math.h"

#include "csmodcontainer.h"
#include "csmodule_scopegl.h"

char *CSmodule_ScopeGl::docString()
{
	const char *r = "\
";
	return const_cast<char*>(r);
}

CSmodule_ScopeGl::~CSmodule_ScopeGl()
{
	if (img) free(img);
}

CSmodule_ScopeGl* CSmodule_ScopeGl::newOne()
{
	return new CSmodule_ScopeGl();
}


void CSmodule_ScopeGl::init()
{
	gname = copyString("visual");
	bname = copyString("scope GL");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_X = &(createInput("x","X")->value);
	_Y = &(createInput("y","Y")->value);
	_Z = &(createInput("z","Z")->value);
	_red = &(createInput("red","red")->value);
	_green = &(createInput("green","green")->value);
	_blue = &(createInput("blue","blue")->value);

	_resetInput = &(createInput("rst","reset input")->value);
	_setInput = &(createInput("set","set input")->value);

	_nrObj = &(createInput("nro","nr objects",100)->value);
	_dist = &(createInput("di","distance",1.0)->value);
	_rotX = &(createInput("rx","rot X")->value);
	_rotY = &(createInput("ry","rot Y")->value);
	_rotZ = &(createInput("rz","rot Z")->value);
	_zdist = &(createInput("zd","distance",1.0)->value);
	_zoom = &(createInput("zoo","size",1.0)->value);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("camera", "camera (persp/fisheye)", 0, 0, 1);
	createPropertyInt("w", "buffer width", 640, 1, 4096);
	createPropertyInt("h", "buffer height", 640, 1, 4096);

	// ---- image and window ----

	lresetInput = 0.0;
	lsetInput = 0.0;
	inputPos = 0.0;

	img = 0;
	setImage(640,640);

	// init the arrays
	memset(coord, CSMOD_SCOPEGL_MAX*3, sizeof(csfloat));
	memset(color, CSMOD_SCOPEGL_MAX*3, sizeof(csfloat));

	createWindow();

}

void CSmodule_ScopeGl::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"w")==0)
		setImage(prop->ivalue, h);
	else
	if (strcmp(prop->name,"h")==0)
		setImage(w,prop->ivalue);
	else CSmodule::propertyCallback(prop);
}

void CSmodule_ScopeGl::setWindowCaption()
{
	if (window)
	{
		char wl[512];
		sprintf(wl, "scope GL (window %dx%d) (buffer %dx%d)",
			window->w(), window->h(), w,h);
		window->label(wl);
	}
}

void CSmodule_ScopeGl::setImage(int w,int h)
{
	setWindowCaption();

	CSwait();
	ready = false;

	this->w = w;
	this->h = h;
	if (img) free(img);
	img = (uchar*) calloc((w*h)<<2, sizeof(uchar));

	setWindowCaption();

	ready = true;
}



void CSmodule_ScopeGl::step()
{
	// reset input pos
	if ((lresetInput<=0.0)&&(*_resetInput>0.0))
	{
		inputPos=0;
	}
	lresetInput = *_resetInput;

	// set a coordinate
	if ((lsetInput<=0.0)&&(*_setInput>0.0))
	{
		int k = inputPos * 3;
		coord[k] = *_X;
		coord[k+1] = *_Y;
		coord[k+2] = *_Z;

		color[k] = *_red;
		color[k+1] = *_green;
		color[k+2] = *_blue;

		inputPos++;
		if ((inputPos>=CSMOD_SCOPEGL_MAX)||(inputPos>=*_nrObj)) inputPos = 0;
	}
	lsetInput = *_setInput;
}



void CSmodule_ScopeGl::onResizeWindow(int newW, int newH)
{
	setImage(newW, newH);
}



void CSmodule_ScopeGl::drawWindow()
{
	// be sure user don't changes the buffer-size
	// due window resizing during this function
	isStep = true;

	//fl_draw_image(img, 0,0,w,h,4);

	/* ----------- */ gl_start(); /* ----------- */

	// clear buffer

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(0.2,1,0.01,10000);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);


	// ---- draw things ----
	glPushMatrix();


	// set in middle of screen
	glTranslatef((float)w/2,(float)h/2,0);
	glScalef(h,h,h);

	// distance
	glTranslatef(0,0,*_dist);

	// rotation
	glRotatef(*_rotX,1,0,0);
	glRotatef(*_rotY,0,1,0);
	glRotatef(*_rotZ,0,0,1);

	float r = 0.02;
	for (int i=0;i<*_nrObj;i++)
	{
		int k=i*3;

		glColor3f(1.0,0.0,0.0);
		glPushMatrix();
		glTranslatef(0,0,coord[k+2]);
		glColor3f(color[k],color[k+1],color[k+2]);
		glRectf(coord[k]-r,coord[k+1]-r,coord[k]+r,coord[k+1]+r);
		glPopMatrix();

	}

	glPopMatrix();

	/* ----------- */ gl_finish(); /* ----------- */


	isStep = false;
}

void CSmodule_ScopeGl::drawAndSaveWindow(const char * /*filename*/)
{
	/*
	// be sure user don't changes the buffer-size
	// due window resizing during this function
	isStep = true;

	storePNG(filename, img, w,h, 4);
	processBuffer();

	isStep = false;
	*/
}

#endif // #ifdef CSMOD_USE_GL
