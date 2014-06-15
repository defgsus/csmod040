#include "math.h"
#include "csmodule_rotate.h"

char *CSmodule_Rotate::docString()
{
	const char *r = "\
this module can be used to rotate an X,Y - vector around the Z axis, or in other words \
it is a sinusoidal cross-fader between inputs <X> and <Y>.\n\
the amount of rotation is given in <rotate Z> from 0.0 (0 degree) to 1.0 (360 degree).\n\n\
of'course you can rotate around other axis as well with this module. to rotate around X axis: \
connect Y->X, Z->Y, to rotate around Y axis: X->X, Z->Y.\
";
	return const_cast<char*>(r);
}

CSmodule_Rotate* CSmodule_Rotate::newOne()
{
	return new CSmodule_Rotate();
}


void CSmodule_Rotate::init()
{
	gname = copyString("math");
	bname = copyString("rotate");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_X = &(createInput("x","X")->value);
	_Y = &(createInput("y","Y")->value);
	_rotZ = &(createInput("z","rotate Z")->value);

	_outX = &(createOutput("outx","X")->value);
	_outY = &(createOutput("outy","Y")->value);

	// ---- properties ----

	createNameProperty();

}



void CSmodule_Rotate::step()
{
	csfloat ca = *_rotZ * TWO_PI;
	csfloat sa = sin( ca );
			ca = cos( ca );
	*_outX = ca * *_X - sa * *_Y;
	*_outY = sa * *_X + ca * *_Y;
}












char *CSmodule_RotateAxis::docString()
{
	const char *r = "\
the module rotates a vector around an arbitrary axis.\n\n\
the vector is given in <X>, <Y>, <Z>.\n\
the axis runs from 0,0,0 to <axis X>, <axis Y>, <axis Z>.\n\
the rotation angle is given in <angle>, ranging from 0.0 to 1.0 (360 degree).\n\n\
NOTES: the module is computationally quite expensive!\n\
if no axis is given (all <axis> inputs 0), the rotation can not be calculated and the output \
will simply be the input.\
";
	return const_cast<char*>(r);
}

CSmodule_RotateAxis* CSmodule_RotateAxis::newOne()
{
	return new CSmodule_RotateAxis();
}


void CSmodule_RotateAxis::init()
{
	gname = copyString("math");
	bname = copyString("rotate axis");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_X = &(createInput("x","X")->value);
	_Y = &(createInput("y","Y")->value);
	_Z = &(createInput("z","Z")->value);
	_aX = &(createInput("ax","axis X")->value);
	_aY = &(createInput("ay","axis Y",1.0)->value);
	_aZ = &(createInput("az","axis Z")->value);
	_ang = &(createInput("a","angle")->value);

	_outX = &(createOutput("ox","X")->value);
	_outY = &(createOutput("oy","Y")->value);
	_outZ = &(createOutput("oz","Z")->value);

	// ---- properties ----

	createNameProperty();

}



void CSmodule_RotateAxis::step()
{
	csfloat CO = *_ang * TWO_PI;
	csfloat SI = sin(CO);
	CO = cos(CO);
	csfloat m = *_aX * *_aX + *_aY * *_aY + *_aZ * *_aZ;
	if (m==0.0) { *_outX = *_X; *_outY = *_Y; *_outZ = *_Z; return; }
	csfloat ms = sqrtf(m);

	*_outX = (*_aX* (*_aX * *_X + *_aY * *_Y + *_aZ * *_Z)
         + CO * ( *_X*(*_aY**_aY + *_aZ**_aZ) + *_aX*(-*_aY* *_Y - *_aZ* *_Z) )
         + SI * ms * (-*_aZ * *_Y + *_aY * *_Z) ) / m;
	*_outY = (*_aY* (*_aX * *_X + *_aY * *_Y + *_aZ * *_Z)
         + CO * ( *_Y*(*_aX**_aX + *_aZ**_aZ) + *_aY*(-*_aX* *_X - *_aZ* *_Z) )
         + SI * ms * ( *_aZ * *_X - *_aX * *_Z) ) / m;
	*_outZ = (*_aZ* (*_aX * *_X + *_aY * *_Y + *_aZ * *_Z)
         + CO * ( *_Z*(*_aX**_aX + *_aY**_aY) + *_aZ*(-*_aX* *_X - *_aY* *_Y) )
         + SI * ms * (-*_aY * *_X + *_aX * *_Y) ) / m;
			/*
  result.x := (ax.x* (ax.x * vec.x + ax.y * vec.y + ax.z * vec.z)
         + CO * ( vec.x*(ax.y*ax.y + ax.z*ax.z) + ax.x*(-ax.y*vec.y - ax.z*vec.z) )
         + SI * ms * (-ax.z * vec.y + ax.y * vec.z) ) / m;
  result.y := (ax.y* (ax.x * vec.x + ax.y * vec.y + ax.z * vec.z)
         + CO * ( vec.y*(ax.x*ax.x + ax.z*ax.z) + ax.y*(-ax.x*vec.x - ax.z*vec.z) )
         + SI * ms * ( ax.z * vec.x - ax.x * vec.z) ) / m;
  result.z := (ax.z* (ax.x * vec.x + ax.y * vec.y + ax.z * vec.z)
         + CO * ( vec.z*(ax.x*ax.x + ax.y*ax.y) + ax.z*(-ax.x*vec.x - ax.y*vec.y) )
         + SI * ms * (-ax.y * vec.x + ax.x * vec.y) ) / m;
         */
}



















char *CSmodule_View7::docString()
{
	const char *r = "\
this module can be used to reproduce a set projection for multiple pairs of x,y,z values \
and thus makes it easier to control the view parameters of different objects at once.\n\n\
each <rot > input ranges from 0.0 (0 degree) to 1.0 (360 degree).\n\
the input coordinates are rotated around the z, the y and the x axis, then shifted forward on the \
z axis, and again rotated around the x, y and z axis.\
nr of input pairs is set in the properties.\
";
	return const_cast<char*>(r);
}

CSmodule_View7* CSmodule_View7::newOne()
{
	return new CSmodule_View7();
}


void CSmodule_View7::init()
{
	gname = copyString("visual");
	bname = copyString("view 7");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_rotZ = &(createInput("rz","rotate Z")->value);
	_rotY = &(createInput("ry","rotate Y")->value);
	_rotX = &(createInput("rx","rotate X")->value);
	_dist = &(createInput("d","distance Z")->value);
	_rotX1 = &(createInput("rx1","rotate X")->value);
	_rotY1 = &(createInput("ry1","rotate Y")->value);
	_rotZ1 = &(createInput("rz1","rotate Z")->value);

	char n[128],n1[128];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"X%d",i+1);
		__X[i] = createInput(n,n);
		_X[i] = &(__X[i]->value);
		sprintf(n,"Y%d",i+1);
		__Y[i] = createInput(n,n);
		_Y[i] = &(__Y[i]->value);
		sprintf(n,"Z%d",i+1);
		__Z[i] = createInput(n,n);
		_Z[i] = &(__Z[i]->value);

		sprintf(n,"OX%d",i+1);
		sprintf(n1,"X%d",i+1);
		__outX[i] = createOutput(n,n1);
		_outX[i] = &(__outX[i]->value);
		sprintf(n,"OY%d",i+1);
		sprintf(n1,"Y%d",i+1);
		__outY[i] = createOutput(n,n1);
		_outY[i] = &(__outY[i]->value);
		sprintf(n,"OZ%d",i+1);
		sprintf(n1,"Z%d",i+1);
		__outZ[i] = createOutput(n,n1);
		_outZ[i] = &(__outZ[i]->value);
	}

	setNrIn(1);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin","nr of channels",nrIn,1,CSMOD_MAX_CHAN);

}



void CSmodule_View7::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_View7::setNrIn(int newNrIn)
{
	nrIn = newNrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__X[i], (i<nrIn));
		setConnectorActive(__Y[i], (i<nrIn));
		setConnectorActive(__Z[i], (i<nrIn));
		setConnectorActive(__outX[i], (i<nrIn));
		setConnectorActive(__outY[i], (i<nrIn));
		setConnectorActive(__outZ[i], (i<nrIn));
	}

	arrangeConnectors();
}



void CSmodule_View7::step()
{
	csfloat caz = *_rotZ * TWO_PI;
	csfloat saz = sin( caz );
			caz = cos( caz );

	csfloat cay = *_rotY * TWO_PI;
	csfloat say = sin( cay );
			cay = cos( cay );

	csfloat cax = *_rotX * TWO_PI;
	csfloat sax = sin( cax );
			cax = cos( cax );

	csfloat caz1 = *_rotZ1 * TWO_PI;
	csfloat saz1 = sin( caz1 );
			caz1 = cos( caz1 );

	csfloat cay1 = *_rotY1 * TWO_PI;
	csfloat say1 = sin( cay1 );
			cay1 = cos( cay1 );

	csfloat cax1 = *_rotX1 * TWO_PI;
	csfloat sax1 = sin( cax1 );
			cax1 = cos( cax1 );

	csfloat
		x,y,z,
		b;

	for (int i=0;i<nrIn;i++)
	{
		// rotation around Z
		if (*_rotZ!=0.0)
		{
			x = caz * *_X[i] - saz * *_Y[i];
			y = saz * *_X[i] + caz * *_Y[i];
		} else { x = *_X[i] ; y = *_Y[i]; }

		// rotation around Y
		if (*_rotY!=0.0)
		{
			b = x * cay + *_Z[i] * say;
			z = x * say - *_Z[i] * cay;
			x = b;
		} else { z = *_Z[i]; }

		// rotation around X + distance
		if (*_rotX!=0.0)
		{
			b = y * cax - z * sax;
			z = y * sax + z * cax + *_dist;
			y = b;
		} else { z += *_dist; }


		// rotation around X
		if (*_rotX1!=0.0)
		{
			b = y * cax1 - z * sax1;
			z = y * sax1 + z * cax1;
			y = b;
		}

		// rotation around Y
		if (*_rotY1!=0.0)
		{
			b = x * cay1 - z * say1;
			z = x * say1 + z * cay1;
			x = b;
		}

		// rotation around Z
		if (*_rotZ1!=0.0)
		{
			b = caz1 * x - saz1 * y;
			y = saz1 * x + caz1 * y;
			x = b;
		}

		*_outX[i] = x;
		*_outY[i] = y;
		*_outZ[i] = z;

	}

}
