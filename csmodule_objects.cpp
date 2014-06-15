#include <math.h>
#include "csmodule_objects.h"



char *CSmodule_StarSphere::docString()
{
	const char *r = "\
the module outputs a typical star sphere.\n\n\
for each audio sample, a precalculated (random) position and brightness is output. \
the positions all lay on a sphere of <radius> and the stars are clustering around the poles (+Y and -Y).\n\
<radius vari.> is the amount in which the radius of the sphere increases randomly. the brightness value for \
each star will be added to the actual <radius> by this amount, darkest stars having the greates radius increase.\n\
\n\
<reset> simply resets the internal counter and restarts the position/brightness sequence.\n\
the number of stars is limited to 10000. note that the number of stars that can be drawn during one screen \
update is determined by sampling rate / frames per second. so f.e. with 44100hz and 30hz screen update, the \
number of drawable stars is 1470.\n\
";
	return const_cast<char*>(r);
}


CSmodule_StarSphere* CSmodule_StarSphere::newOne()
{
	return new CSmodule_StarSphere();
}


void CSmodule_StarSphere::init()
{
	gname = copyString("objects");
	bname = copyString("star sphere");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_nrStars = &(createInput("nr","nr of stars",1000.0)->value);
	_rad = &(createInput("rad","radius",10.0)->value);
	_varirad = &(createInput("vrad","radius vari.",2.0)->value);
	_bright = &(createInput("br","bright",1.0)->value);
	_rst = &(createInput("rst","reset")->value);

	_x = &(createOutput("x","X")->value);
	_y = &(createOutput("y","Y")->value);
	_z = &(createOutput("z","Z")->value);
	_br = &(createOutput("obr","bright")->value);

	// ---- properties ----

	createNameProperty();

	// --- internals ---

	lrst = 0.0;
	count = 0;

	// pre calc stars
	csfloat x,y,z,ca,sa;
	for (int i=0;i<CSMOD_STARS_MAX;i++)
	{
		// random position on axis
		x = 0.002+0.998*(csfloat)rand()/RAND_MAX;
		// rotate around y
		ca = (csfloat)rand()/RAND_MAX*TWO_PI;
		sa = sin(ca);
		ca = cos(ca);
		z = x * sa;
		x = x * ca;

		// bend with beta term
		y = sqrtf(1.0 - (x*x + z*z)) * ((csfloat)(i%2)*2.0-1.0);

		pos[(i<<2)] = (csfloat)rand()/RAND_MAX;
		pos[(i<<2)+1] = x;
		pos[(i<<2)+2] = y;
		pos[(i<<2)+3] = z;
	}
}


void CSmodule_StarSphere::step()
{
	count++;
	if ((count>=CSMOD_STARS_MAX) || (count>=*_nrStars) ||
		((lrst<=0.0)&&(*_rst>0.0)) ) count = 0;
	lrst = *_rst;

	csfloat
		*p = &pos[count<<2],
		rad = *_rad + *_varirad * *p;
	*_br = *_bright * (1.0-0.9* *p); p++;
	*_x = rad * *p; p++;
	*_y = rad * *p; p++;
	*_z = rad * *p;
}

















char *CSmodule_PointMatrix::docString()
{
	const char *r = "\
the module outputs a 3d matrix of positions.\n\n\
for each audio sample, the next point in the cue is ouput (order: -x to x then -y to y and then -z to z). so \
the module can be also thought of as a three step counter.\n\n\
<scale> is a multiplier for the output x,y,z position.\n\
<reset> simply resets the internal counters and restarts the position sequence.\n\
if <gate> is connected the counter will move forward only when a gate receives at this input.\n\n\
the number of points is not limited. note however that the number of points that can be drawn during one screen \
update is determined by sampling rate / frames per second. so f.e. with 44100hz and 30hz screen update, the \
number of drawable points is 1470. since the positions run from - to +, the overall number of points is \
calculated by (nrx*2+1) * (nry*2+1) * (nrz*2+1).\
";
	return const_cast<char*>(r);
}


CSmodule_PointMatrix* CSmodule_PointMatrix::newOne()
{
	return new CSmodule_PointMatrix();
}


void CSmodule_PointMatrix::init()
{
	gname = copyString("objects");
	bname = copyString("point matrix");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_nrX = &(createInput("nrx","nr X",10.0)->value);
	_nrY = &(createInput("nry","nr Y",10.0)->value);
	_nrZ = &(createInput("nrz","nr Z",10.0)->value);
	_width = &(createInput("wi","scale",1.0)->value);
	_rst = &(createInput("rst","reset")->value);
	__gate = createInput("g","gate");
	_gate = &__gate->value;

	_x = &(createOutput("x","X")->value);
	_y = &(createOutput("y","Y")->value);
	_z = &(createOutput("z","Z")->value);

	// ---- properties ----

	createNameProperty();

	// --- internals ---

	lrst = lgate = 0.0;
	countx = -*_nrX;
	county = -*_nrY;
	countz = -*_nrZ;

}


void CSmodule_PointMatrix::step()
{
	// reset
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		countx = -*_nrX;
		county = -*_nrY;
		countz = -*_nrZ;
	}
	lrst = *_rst;

	*_x = *_width * countx;
	*_y = *_width * county;
	*_z = *_width * countz;

	// forward counter
	if ((!__gate->nrCon)||((lgate<=0.0)&&(*_gate>0.0)))
		countx++;
	lgate = *_gate;
	if (countx>*_nrX)
	{
		countx = -*_nrX;
		county++;
		if (county>*_nrY)
		{
			county = -*_nrY;
			countz++;
			if (countz>*_nrZ)
			{
				countz = -*_nrZ;
			}
		}
	}
}












char *CSmodule_GHM::docString()
{
	const char *r = "\
the module outputs positions and quotients of the Grand Harmonical Matrix.\n\n\
all integer x,y,z positions for which the term x/y/z (or x/z/y or any other permutation) yields an \
integer number, a point is set at that position (zero is ignored). \
the extent on the three axis is set in the properties.\n\n\
for each audio sample, the precalculated position and result of the calculation is output.\n\
if <gate> is connected the counter will move forward only when a gate receives at this input.\n\n\
<reset> simply resets the internal counter and restarts the position sequence.\n\
the number of points is limited to 32000. note that the number of points that can be drawn during one screen \
update is determined by sampling rate / frames per second. so f.e. with 44100hz and 30hz screen update, the \
number of drawable stars is 1470. since the calculation of the resulting number of points is fairly difficult, \
that number is output by the module.\
";
	return const_cast<char*>(r);
}


CSmodule_GHM* CSmodule_GHM::newOne()
{
	return new CSmodule_GHM();
}


void CSmodule_GHM::init()
{
	gname = copyString("objects");
	bname = copyString("GHM");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_scale = &(createInput("sc","scale",1.0)->value);
	_rst = &(createInput("rst","reset")->value);
	__gate = createInput("g","gate");
	_gate = &__gate->value;

	_x = &(createOutput("x","X")->value);
	_y = &(createOutput("y","Y")->value);
	_z = &(createOutput("z","Z")->value);
	_q = &(createOutput("q","Q")->value);
	_nr = &(createOutput("nr","nr")->value);

	// --- internals ---

	lrst = lgate = 0.0;
	count = 0;
	calcPos(-5,-5,-5,5,5,5);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("x1","x start", x1);
	createPropertyInt("y1","y start", y1);
	createPropertyInt("z1","z start", z1);
	createPropertyInt("x2","x end", x2);
	createPropertyInt("y2","x end", y2);
	createPropertyInt("z2","x end", z2);
}

void CSmodule_GHM::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"x1")==0)
		calcPos(prop->ivalue,y1,z1,x2,y2,z2);
	else
	if (strcmp(prop->name,"y1")==0)
		calcPos(x1,prop->ivalue,z1,x2,y2,z2);
	else
	if (strcmp(prop->name,"z1")==0)
		calcPos(x1,y1,prop->ivalue,x2,y2,z2);
	else
	if (strcmp(prop->name,"x2")==0)
		calcPos(x1,y1,z1,prop->ivalue,y2,z2);
	else
	if (strcmp(prop->name,"y2")==0)
		calcPos(x1,y1,z1,x2,prop->ivalue,z2);
	else
	if (strcmp(prop->name,"z2")==0)
		calcPos(x1,y1,z1,x2,y2,prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_GHM::calcPos(int x1, int y1, int z1, int x2, int y2, int z2)
{
	this->x1 = x1;
	this->y1 = y1;
	this->z1 = z1;
	this->x2 = x2;
	this->y2 = y2;
	this->z2 = z2;

	nr = 0;
	count = 0;
	float f;
	for (int z=z1;z<=z2;z++) if (z!=0)
	for (int y=y1;y<=y2;y++) if (y!=0)
	for (int x=x1;x<=x2;x++) if (x!=0)
	{
		// test if harmonic
		f = (float)x/y/z; if (f==(int)f) goto addPoint;
		f = (float)x/z/y; if (f==(int)f) goto addPoint;
		f = (float)y/x/z; if (f==(int)f) goto addPoint;
		f = (float)y/z/x; if (f==(int)f) goto addPoint;
		f = (float)y/x/z; if (f==(int)f) goto addPoint;
		f = (float)y/z/x; if (f==(int)f) goto addPoint;
		continue;

		addPoint:
		if (nr>=CSMOD_GHM_MAX) return;

		pos[(nr<<2)] = x;
		pos[(nr<<2)+1] = y;
		pos[(nr<<2)+2] = z;
		pos[(nr<<2)+3] = f;
		nr++;
	}
}


void CSmodule_GHM::step()
{
	csfloat *p = &pos[count<<2];
	*_x = *_scale * *p; p++;
	*_y = *_scale * *p; p++;
	*_z = *_scale * *p; p++;
	*_q = *p;
	*_nr = nr;

	if ((!__gate->nrCon)||((lgate<=0.0)&&(*_gate>0.0)))
		count++;
	lgate = *_gate;
	if ( (count>=nr) ||
		((lrst<=0.0)&&(*_rst>0.0)) ) count = 0;
	lrst = *_rst;

}


















char *CSmodule_Cube::docString()
{
	const char *r = "\
the module outputs x,y,z positions forming a cube.\n\n\
the cube is centered around 0,0,0 and extents <scale x/y/z> units in each direction (- to +).\n\
<phase> ranges from 0.0 to 1.0 and determines the position along the cube outline.\n\n\
the cube module is more for efficient painting and does not output a very nice 'sounding' wave. \
it's more like a composition of saw and square, with lots of discontinuities. the drawing sequence is:\n\
rear rectangle (-Z): top left, bottom left, bottom right, top right\n\
front rectangle (+Z): same as above.\n\
and then the four connections from rear to front, same order as above.\
";
	return const_cast<char*>(r);
}


CSmodule_Cube* CSmodule_Cube::newOne()
{
	return new CSmodule_Cube();
}


void CSmodule_Cube::init()
{
	gname = copyString("objects");
	bname = copyString("cube");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_phase = &(createInput("p","phase")->value);
	_scaleX = &(createInput("sx","scale x",1.0)->value);
	_scaleY = &(createInput("sy","scale y",1.0)->value);
	_scaleZ = &(createInput("sz","scale z",1.0)->value);

	_x = &(createOutput("x","X")->value);
	_y = &(createOutput("y","Y")->value);
	_z = &(createOutput("z","Z")->value);

	// ---- properties ----

	createNameProperty();
}



void CSmodule_Cube::step()
{
	csfloat p = MODULO(fabs(*_phase),1.0) * 12.0;

	// first rect
	if (p<1.0)
	{
		*_x = -*_scaleX;
		*_y = *_scaleY * (0.5-p)*2.0;
		*_z = -*_scaleZ;
	}
	else
	if (p<2.0)
	{
		p -= 1.0;
		*_x = -*_scaleX * (p-0.5)*2.0;
		*_y = -*_scaleY;
		*_z = -*_scaleZ;
	}
	else
	if (p<3.0)
	{
		p -= 2.0;
		*_x = *_scaleX;
		*_y = *_scaleY * (p-0.5)*2.0;
		*_z = -*_scaleZ;
	}
	else
	if (p<4.0)
	{
		p -= 3.0;
		*_x = *_scaleX * (0.5-p)*2.0;
		*_y = *_scaleY;
		*_z = -*_scaleZ;
	}
	else
	// second rect
	if (p<5.0)
	{
		p -= 4.0;
		*_x = -*_scaleX;
		*_y = *_scaleY * (0.5-p)*2.0;
		*_z = *_scaleZ;
	}
	else
	if (p<6.0)
	{
		p -= 5.0;
		*_x = -*_scaleX * (p-0.5)*2.0;
		*_y = -*_scaleY;
		*_z = *_scaleZ;
	}
	else
	if (p<7.0)
	{
		p -= 6.0;
		*_x = *_scaleX;
		*_y = *_scaleY * (p-0.5)*2.0;
		*_z = *_scaleZ;
	}
	else
	if (p<8.0)
	{
		p -= 7.0;
		*_x = *_scaleX * (0.5-p)*2.0;
		*_y = *_scaleY;
		*_z = *_scaleZ;
	}
	else
	// corner connect
	if (p<9.0)
	{
		p -= 8.0;
		*_x = -*_scaleX;
		*_y = *_scaleY;
		*_z = *_scaleZ * (p-0.5)*2.0;
	}
	else
	if (p<10.0)
	{
		p -= 9.0;
		*_x = -*_scaleX;
		*_y = -*_scaleY;
		*_z = *_scaleZ * (p-0.5)*2.0;
	}
	else
	if (p<11.0)
	{
		p -= 10.0;
		*_x = *_scaleX;
		*_y = -*_scaleY;
		*_z = *_scaleZ * (p-0.5)*2.0;
	}
	else
	{
		p -= 11.0;
		*_x = *_scaleX;
		*_y = *_scaleY;
		*_z = *_scaleZ * (p-0.5)*2.0;
	}


}

