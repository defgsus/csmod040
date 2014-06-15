#include "math.h"
#include "csmodule_filter.h"

char *CSmodule_Filter::docString()
{
	const char *r = "\
this module is a simple first-order filter with resonance for low, band and high pass.\n\n\
the <input> will be clipped to -10,10 to avoid internal overflows.\n\
<amplitude> is the input amplitude of the filter module before clipping.\n\
<freq> determines the filter frequency. currently this is not very accurate \
and only gives a approximation of the used frequency. anyways, the lower, the deeper ;) \
very high frequencies, above the nyquist frequency (or sampling-rate / 2) will result in \
strange but interesting high band distortions.\n\
<resonance> is clipped internally to 0.0 - 0.999999...\n\n\
the <band pass> output will only be calculated if connected.\
";
	return const_cast<char*>(r);
}


CSmodule_Filter* CSmodule_Filter::newOne()
{
	return new CSmodule_Filter();
}


void CSmodule_Filter::init()
{
	gname = copyString("filter");
	bname = copyString("filter");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_amp = &(createInput("amp","amplitude", 1.0)->value);
	_freq = &(createInput("f","frequency", 1000.0)->value);
	_reso = &(createInput("res","resonance")->value);

	_outLP = &(createOutput("outlp","low pass")->value);
	BPcon = createOutput("outbp","band pass");
	_outBP = &(BPcon->value);
	_outHP = &(createOutput("outhp","high pass")->value);

	// ---- properties ----

	createNameProperty();

	// -- intern ---

}

void CSmodule_Filter::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}

void CSmodule_Filter::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);
	sam = 0;
	prevsam0 = 0;
	prevsam1 = 0;
	prevsam0b = 0;
	prevsam1b = 0;
	lfreq = -999;
	lreso = -999.0; // just let it be different to force calc. of coeff2
	coeff1 = 0.0;
	coeff2 = 0.0;
}


void CSmodule_Filter::step()
{
	// calculate filter coefficients

	// cutoff coefficient
	if (lfreq!=*_freq)
	{
		coeff1 =
			max((csfloat)0, min((csfloat)2,
				2.0 * *_freq * isampleRate
			));
	}
	lfreq = *_freq;

	// resonance coefficient
	if (lreso!=*_reso)
	{
		coeff2 =
			max((csfloat)0, min((csfloat)0.999999999, *_reso ));
	}
	lreso = *_reso;


	// keep track of last samples
	prevsam1 = prevsam0;
	prevsam0 = *_outLP;

	// scale and clip input
	csfloat inp = max((csfloat)-10,min((csfloat)10, *_amp * *_in ));

	// get lowpass with reso
	*_outLP =
		max((csfloat)-10,min((csfloat)10,

			*_outLP	+ coeff1 * (inp - *_outLP)
					+ coeff2 * (*_outLP - prevsam1)

		));

	// high pass is simply the input signal minus low pass
	*_outHP = inp - *_outLP;


	// band pass
	if (BPcon->nrCon>0)
	{
		// keep track of last samples
		prevsam1b = prevsam0b;
		prevsam0b = *_outBP;

		// make a lowpass of the highpass output
		*_outBP =
			max((csfloat)-10,min((csfloat)10,
				*_outBP	+ coeff1 * (*_outHP - *_outBP)
				+ coeff2 * (*_outBP - prevsam1b)
			));

	}

}







char *CSmodule_Filter24::docString()
{
	const char *r = "\
this module is a 24db/oct chebychev second-order filter with resonance for low and high pass.\n\n\
the <input> will be clipped to -1,1 to avoid internal overflows.\n\
<amplitude> is the input amplitude of the filter module before clipping.\n\
<freq> determines the filter frequency quite accurately and internally is always >= 2.0. currently the \
exact filter frequency is still a few hertz off, especially for frequencies above 1000 hz.\n\
<band ripple> is the resonance factor and ranges from 0.0 (VERY narrow/resonant) to about 2.0 (very wide)...\n\n\
at some point it might happen that the internal states get too high or infinite. this prob will be solved at a \
later time, right now you have to reload the patch...\
";
	return const_cast<char*>(r);
}


CSmodule_Filter24* CSmodule_Filter24::newOne()
{
	return new CSmodule_Filter24();
}


void CSmodule_Filter24::init()
{
	gname = copyString("filter");
	bname = copyString("filter 24");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_amp = &(createInput("amp","amplitude", 1.0)->value);
	_freq = &(createInput("f","frequency", 1000.0)->value);
	_ripple = &(createInput("rip","band ripple",0.5)->value);

	_outLP = &(createOutput("outlp","low pass")->value);
	_outHP = &(createOutput("outhp","high pass")->value);

	// ---- properties ----

	createNameProperty();

	// -- intern ---

}

void CSmodule_Filter24::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}

void CSmodule_Filter24::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);
	lfreq = lripple = lout0 = lout1 = 0;
	k = sg = cg = 0;
	coeff0 = coeff1 = coeff2 = coeff3 = 0;
	a0 = a1 = a2 = a3 = a4 = a5 = 0;
	b0 = b1 = b2 = b3 = b4 = b5 = 0;
	stage1 = state0 = state1 = state2 = state3 = 0;
}


void CSmodule_Filter24::step()
{
	/** @c from: 24db/Oct LP - http://musicdsp.org/archive.php?classid=3#229 */

	// calculate filter coefficients

	if ((lfreq!=*_freq)||(lripple!=*_ripple))
	{
		k = tan(PI * max((csfloat)2, *_freq) * isampleRate);

		cg = min((csfloat)10.0, max((csfloat)0.0, *_ripple));
		sg = sinh(cg);
		cg = cosh(cg);
		cg *= cg;

		coeff0 = 1.0 / (cg-0.85355339059327376220042218105097);
		coeff1 = k * coeff0*sg*1.847759065022573512256366378792;
		coeff2 = 1.0 / (cg-0.14644660940672623779957781894758);
		coeff3 = k * coeff2*sg*0.76536686473017954345691996806;

		k *= k*1.1185; // emperical value

		/** updated code - http://musicdsp.org/showArchiveComment.php?ArchiveID=229 */

		a0 = 1.0/(coeff1+k+coeff0);
		a1 = 2.0*(coeff0-k)*a0;
		a2 = (coeff1-k-coeff0)*a0;
		b0 = a0*k;
		b1 = 2.0*b0;
		b2 = b0;

		a3 = 1.0/(coeff3+k+coeff2);
		a4 = 2.0*(coeff2-k)*a3;
		a5 = (coeff3-k-coeff2)*a3;
		b3 = a3*k;
		b4 = 2.0*b3;
		b5 = b3;

	}
	lfreq = *_freq;
	lripple = *_ripple;

	// scale and clip input
	csfloat inp = max((csfloat)-1,min((csfloat)1, *_amp * *_in ));

	// filter
	lout1 = lout0;
	lout0 = *_outLP;

	stage1 = max((csfloat)-1,min((csfloat)1, b0*inp + state0 ));
	state0 = max((csfloat)-1,min((csfloat)1, b1*inp + a1*stage1 + state1 ));
	state1 = max((csfloat)-1,min((csfloat)1, b2*inp + a2*stage1 ));
	*_outLP = b3*stage1 + state2;
	state2 = max((csfloat)-1,min((csfloat)1, b4*stage1 + a4* *_outLP + state3 ));
	state3 = max((csfloat)-1,min((csfloat)1, b5*stage1 + a5* *_outLP ));

	// high pass is simply the input signal minus low pass
	*_outHP = inp - *_outLP;

}











char *CSmodule_Follow::docString()
{
	const char *r = "\
the module exponentially follows any input signal, with speed determined by <up> and <down> inputs.\n\n\
<up> and <down> are clipped to the range of 0.0 (no follow) to 1.0 (immidiate follow).\
";
	return const_cast<char*>(r);
}


CSmodule_Follow* CSmodule_Follow::newOne()
{
	return new CSmodule_Follow();
}


void CSmodule_Follow::init()
{
	gname = copyString("signal");
	bname = copyString("follower");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_in = &(createInput("in","input")->value);
	_up = &(createInput("up","follow up", 0.5)->value);
	_down = &(createInput("dn","follow down", 0.5)->value);

	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

}

void CSmodule_Follow::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}



void CSmodule_Follow::step()
{
	if (*_in>*_out)
		*_out += max((csfloat)0,min((csfloat)1,
			*_up * *_up * normSampleRate)) * (*_in - *_out);
	else
		*_out += max((csfloat)0,min((csfloat)1,
			*_down * *_down * normSampleRate)) * (*_in - *_out);
}














char *CSmodule_FollowMulti::docString()
{
	const char *r = "\
the module exponentially follows a number of input signals, with speed determined by <up> \
and <down> inputs.\n\n\
<up> and <down> are clipped to the range of 0.0 (no follow) to 1.0 (immidiate follow).\
";
	return const_cast<char*>(r);
}

CSmodule_FollowMulti* CSmodule_FollowMulti::newOne()
{
	return new CSmodule_FollowMulti();
}


void CSmodule_FollowMulti::init()
{
	gname = copyString("signal");
	bname = copyString("follower (multi)");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_up = &(createInput("up","up",0.05)->value);
	_down = &(createInput("dn","down",0.05)->value);

	outputStart = inputStart+2*inputHeight;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("in",i);
		__in[i] = createInput(n,"in");
		_in[i] = &(__in[i]->value);
		free(n);
		n = int2char("out",i);
		__out[i] = createOutput(n,"out");
		_out[i] = &(__out[i]->value);
		free(n);
	}

	oldNrIn = 0;
	setNrIn(2);

	mop = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of inputs", nrIn, 2,CSMOD_MAX_CHAN);

}

void CSmodule_FollowMulti::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_FollowMulti::setNrIn(int newNrIn)
{
	if (newNrIn==oldNrIn) return;
	nrIn = newNrIn;
	oldNrIn = nrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__in[i], (i<nrIn));
		setConnectorActive(__out[i], (i<nrIn));
	}

	arrangeConnectors();
}


void CSmodule_FollowMulti::step()
{
	for (int i=0;i<nrIn;i++)
	{
		if (*_in[i]>*_out[i])
			*_out[i] += max((csfloat)0,min((csfloat)1,
				*_up * *_up * *_up * normSampleRate)) * (*_in[i] - *_out[i]);
		else
			*_out[i] += max((csfloat)0,min((csfloat)1,
				*_down * *_down * *_down * normSampleRate)) * (*_in[i] - *_out[i]);
	}
}











char *CSmodule_Eq::docString()
{
	const char *r = "\
simple equalizer with variable number of bands.\n\n\
nr of bands is set in properties.\n\
the filters work exactly as in the normal filter module.\n\
a bandpassed signal is calculated for each band and added to the input by the amount of <amp x>.\
";
	return const_cast<char*>(r);
}

CSmodule_Eq* CSmodule_Eq::newOne()
{
	return new CSmodule_Eq();
}


void CSmodule_Eq::init()
{
	gname = copyString("filter");
	bname = copyString("eq");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_input = &(createInput("in","input")->value);
	_out = &(createOutput("out","output")->value);

	char n[128];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"amp %d",i+1);
		__amp[i] = createInput(n,n,0.5);
		_amp[i] = &(__amp[i]->value);

		sprintf(n,"freq %d",i+1);
		__freq[i] = createInput(n,n,1000.0);
		_freq[i] = &(__freq[i]->value);

		sprintf(n,"reso %d",i+1);
		__reso[i] = createInput(n,n);
		_reso[i] = &(__reso[i]->value);

	}

	setNrIn(1);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of inputs", nrIn, 1,CSMOD_MAX_CHAN);

}

void CSmodule_Eq::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_Eq::setNrIn(int newNrIn)
{
	nrIn = newNrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__amp[i], (i<nrIn));
		setConnectorActive(__freq[i], (i<nrIn));
		setConnectorActive(__reso[i], (i<nrIn));
	}

	arrangeConnectors();
}

void CSmodule_Eq::setSampleRate(int sr)
{
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		saml[i] = samb[i] =
		prevsam0[i] = prevsam1[i] =
		prevsam0b[i] = prevsam1b[i] =
		lfreq[i] = lreso[i] = -1.0;
		coeff1[i] = coeff2[i] = 0.0;
	}
}

void CSmodule_Eq::step()
{

	// scale and clip input
	csfloat
		inp = max((csfloat)-10,min((csfloat)10, *_input )),
		samh;

	for (int i=0;i<nrIn;i++)
	{
		// calculate filter coefficients

		// cutoff coefficient
		if (lfreq[i]!=*_freq[i])
		{
			coeff1[i] =
				max((csfloat)0, min((csfloat)2,
					2.0 * *_freq[i] * isampleRate
				))	;
		}
		lfreq[i] = *_freq[i];

		// resonance coefficient
		if (lreso[i]!=*_reso[i])
		{
			coeff2[i] =
				max((csfloat)0, min((csfloat)0.999999999, *_reso[i] ));
		}
		lreso[i] = *_reso[i];


		// keep track of last samples
		prevsam1[i] = prevsam0[i];
		prevsam0[i] = saml[i];

		// get lowpass with reso
		saml[i] =
			max((csfloat)-10,min((csfloat)10,

			saml[i]	+ coeff1[i] * (inp - saml[i])
					+ coeff2[i] * (saml[i] - prevsam1[i])

		));

		// highpass
		samh = inp - saml[i];

		// band pass

		// keep track of last samples
		prevsam1b[i] = prevsam0b[i];
		prevsam0b[i] = samb[i];

		// make a lowpass of the highpass output
		samb[i] =
			max((csfloat)-10,min((csfloat)10,
				samb[i] + coeff1[i] * (samh - samb[i])
						+ coeff2[i] * (samb[i] - prevsam1b[i])
			));


		inp = inp + *_amp[i] * samb[i];
	}

	*_out = inp;
}
