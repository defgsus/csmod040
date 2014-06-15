#include "math.h"
#include "csmodule_env.h"

char *CSmodule_EnvD::docString()
{
	const char *r = "\
this is a exponential decay envelope generator.\n\n\
a positive edge in <gate> immidiately restarts the envelope with the value of <velocity>.\n\
the resulting envelope is mulitplied by the value in <amplitude>.\n\
the envelope exponentially fades towards zero in <decay> seconds.\
";
	return const_cast<char*>(r);
}


CSmodule_EnvD* CSmodule_EnvD::newOne()
{
	return new CSmodule_EnvD();
}


void CSmodule_EnvD::init()
{
	gname = copyString("envelope");
	bname = copyString("envelope D");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_rst = &(createInput("rst","gate")->value);
	_velo = &(createInput("vel","velocity",1.0)->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);
	_decay = &(createInput("d","decay",1.0)->value);

	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();

	lrst = 0.0;
	ldecay = 0.0;
	level = 0.0;

}

void CSmodule_EnvD::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}

void CSmodule_EnvD::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);
	// force recalculation of coefficients
	ldecay = -999;
}


void CSmodule_EnvD::step()
{
	// reset
	if ((lrst<=0.0)&&(*_rst>0.0)) level = *_velo;
	lrst = *_rst;

	// calculate coefficient
	if (ldecay != *_decay)
	{
		// org: (log(endlevel) - log(startlevel)) / (decay * samplerate)
		coeff = -8 / ( max((csfloat)0.0001, *_decay) * sampleRate );
	}
	ldecay = *_decay;

	// calc env
	level += coeff * level;

	// set output
	*_out = *_amp * level;
}











char *CSmodule_EnvAD::docString()
{
	const char *r = "\
this is a log-exp attack-decay envelope generator.\n\n\
a positive edge in <gate> restarts the envelope sequence. \n\
it will then raise logarithmically towards the value of <velocity> in <attack> seconds.\n\
afterwards it will exponentially fade back towards zero in <decay> seconds.\n\
the resulting envelope is mulitplied by the value in <amplitude>.\
";
	return const_cast<char*>(r);
}


CSmodule_EnvAD* CSmodule_EnvAD::newOne()
{
	return new CSmodule_EnvAD();
}


void CSmodule_EnvAD::init()
{
	gname = copyString("envelope");
	bname = copyString("envelope AD");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_rst = &(createInput("rst","gate")->value);
	_velo = &(createInput("vel","velocity",1.0)->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);
	_attack = &(createInput("a","attack",0.03)->value);
	_decay = &(createInput("d","decay",1.0)->value);

	_out = &(createOutput("out","output")->value);

	// ---- properties ----

	createNameProperty();


	lrst = 0.0;
	lattack = 0.0;
	ldecay = 0.0;
	level = 0.0;
	velo = 0.0;

	mode = 1;

}

void CSmodule_EnvAD::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}


void CSmodule_EnvAD::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);
	// force recalculation of coefficients
	ldecay = -999;
	lattack = -999;
}


void CSmodule_EnvAD::step()
{
	// reset
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		mode = 0;
		velo = *_velo;
		//level = 0.0;
	}
	lrst = *_rst;

	// check envelope stage
	switch(mode)
	{
		case 0: // attack

			// calculate coefficient
			if (lattack != *_attack)
				coeff1 = 8.0 / max((csfloat)8, *_attack * sampleRate );
			lattack = *_attack;

			// env
			level += coeff1 * (velo-level);

			// mode change
			if (level>=0.999*velo-0.0018* *_attack) mode = 1;
			break;

		case 1: // decay

			// calculate coefficient
			if (ldecay != *_decay)
				coeff2 = -8.0 / max((csfloat)8, *_decay * sampleRate );

			ldecay = *_decay;

			// env
			level += coeff2 * level;
			break;
	}

	// set output
	*_out = *_amp * level;
}













char *CSmodule_EnvADMulti::docString()
{
	const char *r = "\
this is a log-exp attack-decay multi-channel envelope generator.\n\n\
a positive edge in <gate> restarts the envelope sequence of each channel. \n\
it will then raise logarithmically towards the value of <velocity> in <attack> seconds.\n\
afterwards it will exponentially fade back towards zero in <decay> seconds.\n\
the resulting envelope is mulitplied by the value in <amplitude>.\
";
	return const_cast<char*>(r);
}


CSmodule_EnvADMulti* CSmodule_EnvADMulti::newOne()
{
	return new CSmodule_EnvADMulti();
}


void CSmodule_EnvADMulti::init()
{
	gname = copyString("envelope");
	bname = copyString("envelope AD (multi)");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_amp = &(createInput("amp","amplitude",1.0)->value);
	_attack = &(createInput("a","attack",0.03)->value);
	_decay = &(createInput("d","decay",1.0)->value);

	char n[128];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"rst%d",i);
		__rst[i] = createInput(n,"gate");
		_rst[i] = &__rst[i]->value;

		sprintf(n,"vel%d",i);
		__velo[i] = createInput(n,"velocity",1.0);
		_velo[i] = &__velo[i]->value;

		sprintf(n,"out%d",i);
		__out[i] = createOutput(n,"output");
		_out[i] = &__out[i]->value;
	}

	setNrOut(2);

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrout","nr of channels", nrOut, 1, CSMOD_MAX_CHAN);

	// --- internals ---

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		lrst[i] = 0.0;
		level[i] = 0.0;
		velo[i] = 0.0;
		mode[i] = 1;
	}
	lattack = 0.0;
	ldecay = 0.0;

}

void CSmodule_EnvADMulti::propertyCallback(CSmodProperty *prop)
{
	if (!strcmp(prop->name, "nrout"))
		setNrOut(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_EnvADMulti::setNrOut(int newNrOut)
{
	nrOut = newNrOut;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__rst[i], (i<nrOut));
		setConnectorActive(__velo[i], (i<nrOut));
		setConnectorActive(__out[i], (i<nrOut));
	}

	arrangeConnectors();
}

void CSmodule_EnvADMulti::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);
	// force recalculation of coefficients
	ldecay = -999;
	lattack = -999;
}


void CSmodule_EnvADMulti::step()
{
	// calculate coefficient
	if (lattack != *_attack)
		coeff1 = 8.0 / max((csfloat)8, *_attack * sampleRate );
	lattack = *_attack;

	// calculate coefficient
	if (ldecay != *_decay)
		coeff2 = -8.0 / max((csfloat)8, *_decay * sampleRate );
	ldecay = *_decay;

	for (int i=0;i<nrOut;i++)
	{
		// reset
		if ((lrst[i]<=0.0)&&(*_rst[i]>0.0))
		{
			mode[i] = 0;
			velo[i] = *_velo[i];
			//level = 0.0;
		}
		lrst[i] = *_rst[i];

		// check envelope stage
		switch(mode[i])
		{
			case 0: // attack

				// env
				level[i] += coeff1 * (velo[i]-level[i]);

				// mode change
				if (level[i]>=0.999*velo[i]-0.0018* *_attack) mode[i] = 1;
			break;

			case 1: // decay

				// env
				level[i] += coeff2 * level[i];
			break;
		}

		// set output
		*_out[i] = *_amp * level[i];
		CSclipFloat(*_out[i]);
	}
}
















char *CSmodule_Fade::docString()
{
	const char *r = "\
this module fades from one value to the next in a fixed amount of seconds.\n\
\n\
<value 0> is the start value, <value 1> is the final value.\n\
\n\
whenever something above zero goes into <gate> the module starts to fade from <value 0> \
to <value 1>.\n\
the time for the transition is given in <seconds>.\n\
\n\
<shape> sets the quadratic shape function exponent for the <smooth> output.\n\
\n\
a gate in <reset> resets the whole fade sequence.\
";
	return const_cast<char*>(r);
}


CSmodule_Fade* CSmodule_Fade::newOne()
{
	return new CSmodule_Fade();
}


void CSmodule_Fade::init()
{
	gname = copyString("envelope");
	bname = copyString("fade");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_v0 = &(createInput("v0","value 0",0.0)->value);
	_v1 = &(createInput("v1","value 1",1.0)->value);
	_rst = &(createInput("rst","gate")->value);
	_sec = &(createInput("t","seconds",1.0)->value);
	_shape = &(createInput("sh","shape",3.0)->value);
	_reset = &(createInput("rstt","reset")->value);

	_out = &(createOutput("out","output")->value);
	_outS = &(createOutput("outs","smooth")->value);

	// ---- properties ----

	createNameProperty();

	lrst = 0.0;
	samplesPassed = 0;
	samples2Pass = 0;

}

void CSmodule_Fade::propertyCallback(CSmodProperty *prop)
{
	CSmodule::propertyCallback(prop);
}


/*
void CSmodule_Fade::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);

	samples2Pass = *_sec * sampleRate;
	 ^ this would restart the FadeFF module

	!! TODO !!
	cannot use *_inputs in setSampleRate because container calls to early
}
*/

void CSmodule_Fade::step()
{
	// retrigger
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		samplesPassed = 0;
		samples2Pass = max(0, *_sec * sampleRate);
	}
	lrst = *_rst;

	// fade
	if ((samples2Pass!=0)&&(samplesPassed<=samples2Pass))
	{
		// normalize
		csfloat f = (csfloat)samplesPassed / samples2Pass;

		// linear out
		*_out = *_v0 * (1.0-f) + f * *_v1;

		// shape
		f = *_shape * f * f * (1.0-f) + pow(f, *_shape);

		// smooth out
		*_outS = *_v0 * (1.0-f) + f * *_v1;

		samplesPassed++;
	}

	// reset
	if ((lreset<=0.0)&&(*_reset>0.0))
	{
		samplesPassed = 0;
		samples2Pass = 0;
		*_out = *_v0;
		*_outS = *_v0;
	}
	lreset = *_reset;

}





char *CSmodule_FadeFF::docString()
{
	const char *r = "\
this module fades back and forth between to values in a fixed amount of seconds.\n\
\n\
<value 0> and <value 1> are the two values to blend between.\n\
\n\
whenever something above zero goes into <gate> the module starts to fade from <value 0> \
to <value 1>, or from <value 1> to <value 0> any other time.\n\
the time for the transition is given in <seconds>.\n\
\n\
<shape> sets the quadratic shape function exponent for the <smooth> output.\
\n\
a gate in <reset> resets the whole fade sequence.\n\
a gate in <phase reset> only resets the fade direction to UP (0 -> 1).\
";
	return const_cast<char*>(r);
}


CSmodule_FadeFF* CSmodule_FadeFF::newOne()
{
	return new CSmodule_FadeFF();
}


void CSmodule_FadeFF::init()
{
	CSmodule_Fade::init();

	free(gname); gname = copyString("envelope");
	free(bname); bname = copyString("fadeff");
	free(name); name = copyString("fade up/down");
	rename(name);

	if (!parent) return;

	// --- inputs outputs ----

	_preset = &(insertInput(3,"rst1","phase reset")->value);

	// ---- properties ----

	lreset = 0.0;
	dir = 1;
}

void CSmodule_FadeFF::step()
{
	// phase reset
	if ((lpreset<=0.0)&&(*_preset>0.0))
	{
		dir = 1;
	}
	lpreset = *_preset;

	// gate reset
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		samples2Pass = max(0, *_sec * sampleRate);

		dir = 1-dir;
		if (dir==0)
			samplesPassed = 0;
		else
			samplesPassed = samples2Pass;
	}
	lrst = *_rst;

	// fade
	if ((samples2Pass!=0)&&(samplesPassed<=samples2Pass))
	{
		// normalize
		csfloat f = (csfloat)samplesPassed / samples2Pass;

		// linear out
		*_out = *_v0 * (1.0-f) + f * *_v1;

		// shape
		f = *_shape * f * f * (1.0-f) + pow(f, *_shape);

		// smooth out
		*_outS = *_v0 * (1.0-f) + f * *_v1;

		if (dir==0)
			samplesPassed++;
		else
			samplesPassed--;
	}

	// reset
	if ((lreset<=0.0)&&(*_reset>0.0))
	{
		samplesPassed = 0;
		samples2Pass = 0;
		*_out = *_v0;
		*_outS = *_v0;
	}
	lreset = *_reset;

}














char *CSmodule_FadeMulti::docString()
{
	const char *r = "\
this module fades from one value to the next in a fixed amount of seconds.\n\
\n\
<value 0> is the start value, <value 1> is the final value.\n\
\n\
whenever something above zero goes into <gate> the module starts to fade from <value 0> \
to <value 1>.\n\
the time for the transition is given in <seconds>.\n\
\n\
<shape> sets the quadratic shape function exponent for the <smooth> output.\
";
	return const_cast<char*>(r);
}


CSmodule_FadeMulti* CSmodule_FadeMulti::newOne()
{
	return new CSmodule_FadeMulti();
}


void CSmodule_FadeMulti::init()
{
	gname = copyString("envelope");
	bname = copyString("fade multi");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_rst = &(createInput("rst","gate")->value);
	_sec = &(createInput("t","seconds",1.0)->value);
	_shape = &(createInput("sh","shape",3.0)->value);

	char n[128],n1[128];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"v0%d",i);
		sprintf(n1,"A %d",i+1);
		__v0[i] = createInput(n,n1,0.0);
		_v0[i] = &__v0[i]->value;

		sprintf(n,"v1%d",i);
		sprintf(n1,"B %d",i+1);
		__v1[i] = createInput(n,n1,0.0);
		_v1[i] = &__v1[i]->value;

		sprintf(n,"out%d",i);
		sprintf(n1,"out %d",i+1);
		__out[i] = createOutput(n,n1);
		_out[i] = &__out[i]->value;

		sprintf(n,"out%d",i);
		sprintf(n1,"smth %d",i+1);
		__outS[i] = createOutput(n,n1);
		_outS[i] = &__outS[i]->value;

	}


	// ---- properties ----

	setNrIn(2);

	createNameProperty();
	createPropertyInt("nrin","nr of inputs",nrIn,1,CSMOD_MAX_CHAN);

	lrst = 0.0;
	samplesPassed = 0;
	samples2Pass = 0;

}

void CSmodule_FadeMulti::propertyCallback(CSmodProperty *prop)
{
	if (!strcmp(prop->name,"nrin"))
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_FadeMulti::setNrIn(int newNrIn)
{
	nrIn = newNrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__v0[i], (i<nrIn));
		setConnectorActive(__v1[i], (i<nrIn));
		setConnectorActive(__out[i], (i<nrIn));
		setConnectorActive(__outS[i], (i<nrIn));
	}

	arrangeConnectors();
}


/*
void CSmodule_FadeMulti::setSampleRate(int sr)
{
	CSmodule::setSampleRate(sr);

	samples2Pass = *_sec * sampleRate;
	 ^ this would restart the FadeMultiFF module

	!! TODO !!
	cannot use *_inputs in setSampleRate because container calls to early
}
*/

void CSmodule_FadeMulti::step()
{
	// reset
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		samplesPassed = 0;
		samples2Pass = max(0, *_sec * sampleRate);
	}
	lrst = *_rst;

	// fade
	if ((samples2Pass!=0)&&(samplesPassed<=samples2Pass))
	{
		// normalize
		csfloat f = (csfloat)samplesPassed / samples2Pass;

		// linear out
		for (int i=0;i<nrIn;i++)
			*_out[i] = *_v0[i] * (1.0-f) + f * *_v1[i];

		// shape
		f = *_shape * f * f * (1.0-f) + pow(f, *_shape);

		// smooth out
		for (int i=0;i<nrIn;i++)
			*_outS[i] = *_v0[i] * (1.0-f) + f * *_v1[i];

		samplesPassed++;
	}
}












char *CSmodule_FadeMultiFF::docString()
{
	const char *r = "\
this module fades back and forth between to values in a fixed amount of seconds.\n\
\n\
<value 0> and <value 1> are the two values to blend between.\n\
\n\
whenever something above zero goes into <gate> the module starts to fade from <value 0> \
to <value 1>, or from <value 1> to <value 0> any other time.\n\
the time for the transition is given in <seconds>.\n\
\n\
<shape> sets the quadratic shape function exponent for the <smooth> output.\
";
	return const_cast<char*>(r);
}


CSmodule_FadeMultiFF* CSmodule_FadeMultiFF::newOne()
{
	return new CSmodule_FadeMultiFF();
}


void CSmodule_FadeMultiFF::init()
{
	CSmodule_FadeMulti::init();

	free(gname); gname = copyString("envelope");
	free(bname); bname = copyString("fademultiff");
	free(name); name = copyString("fade multi up/down");
	rename(name);

	if (!parent) return;

	// --- inputs outputs ----

	_reset = &(insertInput(3,"rst1","reset")->value);

	// ---- properties ----

	lreset = 0.0;
	dir = 1;
}

void CSmodule_FadeMultiFF::step()
{
	// phase reset
	if ((lreset<=0.0)&&(*_reset>0.0))
	{
		dir = 1;
	}
	lreset = *_reset;

	// gate reset
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		samples2Pass = max(0, *_sec * sampleRate);

		dir = 1-dir;
		if (dir==0)
			samplesPassed = 0;
		else
			samplesPassed = samples2Pass;
	}
	lrst = *_rst;

	// fade
	if ((samples2Pass!=0)&&(samplesPassed<=samples2Pass))
	{
		// normalize
		csfloat f = (csfloat)samplesPassed / samples2Pass;

		// linear out
		for (int i=0;i<nrIn;i++)
			*_out[i] = *_v0[i] * (1.0-f) + f * *_v1[i];

		// shape
		f = *_shape * f * f * (1.0-f) + pow(f, *_shape);

		// smooth out
		for (int i=0;i<nrIn;i++)
			*_outS[i] = *_v0[i] * (1.0-f) + f * *_v1[i];

		if (dir==0)
			samplesPassed++;
		else
			samplesPassed--;
	}
}
