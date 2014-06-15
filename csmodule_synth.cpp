#include <algorithm>
#include <math.h>
#include "csmodule_synth.h"



char *CSmodule_Synth::docString()
{
	const char *r = "\
simple polyphonic sinusoid AD enveloped synthesizer.\n\n\
the number of voices is maximally limited to 128. the nr of voices that are output separately \
is set in properties. the number of actual voices to use is set in the <nr voices> input of the module.\n\n\
a gate in <panic!> will ultimately reset all synth parameters.\n\
<amplitude> will scale all output channels.\n\n\
everything below and including <gate> works polyphonic, that means that separate values of the inputs \
are sampled and hold for each voic whenever a <gate> occures.\n\n\
<freq (hz)> simply sets the frequency in hertz of the synthesizer voice.\n\
<velocity> set the amplitude of the voice.\n\
<attack> and <decay> set the attack and decay phase of the envelope of each synthesizer voice in \
(aproximately) seconds. NOTE: in properties the envelope inputs can be set to immidiate mode which \
makes them act on on all voices all the time.\
";
	return const_cast<char*>(r);
}


CSmodule_Synth* CSmodule_Synth::newOne()
{
	return new CSmodule_Synth();
}


void CSmodule_Synth::init()
{
	gname = copyString("synth");
	bname = copyString("synth");
	name = copyString("synth (sin)");
	uname = copyString(name);

	if (!parent) return;

	// --- inputs outputs ----

	_nrVoices = &(createInput("nv","nr voices",4.0)->value);
	_panic = &(createInput("p","panic!",0.0)->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);

	_gate = &(createInput("g","gate")->value);
	_vel = &(createInput("v","velocity",1.0)->value);
	_freq = &(createInput("f","freq (hz)",0.0)->value);
	_attack = &(createInput("at","attack",0.03)->value);
	_decay = &(createInput("de","decay",1.0)->value);

	//_activeVoices = &(createOutput("av","nr voices")->value);
	_outm = &(createOutput("om","out mix")->value);

	char n[128];
	for (int i=0;i<CSMOD_MIDI_MAX_POLY;i++)
	{
		sprintf(n,"out%d",i+1);
		__out[i] = createOutput(n,n);
		_out[i] = &(__out[i]->value);
	}

	setNrOut(4);
	panic();
	lpanic = lgate = lattack = ldecay = 0.0;
	imidiateEnvelope = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrout", "nr of outputs", nrOut, 1,CSMOD_MAX_CHAN);
	createPropertyInt("imienv", "imidiate envelope? (0/1)", imidiateEnvelope, 0,1);
}


void CSmodule_Synth::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	if (strcmp(prop->name,"imienv")==0)
		imidiateEnvelope = prop->ivalue;
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_Synth::setNrOut(int newNrOut)
{
	nrOut = newNrOut;

	for (int i=0;i<CSMOD_MIDI_MAX_POLY;i++)
	{
		setConnectorActive(__out[i], (i<nrOut));
	}

	arrangeConnectors();
}


void CSmodule_Synth::panic()
{
	for (int i=0;i<CSMOD_MIDI_MAX_POLY;i++)
	{
		active[i] = 0;
		freq[i] = 0.0;
		velo[i] = 0.0;
		env[i] = 0.0;
		coeff1[i] = 0.0;
		coeff2[i] = 0.0;
		*_out[i] = 0.0;
	}
	*_outm = 0.0;
	voiceNr = 0;
	nrActive = 0;
}

void CSmodule_Synth::step()
{

	// start a voice
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		// check voice index
		if ((voiceNr>=CSMOD_MIDI_MAX_POLY) || (voiceNr>=*_nrVoices))
			voiceNr = 0.0;

		// assign initial parameters

		active[voiceNr] = 1.0;
		env[voiceNr] = 0.0;
		freq[voiceNr] = *_freq * TWO_PI * isampleRate;
		velo[voiceNr] = *_vel;
		phase[voiceNr] = 0.0;

		// calculate envelope coefficients

		coeff1[voiceNr] = 8.0 / max((csfloat)8, *_attack * sampleRate );
		coeff2[voiceNr] = -8.0 / max((csfloat)8, *_decay * sampleRate );

		// increment voice index
		voiceNr++;
		nrActive = max(voiceNr, nrActive);
	}
	lgate = *_gate;

	// calculate override envelope coeff
	if (imidiateEnvelope)
	{
			if (lattack != *_attack)
				coeffi1 = 8.0 / max((csfloat)8, *_attack * sampleRate );
			lattack = *_attack;

			if (ldecay != *_decay)
				coeffi2 = -8.0 / max((csfloat)8, *_decay * sampleRate );
			ldecay = *_decay;
	}


	// -.-.- calculate voices -.-.-

	int highestActive = 0;
	csfloat o = 0.0;
	*_outm = 0.0;
	for (int i=0;i<nrActive;i++)
	{
		// check envelope stage
		switch(active[i])
		{
			// ignore this voice
			case 0: continue; break;

			// attack
			case 1:
				if (!imidiateEnvelope) coeffi1 = coeff1[i];
				env[i] += coeffi1 * (velo[i]-env[i]);

				// mode change
				if (env[i]>=0.999*velo[i]-0.0018* *_attack) active[i] = 2;

				// remember highest active voice
				highestActive = i;
			break;

			// decay
			case 2:
				if (!imidiateEnvelope) coeffi2 = coeff2[i];
				env[i] += coeffi2 * env[i];

				// turn off
				if (env[i]<=0.0001*velo[i])
				{
					active[i] = 0.0;
				} else
				// remember highest active voice
				highestActive = i;
			break;

		} // end envelope

		// generate voice
		o = *_amp * env[i] * sinf(phase[i]);
		*_outm += o;
		*_out[i] = o;

		// increment phase
		phase[i] += freq[i];
		// wrap phase around to avoid overflow
		if (phase[i]>TWO_PI) phase[i]-=TWO_PI;
		else
		if (phase[i]<-TWO_PI) phase[i]+=TWO_PI;

	} // end voices

	// limit index cycles
	nrActive = min(nrActive, highestActive+1);


	// picnic!
	if ((lpanic<=0.0)&&(*_panic>0.0)) panic();
	lpanic = *_panic;

}













char *CSmodule_SynthMulti::docString()
{
	const char *r = "\
simple polyphonic multi waveform, AD enveloped synthesizer.\n\n\
the number of voices is maximally limited to 128. the nr of voices that are output separately \
is set in properties. the number of actual voices to use is set in the <nr voices> input of the module.\n\n\
a gate in <panic!> will ultimately reset all synth parameters.\n\
<amplitude> will scale all output channels.\n\n\
everything below and including <gate> works polyphonic, that means that separate values of the inputs \
are sampled and hold for each voic whenever a <gate> occures.\n\n\
<freq (hz)> simply sets the frequency in hertz of the synthesizer voice.\n\
<velocity> set the amplitude of the voice.\n\
<attack> and <decay> set the attack and decay phase of the envelope of each synthesizer voice in \
(aproximately) seconds. NOTE: in properties the envelope inputs can be set to immidiate mode which \
makes them act on on all voices all the time.\n\n\
<wave form> selects the waveform to be used by each voice. the values are 0 sin, 1 cosine, 2 square, \
3 saw, 4 triangle.\n\
the <pulse width> input works the same way as in the normal oscillator module on the square and triangle \
waveforms.\n\
<phase mod> modulates the phase of all oscillator voices.\
";
	return const_cast<char*>(r);
}


CSmodule_SynthMulti* CSmodule_SynthMulti::newOne()
{
	return new CSmodule_SynthMulti();
}


void CSmodule_SynthMulti::init()
{
	CSmodule_Synth::init();
	free(bname); bname = copyString("synth (multi)");
	free(name); name = copyString("synth (multiwave)");
	rename(name);

	// --- inputs outputs ----

	_phm = &(createInput("phm","phase mod",0.0)->value);
	_wave = &(createInput("wf","wave form",0.0)->value);
	_pw = &(createInput("pw","pulse width",0.5)->value);


	for (int i=0;i<CSMOD_MIDI_MAX_POLY;i++)
	{
		waveForm[i] = 0;
		pw[i] = 0.5;
	}
}


void CSmodule_SynthMulti::step()
{
	// start a voice
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		// check voice index
		if ((voiceNr>=CSMOD_MIDI_MAX_POLY) || (voiceNr>=*_nrVoices))
			voiceNr = 0.0;

		// assign initial parameters

		active[voiceNr] = 1.0;
		env[voiceNr] = 0.0;
		freq[voiceNr] = *_freq * TWO_PI * isampleRate;
		velo[voiceNr] = *_vel;
		phase[voiceNr] = 0.0;
		waveForm[voiceNr] = abs((int)*_wave) % 5;
		pw[voiceNr] = max((csfloat)0.00001,min((csfloat)0.99999, *_pw ));;

		// calculate envelope coefficients

		coeff1[voiceNr] = 8.0 / max((csfloat)8, *_attack * sampleRate );
		coeff2[voiceNr] = -8.0 / max((csfloat)8, *_decay * sampleRate );

		// increment voice index
		voiceNr++;
		nrActive = max(voiceNr, nrActive);
	}
	lgate = *_gate;

	// calculate override envelope coeff
	if (imidiateEnvelope)
	{
			if (lattack != *_attack)
				coeffi1 = 8.0 / max((csfloat)8, *_attack * sampleRate );
			lattack = *_attack;

			if (ldecay != *_decay)
				coeffi2 = -8.0 / max((csfloat)8, *_decay * sampleRate );
			ldecay = *_decay;
	}


	// -.-.- calculate voices -.-.-

	int highestActive = 0;
	csfloat o = 0.0;
	*_outm = 0.0;
	for (int i=0;i<nrActive;i++)
	{
		// check envelope stage
		switch(active[i])
		{
			// ignore this voice
			case 0: continue; break;

			// attack
			case 1:
				if (!imidiateEnvelope) coeffi1 = coeff1[i];
				env[i] += coeffi1 * (velo[i]-env[i]);

				// mode change
				if (env[i]>=0.999*velo[i]-0.0018* *_attack) active[i] = 2;

				// remember highest active voice
				highestActive = i;
			break;

			// decay
			case 2:
				if (!imidiateEnvelope) coeffi2 = coeff2[i];
				env[i] += coeffi2 * env[i];

				// turn off
				if (env[i]<=0.0001*velo[i])
				{
					active[i] = 0.0;
				} else
				// remember highest active voice
				highestActive = i;
			break;

		} // end envelope

		// generate voice

		csfloat p = phase[i] + *_phm, Pw;
		switch (waveForm[i])
		{
			case 0:	// sin
				o = sinf(p);
				break;

			case 1:	// cos
				o = cosf(p);
			break;

			case 2:	// --- sqr ---
				p /= TWO_PI;
				if (p>=0) // positive phase
				{
					p = MODULO(p,1.0);
					o = (p>=pw[i])? -1.0 : 1.0;
				}
				else // negative phase
				{
					p = MODULO(-p,1.0);
					o= (p>=1.0-pw[i])? -1.0 : 1.0;
				}
			break;

			case 3:	// --- saw ---
				p = MODULO( fabs( p/TWO_PI ), 1.0);
				o = 2.0 * ( p - 0.5 );
			break;

			case 4:	// --- tri ---
				if (p>=0)
				{
					p = MODULO( p/TWO_PI , 1.0);
					Pw = pw[i];
				}
				else
				{
					p = MODULO( -p/TWO_PI , 1.0);
					Pw = 1.0 - pw[i];
				}
				Pw = max((csfloat)0.00001,min((csfloat)0.99999, Pw ));
				if (p<Pw)
					o = 2.0 * ( 0.5 - (Pw-p)/Pw );
				else
					o = 2.0 * ( (1.0-p)/(1.0-Pw) - 0.5 );
			break;

		}

		// scale and mix
		o *= *_amp * env[i];
		*_outm += o;
		*_out[i] = o;

		// increment phase
		phase[i] += freq[i];
		// wrap phase around to avoid overflow
		if (phase[i]>TWO_PI) phase[i]-=TWO_PI;
		else
		if (phase[i]<-TWO_PI) phase[i]+=TWO_PI;

	} // end voices

	// limit index cycles
	nrActive = min(nrActive, highestActive+1);


	// picnic!
	if ((lpanic<=0.0)&&(*_panic>0.0)) panic();
	lpanic = *_panic;

}


















char *CSmodule_SynthMultiQuadPos::docString()
{
	const char *r = "\
polyphonic multi waveform, AD enveloped, quadro panned synthesizer.\n\n\
the number of voices is maximally limited to 128. the nr of voices that are output separately \
is set in properties. the number of actual voices to use is set in the <nr voices> input of the module.\n\n\
a gate in <panic!> will ultimately reset all synth parameters.\n\
<amplitude> will scale all output channels.\n\n\
everything below and including <gate> works polyphonic, that means that separate values of the inputs \
are sampled and hold for each voic whenever a <gate> occures.\n\n\
<freq (hz)> simply sets the frequency in hertz of the synthesizer voice.\n\
<velocity> set the amplitude of the voice.\n\
<attack> and <decay> set the attack and decay phase of the envelope of each synthesizer voice in \
(aproximately) seconds. NOTE: in properties the envelope inputs can be set to immidiate mode which \
makes them act on on all voices all the time.\n\n\
<wave form> selects the waveform to be used by each voice. the values are 0 sin, 1 cosine, 2 square, \
3 saw, 4 triangle.\n\
the <pulse width> input works the same way as in the normal oscillator module on the square and triangle \
waveforms.\n\
<phase mod> modulates the phase of all oscillator voices.\n\n\
each voice is located on a circle and panned/mixed accordingly to 4 rectangular arranged output channels.\n\
<angle> defines the position on that circle with 0.0 = front, 0.25 = left, 0.5 = rear, 0.75 = right.\n\
<modulation amt> sets the influence of the panning. 0 = no influence, 1.0 = linear influence, >1.0 = \
potentized influence.\
";
	return const_cast<char*>(r);
}


CSmodule_SynthMultiQuadPos* CSmodule_SynthMultiQuadPos::newOne()
{
	return new CSmodule_SynthMultiQuadPos();
}


void CSmodule_SynthMultiQuadPos::init()
{
	CSmodule_SynthMulti::init();
	free(bname); bname = copyString("synth (multi/quadpos)");
	free(name); name = copyString("synth (quadpos)");
	rename(name);

	// --- inputs outputs ----

	_angle = &(createInput("ang","angle")->value);
	_mod = &(createInput("mod","modulation amt",1.0)->value);

	//int i = getConnectorIndex("om");
	//if (i>0) deleteConnector(i);

	_outrl = &(createOutput("orl","rear left")->value);
	_outrr = &(createOutput("orr","rear right")->value);
	_outfr = &(createOutput("ofr","front right")->value);
	_outfl = &(createOutput("ofl","front left")->value);

	for (int i=0;i<CSMOD_MIDI_MAX_POLY;i++)
	{
		amprl[i] = amprr[i] = ampfl[i] = ampfr[i] = 0.0;
	}
}


void CSmodule_SynthMultiQuadPos::step()
{
	// start a voice
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		// check voice index
		if ((voiceNr>=CSMOD_MIDI_MAX_POLY) || (voiceNr>=*_nrVoices))
			voiceNr = 0.0;

		// assign initial parameters

		active[voiceNr] = 1.0;
		env[voiceNr] = 0.0;
		freq[voiceNr] = *_freq * TWO_PI * isampleRate;
		velo[voiceNr] = *_vel;
		phase[voiceNr] = 0.0;
		waveForm[voiceNr] = abs((int)*_wave) % 5;
		pw[voiceNr] = max((csfloat)0.00001,min((csfloat)0.99999, *_pw ));;

		// calculate envelope coefficients
		coeff1[voiceNr] = 8.0 / max((csfloat)8, *_attack * sampleRate );
		coeff2[voiceNr] = -8.0 / max((csfloat)8, *_decay * sampleRate );

		// circular amplitude
		csfloat a = (-*_angle + 0.625) * TWO_PI;

		amprl[voiceNr] = powf( 0.5+0.5*sin( a ), *_mod);
		amprr[voiceNr] = powf( 0.5+0.5*sin( a+HALF_PI ), *_mod);
		ampfr[voiceNr] = powf( 0.5+0.5*sin( a+PI ), *_mod);
		ampfl[voiceNr] = powf( 0.5+0.5*sin( a+PI+HALF_PI ), *_mod);


		// increment voice index
		voiceNr++;
		nrActive = max(voiceNr, nrActive);
	}
	lgate = *_gate;

	// calculate override envelope coeff
	if (imidiateEnvelope)
	{
			if (lattack != *_attack)
				coeffi1 = 8.0 / max((csfloat)8, *_attack * sampleRate );
			lattack = *_attack;

			if (ldecay != *_decay)
				coeffi2 = -8.0 / max((csfloat)8, *_decay * sampleRate );
			ldecay = *_decay;
	}


	// -.-.- calculate voices -.-.-

	int highestActive = 0;
	csfloat o = 0.0;
	*_outm = *_outrl = *_outrr = *_outfr = *_outfl = 0.0;
	for (int i=0;i<nrActive;i++)
	{
		// check envelope stage
		switch(active[i])
		{
			// ignore this voice
			case 0: continue; break;

			// attack
			case 1:
				if (!imidiateEnvelope) coeffi1 = coeff1[i];
				env[i] += coeffi1 * (velo[i]-env[i]);

				// mode change
				if (env[i]>=0.999*velo[i]-0.0018* *_attack) active[i] = 2;

				// remember highest active voice
				highestActive = i;
			break;

			// decay
			case 2:
				if (!imidiateEnvelope) coeffi2 = coeff2[i];
				env[i] += coeffi2 * env[i];

				// turn off
				if (env[i]<=0.0001*velo[i])
				{
					active[i] = 0.0;
				} else
				// remember highest active voice
				highestActive = i;
			break;

		} // end envelope

		// generate voice

		csfloat
			p = phase[i] + *_phm,
			Pw;
		switch (waveForm[i])
		{
			case 0:	// sin
				o = sinf(p);
				break;

			case 1:	// cos
				o = cosf(p);
			break;

			case 2:	// --- sqr ---
				p /= TWO_PI;
				if (p>=0) // positive phase
				{
					p = MODULO(p,1.0);
					o = (p>=pw[i])? -1.0 : 1.0;
				}
				else // negative phase
				{
					p = MODULO(-p,1.0);
					o= (p>=1.0-pw[i])? -1.0 : 1.0;
				}
			break;

			case 3:	// --- saw ---
				p = MODULO( fabs( p/TWO_PI ), 1.0);
				o = 2.0 * ( p - 0.5 );
			break;

			case 4:	// --- tri ---
				if (p>=0)
				{
					p = MODULO( p/TWO_PI , 1.0);
					Pw = pw[i];
				}
				else
				{
					p = MODULO( -p/TWO_PI , 1.0);
					Pw = 1.0 - pw[i];
				}
				Pw = max((csfloat)0.00001,min((csfloat)0.99999, Pw ));
				if (p<Pw)
					o = 2.0 * ( 0.5 - (Pw-p)/Pw );
				else
					o = 2.0 * ( (1.0-p)/(1.0-Pw) - 0.5 );
			break;

		}

		// scale and mix
		o *= *_amp * env[i];
		*_out[i] = o;
		*_outm += o;

		// quadro mix
		*_outrl += amprl[i] * o;
		*_outrr += amprr[i] * o;
		*_outfr += ampfr[i] * o;
		*_outfl += ampfl[i] * o;

		// increment phase
		phase[i] += freq[i];
		// wrap phase around to avoid overflow
		if (phase[i]>TWO_PI) phase[i]-=TWO_PI;
		else
		if (phase[i]<-TWO_PI) phase[i]+=TWO_PI;

	} // end voices

	// limit index cycles
	nrActive = min(nrActive, highestActive+1);


	// picnic!
	if ((lpanic<=0.0)&&(*_panic>0.0)) panic();
	lpanic = *_panic;

}
















char *CSmodule_SynthMulti3D::docString()
{
	const char *r = "\
polyphonic multi waveform, AD enveloped, 3d panned synthesizer.\n\n\
the number of voices is limited to maximally 64. the nr of voices that are output separately \
is set in properties. the number of actual voices to use is set in the <nr voices> input of the module.\n\n\
a gate in <panic!> will ultimately reset all synth parameters.\n\
<amplitude> will scale all output channels.\n\
\n\
<center x/y/z> define the center of the listener. the module operates similiar to the microphone3d module. \
the individual microphones relative positions around the listener are set in the property window.\n\
<radius> is the maximum listening radius around the listener.\n\
<falloff> is the falloff curve (potence) of the amplitude depending on the distance of the sound. \
< 1.0 will decrease the influence, > 1.0 will increase the influence.\n\
<speed of sound> sets the nr of units the sound travels in one second.\n\
\n\
everything below and including <gate> works polyphonic, that means that separate values of the inputs \
are sampled and hold for each voice whenever a <gate> occures.\n\n\
<freq (hz)> simply sets the frequency in hertz of the synthesizer voice.\n\
<velocity> set the amplitude of the voice.\n\
<attack> and <decay> set the attack and decay phase of the envelope of each synthesizer voice in \
(aproximately) seconds. NOTE: in properties the envelope inputs can be set to immidiate mode which \
makes them act on on all voices all the time.\n\n\
<wave form> selects the waveform to be used by each voice. the values are 0 sin, 1 cosine, 2 square, \
3 saw, 4 triangle.\n\
the <pulse width> input works the same way as in the normal oscillator module on the square and triangle \
waveforms.\n\
<phase mod> modulates the phase of all oscillator voices.\n\n\
each voice is located on a circle and panned/mixed accordingly to 4 rectangular arranged output channels.\n\
<angle> defines the position on that circle with 0.0 = front, 0.25 = left, 0.5 = rear, 0.75 = right.\n\
<modulation amt> sets the influence of the panning. 0 = no influence, 1.0 = linear influence, >1.0 = \
potentized influence.\n\n\
<X>, <Y>, and <Z> supply the (fixed) position of the voice.\n\
\n\
NOTE: for speed reasons, in this module, the microphones are set in the properties (to precalculated some values). \
unfortunately, since the position of a polyphonic voice can not be changed, once it is started, it is not \
possible to rotate around the sounds. you can only move the center (and the microphones around it) but not \
rotate the microphones.\
";
	return const_cast<char*>(r);
}


CSmodule_SynthMulti3D* CSmodule_SynthMulti3D::newOne()
{
	return new CSmodule_SynthMulti3D();
}


void CSmodule_SynthMulti3D::init()
{
	CSmodule_SynthMulti::init();
	free(bname); bname = copyString("synth (multi/3d)");
	free(name); name = copyString("synth (3d)");
	rename(name);

	// --- inputs outputs ----

	int i=getConnectorIndex("g");
	_cz = &(insertInput(i,"cz","center Z")->value);
	_cy = &(insertInput(i,"cy","center Y")->value);
	_cx = &(insertInput(i,"cx","center X")->value);
	_sos = &(insertInput(i,"sos","speed of sound",330.0)->value);
	_falloff = &(insertInput(i,"fo","falloff",1.0)->value);
	_rad = &(insertInput(i,"rad","radius",100.0)->value);

	_x = &(createInput("x","X")->value);
	_y = &(createInput("y","Y")->value);
	_z = &(createInput("z","Z")->value);

	char n[128],n1[128];
	for (int i=0;i<CSMOD_SYNTH3D_CHAN;i++)
	{
		sprintf(n,"outmic%d",i);
		sprintf(n1,"mic %d",i+1);
		__outsp[i] = createOutput(n,n1);
		_outsp[i] = &__outsp[i]->value;
	}

	// --- internals ---

	mx[0] = -1.0; // back left (on dome master: top left)
	my[0] = 1.0;
	mz[0] = 0.1;

	mx[1] = 1.0; // back right
	my[1] = 1.0;
	mz[1] = 0.1;

	mx[2] = 1.0; // front right (on dome master: bottom right)
	my[2] = -1.0;
	mz[2] = 0.1;

	mx[3] = -1.0; // front left
	my[3] = -1.0;
	mz[3] = 0.1;

	mx[4] = 0.0; // zenit
	my[4] = 0.0;
	mz[4] = 1.0;

	mx[5] = 0.0; // behind
	my[5] = 0.0;
	mz[5] = -1.0;

	calcMicros();

	bufpos = 0.0;
	for (int i=0;i<CSMOD_SYNTH3D_MAX_POLY;i++)
	{
		//
	}

	// --- properties ---

	for (int i=0;i<CSMOD_SYNTH3D_CHAN;i++)
	{
		sprintf(n,"mx%d",i);
		sprintf(n1,"micro#%d X", i+1);
		createPropertyFloat(n,n1,mx[i]);
		sprintf(n,"my%d",i);
		sprintf(n1,"micro#%d Y", i+1);
		createPropertyFloat(n,n1,my[i]);
		sprintf(n,"mz%d",i);
		sprintf(n1,"micro#%d Z", i+1);
		createPropertyFloat(n,n1,mz[i]);
	}

}

void CSmodule_SynthMulti3D::propertyCallback(CSmodProperty *prop)
{
	/* TODO: thats not 'very very' clever... */
	if (strcmp(prop->name,"mx0")==0) mx[0] = prop->fvalue; else
	if (strcmp(prop->name,"my0")==0) my[0] = prop->fvalue; else
	if (strcmp(prop->name,"mz0")==0) mz[0] = prop->fvalue; else
	if (strcmp(prop->name,"mx1")==0) mx[1] = prop->fvalue; else
	if (strcmp(prop->name,"my1")==0) my[1] = prop->fvalue; else
	if (strcmp(prop->name,"mz1")==0) mz[1] = prop->fvalue; else
	if (strcmp(prop->name,"mx2")==0) mx[2] = prop->fvalue; else
	if (strcmp(prop->name,"my2")==0) my[2] = prop->fvalue; else
	if (strcmp(prop->name,"mz2")==0) mz[2] = prop->fvalue; else
	if (strcmp(prop->name,"mx3")==0) mx[3] = prop->fvalue; else
	if (strcmp(prop->name,"my3")==0) my[3] = prop->fvalue; else
	if (strcmp(prop->name,"mz3")==0) mz[3] = prop->fvalue; else
	CSmodule_Synth::propertyCallback(prop);
	calcMicros();
}

void CSmodule_SynthMulti3D::calcMicros()
{
	csfloat n;
	for (int i=0;i<CSMOD_SYNTH3D_CHAN;i++)
	{
		// normalize cam->micro vector
		n = CSdist0(mx[i],my[i],mz[i]);
		dcm[i] = n;
		if (n>0.0) n = 1.0/n; else n = 0.0;
		cmx[i] = n*mx[i];
		cmy[i] = n*my[i];
		cmz[i] = n*mz[i];
	}
}

void CSmodule_SynthMulti3D::step()
{
	csfloat
		o = 0.0,
		di,d,b,p,f,
		csx,csy,csz;

	// start a voice
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		// check voice index
		if ((voiceNr>=CSMOD_SYNTH3D_MAX_POLY) || (voiceNr>=*_nrVoices))
			voiceNr = 0.0;

		// assign initial parameters

		active[voiceNr] = 1.0;
		env[voiceNr] = 0.0;
		freq[voiceNr] = *_freq * TWO_PI * isampleRate;
		velo[voiceNr] = *_vel;
		phase[voiceNr] = 0.0;
		waveForm[voiceNr] = abs((int)*_wave) % 5;
		pw[voiceNr] = max((csfloat)0.00001,min((csfloat)0.99999, *_pw ));;

		// calculate envelope coefficients
		coeff1[voiceNr] = 8.0 / max((csfloat)8, *_attack * sampleRate );
		coeff2[voiceNr] = -8.0 / max((csfloat)8, *_decay * sampleRate );

		// save 3d position
		x[voiceNr] = *_x;
		y[voiceNr] = *_y;
		z[voiceNr] = *_z;

		// increment voice index
		voiceNr++;
		nrActive = max(voiceNr, nrActive);
	}
	lgate = *_gate;

	// calculate override envelope coeff
	if (imidiateEnvelope)
	{
			if (lattack != *_attack)
				coeffi1 = 8.0 / max((csfloat)8, *_attack * sampleRate );
			lattack = *_attack;

			if (ldecay != *_decay)
				coeffi2 = -8.0 / max((csfloat)8, *_decay * sampleRate );
			ldecay = *_decay;
	}


	// -.-.- calculate voices -.-.-

	int highestActive = 0;
	*_outm = 0.0;

	for (int i=0;i<CSMOD_SYNTH3D_CHAN;i++)
		*_outsp[i] = 0.0;
	for (int i=0;i<nrActive;i++)
	{
		// check envelope stage
		switch(active[i])
		{
			// ignore this voice
			case 0: continue; break;

			// attack
			case 1:
				if (!imidiateEnvelope) coeffi1 = coeff1[i];
				env[i] += coeffi1 * (velo[i]-env[i]);

				// mode change
				if (env[i]>=0.999*velo[i]-0.0018* *_attack) active[i] = 2;

				// remember highest active voice
				highestActive = i;
			break;

			// decay
			case 2:
				if (!imidiateEnvelope) coeffi2 = coeff2[i];
				env[i] += coeffi2 * env[i];

				// turn off
				if (env[i]<=0.0001*velo[i])
				{
					active[i] = 0.0;
				} else
				// remember highest active voice
				highestActive = i;
			break;

		} // end envelope

		// generate voice

		p = phase[i] + *_phm;
		csfloat Pw;
		switch (waveForm[i])
		{
			case 0:	// sin
				o = sinf(p);
				break;

			case 1:	// cos
				o = cosf(p);
			break;

			case 2:	// --- sqr ---
				p /= TWO_PI;
				if (p>=0) // positive phase
				{
					p = MODULO(p,1.0);
					o = (p>=pw[i])? -1.0 : 1.0;
				}
				else // negative phase
				{
					p = MODULO(-p,1.0);
					o= (p>=1.0-pw[i])? -1.0 : 1.0;
				}
			break;

			case 3:	// --- saw ---
				p = MODULO( fabs( p/TWO_PI ), 1.0);
				o = 2.0 * ( p - 0.5 );
			break;

			case 4:	// --- tri ---
				if (p>=0)
				{
					p = MODULO( p/TWO_PI , 1.0);
					Pw = pw[i];
				}
				else
				{
					p = MODULO( -p/TWO_PI , 1.0);
					Pw = 1.0 - pw[i];
				}
				Pw = max((csfloat)0.00001,min((csfloat)0.99999, Pw ));
				if (p<Pw)
					o = 2.0 * ( 0.5 - (Pw-p)/Pw );
				else
					o = 2.0 * ( (1.0-p)/(1.0-Pw) - 0.5 );
			break;

		}

		// scale and mix
		o *= *_amp * env[i];
		*_out[i] = o;
		*_outm += o;

		// store in delay buf
		int dp = i<<CSMOD_SYNTH3D_MAX_DELAY_SHIFT;
		dbuf[dp+bufpos] = o;

		// increment phase
		phase[i] += freq[i];
		// wrap phase around to avoid overflow
		if (phase[i]>TWO_PI) phase[i]-=TWO_PI;
		else
		if (phase[i]<-TWO_PI) phase[i]+=TWO_PI;


		// --- 3d voices ---

		// get distance center->sound
		csx = x[i] - *_cx;
		di = csx*csx;
		csy = y[i] - *_cy;
		di += csy*csy;
		csz = z[i] - *_cz;
		di = sqrtf(di+csz*csz);
		dist[i] = di;

		// skip when out of range
		if (di>*_rad) continue;

		// normalize center/sound vector
		b = CSdist0(csx,csy,csz);
		if (b>0) { b = 1.0/b; csx *= b; csy *= b; csz *= b; }

		// calc delay pos
		d = di * sampleRate / max((csfloat)0.001, *_sos);
		d = min((csfloat)CSMOD_SYNTH3D_MAX_DELAY-1, max((csfloat)1, d ));
		int dt = (int)d;

		// read delayed voice
		o = dbuf[ dp + ((bufpos-dt+CSMOD_SYNTH3D_MAX_DELAY) & (CSMOD_SYNTH3D_MAX_DELAY-1)) ];

		// interpolate delay
		d -= dt;
		b = dbuf[ dp + ((bufpos-dt-1+(CSMOD_SYNTH3D_MAX_DELAY<<1)) & (CSMOD_SYNTH3D_MAX_DELAY-1)) ];
		o = o*(1.0-d)+d*b;

		// amplitude from distance
		if (*_falloff != 1.0)
			o *= powf(1.0 - di / *_rad, *_falloff);
		else
			o *= (1.0 - di / *_rad);

		// pan to 3d outputs
		for (int j=0;j<CSMOD_SYNTH3D_CHAN;j++)
		{
	        // angle amplitude, distance normalized center->sound : center->micro
			b = 1.0 - CSdist(csx,csy,csz, cmx[j],cmy[j],cmz[j])/2.0;
			//if (*_ang!=1.0) am = pow(am, *_ang);

			// blend inside circle
			if ( (dcm[j]>0.0) && (di<dcm[j]))
			{
				f = (dcm[j]-di)/dcm[j];
				b = b*(1.0-f) + f;
			}
			// smooth the spikes
			b = 1.0-(1.0-b)*(1.0-b);

			// send to micro output
			*_outsp[j] += o * b;
		}


	} // end voices

	// limit index cycles
	nrActive = min(nrActive, highestActive+1);

	// increase delay buf index
	bufpos = (bufpos+1) & (CSMOD_SYNTH3D_MAX_DELAY-1);

	// picnic!
	if ((lpanic<=0.0)&&(*_panic>0.0)) panic();
	lpanic = *_panic;

}


















char *CSmodule_SynthMultiFilter3D::docString()
{
	const char *r = "\
polyphonic multi waveform, AD enveloped, lowpass filtered, 3d panned synthesizer.\n\n\
the number of voices is limited to 64. the nr of voices that are output separately \
is set in properties. the number of actual voices to use is set in the <nr voices> input of the module.\n\n\
a gate in <panic!> will ultimately reset all synth parameters.\n\
<amplitude> will scale all output channels.\n\
\n\
<center x/y/z> define the center of the listener. the module operates similiar to the microphone3d module. \
the individual microphones relative positions around the listener are set in the property window.\n\
<radius> is the maximum listening radius around the listener.\n\
<falloff> is the falloff curve (potence) of the amplitude depending on the distance of the sound. \
< 1.0 will decrease the influence, > 1.0 will increase the influence.\n\
<speed of sound> sets the nr of units the sound travels in one second.\n\
\n\
everything below and including <gate> works polyphonic, that means that separate values of the inputs \
are sampled and hold for each voice whenever a <gate> occures.\n\n\
<freq (hz)> simply sets the frequency in hertz of the synthesizer voice.\n\
<velocity> set the amplitude of the voice.\n\
<attack> and <decay> set the attack and decay phase of the envelope of each synthesizer voice in \
(aproximately) seconds. NOTE: in properties the envelope inputs can be set to immidiate mode which \
makes them act on on all voices all the time.\n\n\
<wave form> selects the waveform to be used by each voice. the values are 0 sin, 1 cosine, 2 square, \
3 saw, 4 triangle.\n\
the <pulse width> input works the same way as in the normal oscillator module on the square and triangle \
waveforms.\n\
<phase mod> modulates the phase of all oscillator voices.\n\n\
each voice is located on a circle and panned/mixed accordingly to 4 rectangular arranged output channels.\n\
<angle> defines the position on that circle with 0.0 = front, 0.25 = left, 0.5 = rear, 0.75 = right.\n\
<modulation amt> sets the influence of the panning. 0 = no influence, 1.0 = linear influence, >1.0 = \
potentized influence.\n\
\n\
<filter> is between 0.0 and 1.0 and sets the frequency of the low pass filter for each voice. a value of 1.0 is \
the sampling rate, 0.5 is 12.5% of the sampling rate, aso...\n\
<filter reso> is the resonance of the filter between 0.0 and 0.99999. a value of >= ~0.999 starts self-resonance of \
the filter.\n\
<filter key> sets the influence of the voice's frequency on the filter cutoff frequency. the (somewhat scaled) \
frequency is added to the actual <filter> frequency by the amount of <filter key>. a 1.0 will match about the \
frequency of the voice. values can be greater than 1.0 and negative as well.\n\
<filter env> sets the influence of a seperate envelope for the filter frequency. the envelope is added to the \
fixed filter frequency by the amount of <filter env>. the value can be greather than 1.0 and negative as well.\n\
<filter env A> and <filter env D> are the attack and decay times for the filter envelope. they work as the normal \
amlitude envelopes attack and decay values.\n\
\n\
<X>, <Y>, and <Z> supply the (fixed) position of the voice.\n\
\n\
NOTE: for speed reasons, in this module, the microphones are set in the properties (to precalculated some values). \
unfortunately, since the position of a polyphonic voice can not be changed, once it is started, it is not \
possible to rotate around the sounds. you can only move the center (and the microphones around it) but not \
rotate the microphones.\
";
	return const_cast<char*>(r);
}


CSmodule_SynthMultiFilter3D* CSmodule_SynthMultiFilter3D::newOne()
{
	return new CSmodule_SynthMultiFilter3D();
}


void CSmodule_SynthMultiFilter3D::init()
{
	CSmodule_SynthMulti3D::init();
	free(bname); bname = copyString("synth (filter/3d)");
	free(name); name = copyString("synth (filter/3d)");
	rename(name);

	// --- inputs outputs ----

	int i=getConnectorIndex("x");
	_fdecay = &(insertInput(i,"fenvd","filter env D",1.0)->value);
	_fattack = &(insertInput(i,"fenva","filter env A",0.03)->value);
	_fenv = &(insertInput(i,"fenv","filter env",0.0)->value);
	_cutoff_kf = &(insertInput(i,"fltkf","filter key",0.0)->value);
	_cutoff_res = &(insertInput(i,"fltr","filter reso",0.0)->value);
	_cutoff = &(insertInput(i,"flt","filter",1.0)->value);



	// --- properties ---

}

void CSmodule_SynthMultiFilter3D::propertyCallback(CSmodProperty *prop)
{
	CSmodule_SynthMulti3D::propertyCallback(prop);
}


void CSmodule_SynthMultiFilter3D::panic()
{
	CSmodule_Synth::panic();
	for (int i=0;i<CSMOD_SYNTH3D_MAX_POLY;i++)
	{
		fcoeff1[i] = fcoeff2[i] =
		fecoeff1[i] = fecoeff2[i] =
		fenv[i] = fenvmax[i] = 0.0;
		fenvmode[i] = 0;
	}
	for (int i=0;i<CSMOD_SYNTH3D_MAX_POLY*2;i++)
		fbuf[i] = 0.0;
}

void CSmodule_SynthMultiFilter3D::step()
{
	csfloat
		o = 0.0,
		di,d,b,p,f,
		csx,csy,csz;

	// start a voice
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		// check voice index
		if ((voiceNr>=CSMOD_SYNTH3D_MAX_POLY) || (voiceNr>=*_nrVoices))
			voiceNr = 0.0;

		// assign initial parameters

		// normalized frequency
		b = *_freq * isampleRate;

		active[voiceNr] = 1.0;
		env[voiceNr] = 0.0;
		freq[voiceNr] = b * TWO_PI;
		velo[voiceNr] = *_vel;
		phase[voiceNr] = 0.0;
		waveForm[voiceNr] = abs((int)*_wave) % 5;
		pw[voiceNr] = max((csfloat)0.00001,min((csfloat)0.99999, *_pw ));;

		// calculate envelope coefficients
		coeff1[voiceNr] = 8.0 / max((csfloat)8, *_attack * sampleRate );
		coeff2[voiceNr] = -8.0 / max((csfloat)8, *_decay * sampleRate );

		// calc filter coefficients
		fcoeff1[voiceNr] =
			// cutoff
			*_cutoff +
			// key follow * norm freq
			*_cutoff_kf * b * 20.0;

		fcoeff2[voiceNr] = max((csfloat)0.0, min((csfloat)0.99999,
			*_cutoff_res ));

		// filter envelope coefficients
		fenvmax[voiceNr] = *_fenv;
		fecoeff1[voiceNr] = 8.0 / max((csfloat)8, *_fattack * sampleRate );
		fecoeff2[voiceNr] = -8.0 / max((csfloat)8, *_fdecay * sampleRate );
		fenv[voiceNr] = 0.0;
		fenvmode[voiceNr] = 0;

		// save 3d position
		x[voiceNr] = *_x;
		y[voiceNr] = *_y;
		z[voiceNr] = *_z;

		// increment voice index
		voiceNr++;
		nrActive = max(voiceNr, nrActive);
	}
	lgate = *_gate;

	// calculate override envelope coeff
	if (imidiateEnvelope)
	{
			if (lattack != *_attack)
				coeffi1 = 8.0 / max((csfloat)8, *_attack * sampleRate );
			lattack = *_attack;

			if (ldecay != *_decay)
				coeffi2 = -8.0 / max((csfloat)8, *_decay * sampleRate );
			ldecay = *_decay;
	}


	// -.-.- calculate voices -.-.-

	int highestActive = 0;
	*_outm = 0.0;

	for (int i=0;i<CSMOD_SYNTH3D_CHAN;i++)
		*_outsp[i] = 0.0;
	for (int i=0;i<nrActive;i++)
	{
		// check envelope stage
		switch(active[i])
		{
			// ignore this voice
			case 0: continue; break;

			// attack
			case 1:
				if (!imidiateEnvelope) coeffi1 = coeff1[i];
				env[i] += coeffi1 * (velo[i]-env[i]);

				// mode change
				if (env[i]>=0.999*velo[i]-0.0018* *_attack) active[i] = 2;

				// remember highest active voice
				highestActive = i;
			break;

			// decay
			case 2:
				if (!imidiateEnvelope) coeffi2 = coeff2[i];
				env[i] += coeffi2 * env[i];

				// turn off
				if (env[i]<=0.0001*velo[i])
				{
					active[i] = 0.0;
				} else
				// remember highest active voice
				highestActive = i;
			break;

		} // end envelope

		// generate voice

		p = phase[i] + *_phm;
		csfloat Pw;
		switch (waveForm[i])
		{
			case 0:	// sin
				o = sinf(p);
				break;

			case 1:	// cos
				o = cosf(p);
			break;

			case 2:	// --- sqr ---
				p /= TWO_PI;
				if (p>=0) // positive phase
				{
					p = MODULO(p,1.0);
					o = (p>=pw[i])? -1.0 : 1.0;
				}
				else // negative phase
				{
					p = MODULO(-p,1.0);
					o= (p>=1.0-pw[i])? -1.0 : 1.0;
				}
			break;

			case 3:	// --- saw ---
				p = MODULO( fabs( p/TWO_PI ), 1.0);
				o = 2.0 * ( p - 0.5 );
			break;

			case 4:	// --- tri ---
				if (p>=0)
				{
					p = MODULO( p/TWO_PI , 1.0);
					Pw = pw[i];
				}
				else
				{
					p = MODULO( -p/TWO_PI , 1.0);
					Pw = 1.0 - pw[i];
				}
				Pw = max((csfloat)0.00001,min((csfloat)0.99999, Pw ));
				if (p<Pw)
					o = 2.0 * ( 0.5 - (Pw-p)/Pw );
				else
					o = 2.0 * ( (1.0-p)/(1.0-Pw) - 0.5 );
			break;

		}

		// scale
		o *= *_amp * env[i];

		// filter envelope
		switch (fenvmode[i])
		{
			case 0:
				fenv[i] += fecoeff1[i] * (fenvmax[i]-fenv[i]);
				// mode change
				if (fabs(fenvmax[i]-fenv[i])<0.001) fenvmode[i] = 1;
				break;
			default:
				fenv[i] += fecoeff2[i] * fenv[i];
				break;
		}

		// filter
		csfloat *fo = &fbuf[(i<<1)],
				*fo1 = fo;
		fo1++;
		b = max((csfloat)0, min((csfloat)1.0, fcoeff1[i] + fenv[i] ));
		o = *fo
			// filtered sample
			+ b*b*b * ( o - *fo )
			// resonance
			+ fcoeff2[i] * ( *fo - *fo1 );
		*fo1 = *fo;
		*fo = o;

		// mix
		*_out[i] = o;
		*_outm += o;

		// store in delay buf
		int dp = i<<CSMOD_SYNTH3D_MAX_DELAY_SHIFT;
		dbuf[dp+bufpos] = o;

		// increment phase
		phase[i] += freq[i];
		// wrap phase around to avoid overflow
		if (phase[i]>TWO_PI) phase[i]-=TWO_PI;
		else
		if (phase[i]<-TWO_PI) phase[i]+=TWO_PI;


		// --- 3d voices ---

		// get distance center->sound
		csx = x[i] - *_cx;
		di = csx*csx;
		csy = y[i] - *_cy;
		di += csy*csy;
		csz = z[i] - *_cz;
		di = sqrtf(di+csz*csz);
		dist[i] = di;

		// skip when out of range
		if (di>*_rad) continue;

		// normalize center/sound vector
		b = CSdist0(csx,csy,csz);
		if (b>0) { b = 1.0/b; csx *= b; csy *= b; csz *= b; }

		// calc delay pos
		d = di * sampleRate / max((csfloat)0.001, *_sos);
		d = min((csfloat)CSMOD_SYNTH3D_MAX_DELAY-1, max((csfloat)1, d ));
		int dt = (int)d;

		// read delayed voice
		o = dbuf[ dp + ((bufpos-dt+CSMOD_SYNTH3D_MAX_DELAY) & (CSMOD_SYNTH3D_MAX_DELAY-1)) ];

		// interpolate delay
		d -= dt;
		b = dbuf[ dp + ((bufpos-dt-1+(CSMOD_SYNTH3D_MAX_DELAY<<1)) & (CSMOD_SYNTH3D_MAX_DELAY-1)) ];
		o = o*(1.0-d)+d*b;

		// amplitude from distance
		if (*_falloff != 1.0)
			o *= powf(1.0 - di / *_rad, *_falloff);
		else
			o *= (1.0 - di / *_rad);

		// pan to 3d outputs
		for (int j=0;j<CSMOD_SYNTH3D_CHAN;j++)
		{
	        // angle amplitude, distance normalized center->sound : center->micro
			b = 1.0 - CSdist(csx,csy,csz, cmx[j],cmy[j],cmz[j])/2.0;
			//if (*_ang!=1.0) am = pow(am, *_ang);

			// blend inside circle
			if ( (dcm[j]>0.0) && (di<dcm[j]))
			{
				f = (dcm[j]-di)/dcm[j];
				b = b*(1.0-f) + f;
			}
			// smooth the spikes
			b = 1.0-(1.0-b)*(1.0-b);

			// send to micro output
			*_outsp[j] += o * b;
		}


	} // end voices

	// limit index cycles
	nrActive = min(nrActive, highestActive+1);

	// increase delay buf index
	bufpos = (bufpos+1) & (CSMOD_SYNTH3D_MAX_DELAY-1);

	// picnic!
	if ((lpanic<=0.0)&&(*_panic>0.0)) panic();
	lpanic = *_panic;

}
