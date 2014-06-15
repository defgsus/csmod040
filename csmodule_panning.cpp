#include <math.h>
#include "csmodule_panning.h"


char *CSmodule_Pos2D::docString()
{
	const char *r = "\
the module simulates a number of directed microphones on a 2d plane (x-z) and a number of positionable sound sources. \
the usage is normally to set the virtual microphones at the same positions as your real-world speaker system \
is positioned.\n\n\
<center X> and <center Z> is the 'listener' position on the plane.\n\
<micro X> and <micro Z> is the position of a microphone relative to the 'listener' position. \
the first 4 microphones by default are aranged in a square laying on the x-z plane around 0,0. \
each microphone has it's output on the module to (normally) feed it to the corresponding speaker.\n\n\
<radius> is the maximum listening radius around the 'listener'. sounds outside this radius are not passed through.\n\
<falloff> is the loss of amplitude for distant sounds. a value of 10.0 or 20.0 is realistic if the position units \
are thought of as meters.\n\
<falloff angle> is the influence of the angle difference (center->sound / micro->sound) to the amplitude. on \
default setting 1.0, the amplitude will be 1.0 for same direction, 0.5 for 90 degree and 0.0 for 180 degree.\n\
<speed of sound> is the speed of sound in air in meters per second (where one meter is a 1.0 in the position inputs).\n\n\
<amp mix> and <delay mix> are in the range of 0.0 and 1.0 and determine the position to use to calculate the \
amplitude and delaytime between 'listener' position and microphone position. \
to calculate the amplitude and delaytime the module calculates the distance between the 'listener' and the \
sound source. with the <amp mix> and <delay mix> values you can shift the position to the position of the \
individual microphone (0.0 = 'listener', 0.5 = half-way between 'listener' and microphone, 1.0 = microphone position). \n\
for example your ears are some centimeters apart from each other, so a sound coming from the left will reach the \
left ear some (very) short time before it reaches the right ear. however, for speakers (microphones) that are \
some meters apart from each other it is not realistic to calculate the delaytime for each of that speakers separately \
since this results in noticeable different doppler effects for each speaker and a single moving sound source might \
sound like many sound sources. but it is o.k. or even preferable to set the <delay mix> value to some small constant \
like 0.01 to 0.07 or so, just to have that little bit of phasing as we are used to by our ears.\n\n\
<input> is the input for an audio signal you want to position in space and <X> and <Z> are the inputs for it's \
position.\n\
\n\
an additional amplitude calculation is done internally: each amplitude is further modulated by the distance of \
the vector between 'listener' and microphone and the vector between 'listener' and sound source. that means if \
a sound comes from the same direction as the microphone points to it is maximally loud. if it comes from the \
opposite position it will be completely off. you can think of it as directional microphones that are arranged \
spherically around the 'listener' each sampling the space away from the 'listener'.\n\
\n\
the number of microphones and sound sources are set in the property view. also the maximum delay time and if the \
delay should be used at all is set there. note that the maximum nescessary delaytime depends on the <speed of sound> \
setting and the possible distance of the sound source to the 'listener'/microphone.\
";
	return const_cast<char*>(r);
}


CSmodule_Pos2D* CSmodule_Pos2D::newOne()
{
	return new CSmodule_Pos2D();
}


void CSmodule_Pos2D::init()
{
	gname = copyString("panning");
	bname = copyString("microphone 2d");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_rad = &(createInput("rad","radius",100.0)->value);
	_falloff = &(createInput("fo","falloff",10.0)->value);
	_ang = &(createInput("foa","falloff angle",1.0)->value);
	_micamp = &(createInput("ma","amp mix",0.5)->value);
	_micdly = &(createInput("md","delay mix",0.01)->value);
	__sos = createInput("sos","speed of sound",330.0);
	_sos = &__sos->value;

	_cx = &(createInput("cx","center X")->value);
	_cz = &(createInput("cz","center Z")->value);

	char n[32],n1[32];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"mx%d",i);
		sprintf(n1,"micro%d X",i+1);
		__mx[i] = createInput(n,n1); _mx[i] = &(__mx[i]->value);
		sprintf(n,"mz%d",i);
		sprintf(n1,"micro%d Z",i+1);
		__mz[i] = createInput(n,n1); _mz[i] = &(__mz[i]->value);
	}
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"in%d",i);
		__in[i] = createInput(n,"input"); _in[i] = &(__in[i]->value);

		sprintf(n,"ax%d",i);
		__ax[i] = createInput(n,"X"); _ax[i] = &(__ax[i]->value);
		sprintf(n,"az%d",i);
		__az[i] = createInput(n,"Z"); _az[i] = &(__az[i]->value);
	}
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"o%d",i);
		sprintf(n1,"micro%d",i+1);
		__out[i] = createOutput(n,n1); _out[i] = &(__out[i]->value);
	}

	// preinit micros
	setConnectorValue(__mx[0], -1.0);
	setConnectorValue(__mz[0], -1.0);
	setConnectorValue(__mx[1],  1.0);
	setConnectorValue(__mz[1], -1.0);
	setConnectorValue(__mx[2],  1.0);
	setConnectorValue(__mz[2],  1.0);
	setConnectorValue(__mx[3], -1.0);
	setConnectorValue(__mz[3],  1.0);

	// init buffer
	bufpos=0;
	buf=buf1=buf2=0;
	usebuf=changebuf=0;
	cursec = 2.0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrs", "nr of microphones", 1, 1,CSMOD_MAX_CHAN);
	createPropertyInt("nrin", "nr of sound sources", 1, 1,CSMOD_MAX_CHAN);
	createPropertyInt("delay", "use delay", 1, 0,1);
	createPropertyFloat("mdelay", "max delay time", cursec, 1.0,60.0);

	setNrIn(1);
	setNrSpeaker(1);
	doDelay = true;

}


void CSmodule_Pos2D::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"nrs")==0)
		setNrSpeaker(prop->ivalue);
	else
	if (strcmp(prop->name,"delay")==0)
	{
		doDelay = (prop->ivalue==1);
		setConnectorActive(__sos, doDelay);
	}
	else
	if (strcmp(prop->name,"mdelay")==0)
		setBuf(prop->fvalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Pos2D::setBuf(float sec)
{
	if (usebuf==2)
	{
		if (buf1) free(buf1);
		newbuflen = max(4, sec*sampleRate);
		buf1 = (csfloat*) calloc(newbuflen*nrIn, sizeof(csfloat));
		bufpos=0;
		changebuf = 1;
	} else
	{
		if (buf2) free(buf2);
		newbuflen = max(4, sec*sampleRate);
		buf2 = (csfloat*) calloc(newbuflen*nrIn, sizeof(csfloat));
		bufpos=0;
		changebuf = 2;
	}
	cursec = sec;
}

void CSmodule_Pos2D::setNrIn(int newNrIn)
{
	nrIn = newNrIn;
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
		setConnectorActive(__ax[i], (i<nrIn));
		setConnectorActive(__az[i], (i<nrIn));
	}
	arrangeConnectors();
	setBuf(cursec);
}

void CSmodule_Pos2D::setNrSpeaker(int newNrSpeaker)
{
	nrSpeaker = newNrSpeaker;
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__mx[i], (i<nrSpeaker));
		setConnectorActive(__mz[i], (i<nrSpeaker));
		setConnectorActive(__out[i], (i<nrSpeaker));
	}
	arrangeConnectors();
	setBuf(cursec);
}



#define dist02(x,z) \
	sqrt( (x)*(x)+(z)*(z) )

#define dist2(x1,z1, x2,z2) \
	sqrt( ((x2)-(x1))*((x2)-(x1)) + \
		  ((z2)-(z1))*((z2)-(z1)) )\

#define distSqr2(x1,z1, x2,z2) \
	((x2)-(x1))*((x2)-(x1)) + \
	((z2)-(z1))*((z2)-(z1)) \

void CSmodule_Pos2D::step()
{
	csfloat
		radSq = *_rad * *_rad,
		micamp = max((csfloat)0,min((csfloat)1, *_micamp )),
		amx,amz,
		micdly = max((csfloat)0,min((csfloat)1, *_micdly )),
		dlx,dlz,

		am,amp,
		dcs,csx,csz,
		dcm,cmx,cmz;


	// sample input into delaybuffer
	if (doDelay)
	{
		// use new allocated buffer if changed
		if (changebuf!=0)
		{
			if (changebuf==1)
				buf = buf1;
			else
				buf = buf2;
			buflen = newbuflen;
			usebuf = changebuf;
			changebuf = 0;
		}

		// for each sound
		for (int i=0;i<nrIn;i++)
		{
			int p=buflen*i;

			// store
			buf[p+bufpos] = *_in[i];

		}
	}


	// go through all microphones/speakers
	for (int i=0;i<nrSpeaker;i++)
	{
		// clear output
		*_out[i] = 0.0;

		// when microphone radius is too small
		if (*_rad<0.000001) continue;

		// used microphone position (amplitude)
		amx = *_cx * (1.0-micamp) + micamp * (*_cx + *_mx[i]);
		amz = *_cz * (1.0-micamp) + micamp * (*_cz + *_mz[i]);

		// distance micro -> center
		dcm = dist2(*_cx,*_cz, *_mx[i],*_mz[i]);
		// and direction normal
		cmx = *_mx[i] - *_cx;
		cmz = *_mz[i] - *_cz;
		// normalize
		csfloat n = dist02(cmx,cmz);
		if (n>0) { cmx /= n; cmz /= n; }

		// go through all sound sources
		for (int j=0;j<nrIn;j++)
		{
			// distance to sound (for amp)
			csfloat d = distSqr2(amx,amz, *_ax[j], *_az[j]);

			// out of audible range
			if (d>radSq) continue;
			d = sqrt(d);

			// amplitude
			amp = 1.0 - d / *_rad;
			if ((*_falloff != 1.0) && (*_falloff>=0.0))
				amp = pow(amp, *_falloff);

			// do the circular amp mod thing

			// cam / sound
			dcs = d;
			csx = *_ax[j] - *_cx;
			csz = *_az[j] - *_cz;
			// normalize
			n = dist02(csx,csz);
			if (n>0) { csx /= n; csz /= n; }

	        // angle amplitude
			am = 1.0 - dist2(csx,csz, cmx,cmz)/2.0;
			if (*_ang!=1.0) am = pow(am, *_ang);

			// blend inside circle
			if ( (dcm>0.0) && (dcs<dcm))
			{
				csfloat f = (dcm-dcs)/dcm;
				am = am*(1.0-f) + f;
			}

			// smooth the spikes
			am = 1.0-(1.0-am)*(1.0-am);
			amp *= am;

			// delay the signal
			if (doDelay)
			{
				// used microphone position (delay)
				dlx = *_cx * (1.0-micdly) + micdly * (*_cx + *_mx[i]);
				dlz = *_cz * (1.0-micdly) + micdly * (*_cz + *_mz[i]);

				d = dist2(dlx,dlz, *_ax[j], *_az[j]);
				d = d * sampleRate / max((csfloat)0.001, *_sos);

				// calc buf position
				d = min((csfloat)buflen-1, max((csfloat)1, d ));
				int dt = (int)d;
				int p = buflen * j;

				// read
				am = buf[ p + (bufpos-dt+buflen) % buflen ];

				// linear interpolate
				d -= dt;
				am = am * d + (1.0-d) *
					buf[ p + (bufpos-dt+1+buflen) % buflen ];

				// write to output
				*_out[i] += amp * am;
			}
			else
			// simpy pass through
			*_out[i] += amp * *_in[j];


		} // sound sources
	} // microphones


	// inc delaybuf write pointer
	if (doDelay)
		bufpos++; if (bufpos>=buflen) bufpos = 0;

}


















































char *CSmodule_Pos3D::docString()
{
	const char *r = "\
the module simulates a number of directed microphones in 3d space and a number of positionable sound sources. \
the usage is normally to set the virtual microphones at the same positions as your real-world speaker system \
is positioned.\n\n\
<center X>, <center Y> and <center Z> is the 'listener' position in space.\n\
<micro X>, <micro Y> and <micro Z> is the position of a microphone relative to the 'listener' position. \
the first 4 microphones by default are aranged as a square laying on the x-z plane around 0,0. \
each microphone has it's output on the module to (normally) feed it to the corresponding speaker.\n\n\
<radius> is the maximum listening radius around the 'listener'. sounds outside this radius are not passed through.\n\
<falloff> is the loss of amplitude for distant sounds. a value of 10.0 or 20.0 is realistic if the position units \
are thought of as meters.\n\
<falloff angle> is the influence of the angle difference (center->sound / micro->sound) to the amplitude. on \
default setting 1.0, the amplitude will be 1.0 for same direction, 0.5 for 90 degree and 0.0 for 180 degree.\n\
<speed of sound> is the speed of sound in air in meters per second (where one meter is a 1.0 in the position inputs).\n\n\
<amp mix> and <delay mix> are in the range of 0.0 and 1.0 and determine the position to use to calculate the \
amplitude and delaytime between 'listener' position and microphone position. \
to calculate the amplitude and delaytime the module calculates the distance between the 'listener' and the \
sound source. with the <amp mix> and <delay mix> values you can shift the position to the position of the \
individual microphone (0.0 = 'listener', 0.5 = half-way between 'listener' and microphone, 1.0 = microphone position). \n\
for example your ears are some centimeters apart from each other, so a sound coming from the left will reach the \
left ear some (very) short time before it reaches the right ear. however, for speakers (microphones) that are \
some meters apart from each other it is not realistic to calculate the delaytime for each of that speakers separately \
since this results in noticeable different doppler effects for each speaker and a single moving sound source might \
sound like many sound sources. but it is o.k. or even preferable to set the <delay mix> value to some small constant \
like 0.01 to 0.07 or so, just to have that little bit of phasing as we are used to by our ears.\n\n\
<input> is the input for an audio signal you want to position in space and <X>, <Y>, <Z> are the inputs for it's \
position.\n\
\n\
an additional amplitude calculation is done internally: each amplitude is further modulated by the distance of \
the vector between 'listener' and microphone and the vector between 'listener' and sound source. that means if \
a sound comes from the same direction as the microphone points to it is maximally loud. if it comes from the \
opposite position it will be completely off. you can think of it as directional microphones that are arranged \
spherically around the 'listener' each sampling the space away from the 'listener'.\n\
\n\
the number of microphones and sound sources are set in the property view. also the maximum delay time and if the \
delay should be used at all is set there. note that the maximum nescessary delaytime depends on the <speed of sound> \
setting and the possible distance of the sound source to the 'listener'/microphone.\
";
	return const_cast<char*>(r);
}


CSmodule_Pos3D* CSmodule_Pos3D::newOne()
{
	return new CSmodule_Pos3D();
}


void CSmodule_Pos3D::init()
{
	gname = copyString("panning");
	bname = copyString("microphone 3d");
	name = copyString(bname);
	uname = copyString(bname);

	minWidth += 30;

	// --- inputs outputs ----

	_rad = &(createInput("rad","radius",100.0)->value);
	_falloff = &(createInput("fo","falloff",10.0)->value);
	_ang = &(createInput("foa","falloff angle",1.0)->value);
	_micamp = &(createInput("ma","amp mix",0.5)->value);
	_micdly = &(createInput("md","delay mix",0.01)->value);
	__sos = createInput("sos","speed of sound",330.0);
	_sos = &__sos->value;

	_cx = &(createInput("cx","center X")->value);
	_cy = &(createInput("cy","center Y")->value);
	_cz = &(createInput("cz","center Z")->value);

	char n[32],n1[32];
	// soundsources
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"in%d",i);
		__in[i] = createInput(n,"input"); _in[i] = &(__in[i]->value);

		sprintf(n,"ax%d",i);
		__ax[i] = createInput(n,"X"); _ax[i] = &(__ax[i]->value);
		sprintf(n,"ay%d",i);
		__ay[i] = createInput(n,"Y"); _ay[i] = &(__ay[i]->value);
		sprintf(n,"az%d",i);
		__az[i] = createInput(n,"Z"); _az[i] = &(__az[i]->value);
	}
	// microphones
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"mx%d",i);
		sprintf(n1,"micro%d X",i+1);
		__mx[i] = createInput(n,n1); _mx[i] = &(__mx[i]->value);
		sprintf(n,"my%d",i);
		sprintf(n1,"micro%d Y",i+1);
		__my[i] = createInput(n,n1); _my[i] = &(__my[i]->value);
		sprintf(n,"mz%d",i);
		sprintf(n1,"micro%d Z",i+1);
		__mz[i] = createInput(n,n1); _mz[i] = &(__mz[i]->value);
	}
	// outputs
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"o%d",i);
		sprintf(n1,"micro%d",i+1);
		__out[i] = createOutput(n,n1); _out[i] = &(__out[i]->value);
	}

	// preinit micros
	setConnectorValue(__mx[0], -1.0);
	setConnectorValue(__mz[0], -1.0);
	setConnectorValue(__mx[1],  1.0);
	setConnectorValue(__mz[1], -1.0);
	setConnectorValue(__mx[2],  1.0);
	setConnectorValue(__mz[2],  1.0);
	setConnectorValue(__mx[3], -1.0);
	setConnectorValue(__mz[3],  1.0);
	setConnectorValue(__my[4],  1.0);
	setConnectorValue(__mz[5], -1.0);

	// init buffer
	bufpos=0;
	buf=buf1=buf2=0;
	usebuf=changebuf=0;
	cursec = 2.0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrs", "nr of microphones", 1, 1,CSMOD_MAX_CHAN);
	createPropertyInt("nrin", "nr of sound sources", 1, 1,CSMOD_MAX_CHAN);
	createPropertyInt("delay", "use delay", 1, 0,1);
	createPropertyFloat("mdelay", "max delay time", cursec, 1.0,60.0);

	setNrIn(1);
	setNrSpeaker(1);
	doDelay = true;

}


void CSmodule_Pos3D::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"nrs")==0)
		setNrSpeaker(prop->ivalue);
	else
	if (strcmp(prop->name,"delay")==0)
	{
		doDelay = (prop->ivalue==1);
		setConnectorActive(__sos, doDelay);
	}
	else
	if (strcmp(prop->name,"mdelay")==0)
		setBuf(prop->fvalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Pos3D::setBuf(float sec)
{
	if (usebuf==2)
	{
		if (buf1) free(buf1);
		newbuflen = max(4, sec*sampleRate);
		buf1 = (csfloat*) calloc(newbuflen*nrIn, sizeof(csfloat));
		bufpos=0;
		changebuf = 1;
	} else
	{
		if (buf2) free(buf2);
		newbuflen = max(4, sec*sampleRate);
		buf2 = (csfloat*) calloc(newbuflen*nrIn, sizeof(csfloat));
		bufpos=0;
		changebuf = 2;
	}
	cursec = sec;
}

void CSmodule_Pos3D::setNrIn(int newNrIn)
{
	nrIn = newNrIn;
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
		setConnectorActive(__ax[i], (i<nrIn));
		setConnectorActive(__ay[i], (i<nrIn));
		setConnectorActive(__az[i], (i<nrIn));
	}
	arrangeConnectors();
	setBuf(cursec);
}

void CSmodule_Pos3D::setNrSpeaker(int newNrSpeaker)
{
	nrSpeaker = newNrSpeaker;
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__mx[i], (i<nrSpeaker));
		setConnectorActive(__my[i], (i<nrSpeaker));
		setConnectorActive(__mz[i], (i<nrSpeaker));
		setConnectorActive(__out[i], (i<nrSpeaker));
	}
	arrangeConnectors();
	setBuf(cursec);
}




void CSmodule_Pos3D::step()
{
	csfloat
		radSq = *_rad * *_rad,
		micamp = max((csfloat)0,min((csfloat)1, *_micamp )),
		amx,amy,amz,
		micdly = max((csfloat)0,min((csfloat)1, *_micdly )),
		dlx,dly,dlz,

		am,amp,
		dcs,csx,csy,csz,
		dcm,cmx,cmy,cmz;


	// sample input into delaybuffer
	if (doDelay)
	{
		// use new allocated buffer if changed
		if (changebuf!=0)
		{
			if (changebuf==1)
				buf = buf1;
			else
				buf = buf2;
			buflen = newbuflen;
			usebuf = changebuf;
			changebuf = 0;
		}

		// for each sound
		for (int i=0;i<nrIn;i++)
		{
			int p=buflen*i;

			// store
			buf[p+bufpos] = *_in[i];

		}
	}


	// go through all microphones/speakers
	for (int i=0;i<nrSpeaker;i++)
	{
		// clear output
		*_out[i] = 0.0;

		// when microphone radius is too small
		if (*_rad<0.000001) continue;

		// used microphone position (amplitude)
		amx = *_cx + micamp * *_mx[i];
		amy = *_cy + micamp * *_my[i];
		amz = *_cz + micamp * *_mz[i];

		// distance micro -> center
		dcm = CSdist(*_cx,*_cy,*_cz, *_mx[i], *_my[i], *_mz[i]);
		// and direction normal
		cmx = *_mx[i];// - *_cx;
		cmy = *_my[i];// - *_cy;
		cmz = *_mz[i];// - *_cz;
		// normalize
		csfloat n = CSdist0(cmx,cmy,cmz);
		if (n>0) { cmx /= n; cmy /= n; cmz /= n; }

		// go through all sound sources
		for (int j=0;j<nrIn;j++)
		{
			// distance to sound (for amp)
			csfloat d = CSdistSqr(amx,amy,amz, *_ax[j], *_ay[j], *_az[j]);

			// out of audible range
			if (d>radSq) continue;
			d = sqrt(d);

			// amplitude from distance
			amp = 1.0 - d / *_rad;
			if (*_falloff != 1.0)
				amp = pow(amp, *_falloff);

			// do the circular amp mod thing

			// cam / sound
			dcs = d;
			csx = *_ax[j] - *_cx;
			csy = *_ay[j] - *_cy;
			csz = *_az[j] - *_cz;
			// normalize
			n = CSdist0(csx,csy,csz);
			if (n>0) { csx /= n; csy /= n; csz /= n; }

	        // angle amplitude
			am = 1.0 - CSdist(csx,csy,csz, cmx,cmy,cmz)/2.0;
			if (*_ang!=1.0) am = pow(am, *_ang);

			// blend inside circle
			if ( (dcm>0.0) && (dcs<dcm))
			{
				csfloat f = (dcm-dcs)/dcm;
				am = am*(1.0-f) + f;
			}

			// smooth the spikes
			am = 1.0-(1.0-am)*(1.0-am);
			amp *= am;

			// delay the signal
			if (doDelay)
			{
				// used microphone position (delay)
				dlx = *_cx + micdly * *_mx[i];
				dly = *_cy + micdly * *_my[i];
				dlz = *_cz + micdly * *_mz[i];

				d = CSdist(dlx,dly,dlz, *_ax[j], *_ay[j], *_az[j]);
				d = d * sampleRate / max((csfloat)0.001, *_sos);

				// calc buf position
				d = min((csfloat)buflen-1, max((csfloat)1, d ));
				int dt = (int)d;
				int p = buflen * j;

				// read
				am = buf[ p + (bufpos-dt+buflen) % buflen ];

				// linear interpolate
				d -= dt;
				am = am * d + (1.0-d) *
					buf[ p + (bufpos-dt-1+(buflen<<1)) % buflen ];

				// write to output
				*_out[i] += amp * am;
			}
			else
			// simpy pass through
			*_out[i] += amp * *_in[j];


		} // sound sources
	} // microphones


	// inc delaybuf write pointer
	if (doDelay)
		bufpos++; if (bufpos>=buflen) bufpos = 0;

}


























