#include <math.h>
#include "csmodule_camera.h"

char *CSmodule_Camera::docString()
{
	const char *r = "\
";
	return const_cast<char*>(r);
}


CSmodule_Camera* CSmodule_Camera::newOne()
{
	return new CSmodule_Camera();
}


void CSmodule_Camera::init()
{
	gname = copyString("visual");
	bname = copyString("camera");
	name = copyString("camera (look at)");
	uname = copyString(bname);

	// --- inputs outputs ----

	_x = &(createInput("x","x")->value);
	_y = &(createInput("y","y")->value);
	_z = &(createInput("z","z")->value);
	_posx = &(createInput("px","pos x")->value);
	_posy = &(createInput("py","pos y")->value);
	_posz = &(createInput("pz","pos z",-1.0)->value);
	_lookx = &(createInput("lx","lookat x")->value);
	_looky = &(createInput("ly","lookat y")->value);
	_lookz = &(createInput("lz","lookat z")->value);
	_upx = &(createInput("ux","up x")->value);
	_upy = &(createInput("uy","up y",1.0)->value);
	_upz = &(createInput("uz","up z")->value);
	_ox = &(createOutput("ox","x")->value);
	_oy = &(createOutput("oy","y")->value);
	_oz = &(createOutput("oz","z")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Camera::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}




void CSmodule_Camera::vnormalize(csfloat *x,csfloat *y,csfloat *z)
{
	csfloat f = *x * *x + *y * *y + *z * *z;
	if (f==0) return;
	f = sqrtf(f);
	*x /= f;
	*y /= f;
	*z /= f;
}


void CSmodule_Camera::step()
{

	// normalize conditions

	vnormalize(_upx, _upy, _upz);

	// transform lookat into look axis
	*_lookx -= *_posx;
	*_looky -= *_posy;
	*_lookz -= *_posz;
	// keep vector campos -> camlookat
	/*
	csfloat ex = *_lookx;
	csfloat ey = *_looky;
	csfloat ez = *_lookz;
	*/
	vnormalize(_lookx, _looky, _lookz);

	// get perpendicular axis to look and up (left)
	csfloat vx = *_upy * *_lookz - *_upz * *_looky;
	csfloat vy = *_upz * *_lookx - *_upx * *_lookz;
	csfloat vz = *_upx * *_looky - *_upy * *_lookx;
	vnormalize(&vx, &vy, &vz);

    // make up perpendicular to look and left
	*_upx = *_looky * vz - *_lookz * vy;
	*_upy = *_lookz * vx - *_lookx * vz;
	*_upz = *_lookx * vy - *_looky * vx;


	csfloat tx = *_posx * vx + *_posy * vy + *_posz * vz;
	csfloat ty = *_posx * *_upx + *_posy * *_upy + *_posz * *_upz;
	csfloat tz = *_posx * *_lookx + *_posy * *_looky + *_posz * *_lookz;

	// matrix transform
	*_ox = *_lookx * *_z + *_upx * *_y + vx * *_x - tx;
	*_oy = *_looky * *_z + *_upy * *_y + vy * *_x - ty;
	*_oz = *_lookz * *_z + *_upz * *_y + vz * *_x - tz;
	/*
	*_ox = *_lookx * *_z + *_upx * *_y + vx * *_x +   vx*ex + *_upx*ey + *_lookx*ez;
	*_oy = *_looky * *_z + *_upy * *_y + vy * *_x +   vy*ex + *_upy*ey + *_looky*ez;
	*_oz = *_lookz * *_z + *_upz * *_y + vz * *_x +   vz*ex + *_upz*ey + *_lookz*ez;
	*/
	/*
	*_ox = (*_lookx * *_z + *_upx * *_y + vx * *_x) * ( vx*ex + *_upx*ey + *_lookx*ez );
	*_oy = (*_looky * *_z + *_upy * *_y + vy * *_x) * ( vy*ex + *_upy*ey + *_looky*ez );
	*_oz = (*_lookz * *_z + *_upz * *_y + vz * *_x) * ( vz*ex + *_upz*ey + *_lookz*ez );
	*/

	/*csfloat d = ex*ex+ey*ey+ez*ez;
	if (d!=0.0)
		*_z += sqrt(d);
*/
	/*
	// normalize conditions

	vnormalize(_upx, _upy, _upz);

	// transform lookat into look axis
	csfloat lookx = *_lookx; *_lookx -= *_posx;
	csfloat looky = *_looky; *_looky -= *_posy;
	csfloat lookz = *_lookz; *_lookz -= *_posz;
	vnormalize(_lookx, _looky, _lookz);

	// get perpendicular axis to look and up (left)
	csfloat vx = *_upy * *_lookz - *_upz * *_looky;
	csfloat vy = *_upz * *_lookx - *_upx * *_lookz;
	csfloat vz = *_upx * *_looky - *_upy * *_lookx;
	vnormalize(&vx, &vy, &vz);

    // make up perpendicular to look and left
	*_upx = *_looky * vz - *_lookz * vy;
	*_upy = *_lookz * vx - *_lookx * vz;
	*_upz = *_lookx * vy - *_looky * vx;


	// translate x,y,z

	*_ox = *_lookx * *_z + *_upx * *_y + vx * *_x;
	*_oy = *_looky * *_z + *_upy * *_y + vy * *_x;
	*_oz = *_lookz * *_z + *_upz * *_y + vz * *_x;

	*_ox -= *_posx + lookx;
	*_oy -= *_posy + looky;
	*_oz -= *_posz + lookz;
	*/
}
























char *CSmodule_Fisheye::docString()
{
	const char *r = "\
the module transforms 3-d coordinates into 2-d coordinates using a 180 degree fisheye projection.\n\n\
<X>, <Y> and <Z> are the vectors of a point in space, assuming that 0,0,0 is the center of the camera, \
and that positive <Z> is in front of camera, positive <Y> is top of camera and positive <X> is right of \
camera center.\n\n\
the output coordinates will always be in the range -1.0 and 1.0.\
";
	return const_cast<char*>(r);
}


CSmodule_Fisheye* CSmodule_Fisheye::newOne()
{
	return new CSmodule_Fisheye();
}


void CSmodule_Fisheye::init()
{
	gname = copyString("visual");
	bname = copyString("fisheye");
	name = copyString("fisheye projection");
	uname = copyString(bname);

	// --- inputs outputs ----

	_x = &(createInput("x","x")->value);
	_y = &(createInput("y","y")->value);
	_z = &(createInput("z","z")->value);
	_ox = &(createOutput("ox","x")->value);
	_oy = &(createOutput("oy","y")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Fisheye::step()
{
	// cart->sphere
	csfloat S = *_x * *_x + *_y * *_y;
	if (S==0)
	{
		*_ox = 0;
		*_oy = 0;
		return;
	}
	csfloat R = sqrtf(S + *_z * *_z);
	if (R==0)
	{
		*_ox = 0;
		*_oy = 0;
		return;
	}
	S = sqrtf(S);
	csfloat phi = sin( acosf(*_z / R) );
	csfloat theta =
		(*_x>=0)? asinf(*_y/S) : (PI - asin(*_y/S));

	// sphere->cart
	*_ox = phi * cos(theta);
	*_oy = phi * sin(theta);

}










char *CSmodule_Hsb2Rgb::docString()
{
	const char *r = "\
the module turns a hue/saturation/brightness description of a color into it's red/green/blue description.\n\
\n\
all values are normalized to 0.0 - 1.0.\n\
<hue> value will be wrapped around.\
";
	return const_cast<char*>(r);
}


CSmodule_Hsb2Rgb* CSmodule_Hsb2Rgb::newOne()
{
	return new CSmodule_Hsb2Rgb();
}


void CSmodule_Hsb2Rgb::init()
{
	gname = copyString("visual");
	bname = copyString("HSB 2 RGB");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_H = &(createInput("h","hue")->value);
	_S = &(createInput("s","sat",1.0)->value);
	_B = &(createInput("b","bright",1.0)->value);
	_r = &(createOutput("r","red")->value);
	_g = &(createOutput("g","green")->value);
	_b = &(createOutput("bl","blue")->value);

	// ---- properties ----

	createNameProperty();
}


void CSmodule_Hsb2Rgb::step()
{
	// hue
	csfloat f = MODULO((*_H<0)?-*_H:*_H,1)*6;
	if (f<1.0)
	{
		*_r = 1.0;
		*_g = f;
		*_b = 0.0;
	} else
	if (f<2.0)
	{
		*_r = 1.0-(f-1.0);
		*_g = 1.0;
		*_b = 0.0;
	} else
	if (f<3.0)
	{
		*_r = 0.0;
		*_g = 1.0;
		*_b = f-2.0;
	} else
	if (f<4.0)
	{
		*_r = 0.0;
		*_g = 1.0-(f-3.0);
		*_b = 1.0;
	} else
	if (f<5.0)
	{
		*_r = (f-4.0);
		*_g = 0.0;
		*_b = 1.0;
	} else
	{
		*_r = 1.0;
		*_g = 0.0;
		*_b = 1.0-(f-5.0);
	}

	// saturation & brightness
	*_r = *_B * ( *_r * *_S + 1.0 - *_S );
	*_g = *_B * ( *_g * *_S + 1.0 - *_S );
	*_b = *_B * ( *_b * *_S + 1.0 - *_S );

	}
