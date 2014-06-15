#include <math.h>
#include "csmodule_sampler.h"



char *CSmodule_SamplerPoly::docString()
{
	const char *r = "\
simple polyphonic AD enveloped sample player.\n\n\
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


CSmodule_SamplerPoly* CSmodule_SamplerPoly::newOne()
{
	return new CSmodule_SamplerPoly();
}


CSmodule_SamplerPoly::~CSmodule_SamplerPoly()
{
	if (wr) delete wr;
}

void CSmodule_SamplerPoly::init()
{
	gname = copyString("sampler");
	bname = copyString("sampler (poly)");
	name = copyString(bname);
	uname = copyString(bname);

	wr = 0;

	if (!parent) return;

	// --- inputs outputs ----

	_nrVoices = &(createInput("nv","nr voices",4.0)->value);
	_panic = &(createInput("p","panic!",0.0)->value);
	_amp = &(createInput("amp","amplitude",1.0)->value);
	_rootFreq = &(createInput("rf","root frequency",44100.0)->value);

	_gate = &(createInput("g","gate")->value);
	_vel = &(createInput("v","velocity",1.0)->value);
	_freq = &(createInput("f","freq (hz)",0.0)->value);
	_attack = &(createInput("at","attack",0.03)->value);
	_decay = &(createInput("de","decay",1.0)->value);
	_startpos = &(createInput("spos","start (sec)",0.0)->value);

	//_activeVoices = &(createOutput("av","nr voices")->value);
	_outm = &(createOutput("om","out mix")->value);

	char n[128];
	for (int i=0;i<CSMOD_MIDI_MAX_POLY;i++)
	{
		sprintf(n,"out%d",i+1);
		__out[i] = createOutput(n,n);
		_out[i] = &(__out[i]->value);
	}

	wr = new CSwaveReader();

	setNrOut(4);
	panic();
	lpanic = lgate = lattack = ldecay = 0.0;
	imidiateEnvelope = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyString("wavname", "wave file name", "-");
	createPropertyInt("nrout", "nr of outputs", nrOut, 1,CSMOD_MAX_CHAN);
	createPropertyInt("imienv", "imidiate envelope? (0/1)", imidiateEnvelope, 0,1);
}


void CSmodule_SamplerPoly::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"wavname")==0)
		loadWave(prop->svalue);
	else
	if (strcmp(prop->name,"nrout")==0)
		setNrOut(prop->ivalue);
	else
	if (strcmp(prop->name,"imienv")==0)
		imidiateEnvelope = prop->ivalue;
	else
	CSmodule::propertyCallback(prop);
}


void CSmodule_SamplerPoly::setNrOut(int newNrOut)
{
	nrOut = newNrOut;

	for (int i=0;i<CSMOD_MIDI_MAX_POLY;i++)
	{
		setConnectorActive(__out[i], (i<nrOut));
	}

	arrangeConnectors();
}


void CSmodule_SamplerPoly::loadWave(const char *filename)
{
	if (wr->open(filename))
	{
//		int i=getConnectorIndex("rf");
//		if (i>=0) setConnectorValue(i, wr->rate);
	}
}

void CSmodule_SamplerPoly::panic()
{
	for (int i=0;i<CSMOD_MIDI_MAX_POLY;i++)
	{
		active[i] = 0;
		pos[i] = 0;
		freq[i] = 0.0;
		velo[i] = 0.0;
		env[i] = 0.0;
		coeff1[i] = 0.0;
		coeff2[i] = 0.0;
		*_out[i] = 0.0;
		// request the beginning
		wr->getSamples(0,0,0);
	}
	*_outm = 0.0;
	voiceNr = 0;
	nrActive = 0;
}

void CSmodule_SamplerPoly::step()
{
	if (!wr->datasize) return;

	// start a voice
	if ((lgate<=0.0)&&(*_gate>0.0))
	{
		// check voice index
		if ((voiceNr>=CSMOD_MIDI_MAX_POLY) || (voiceNr>=*_nrVoices))
			voiceNr = 0.0;

		// assign initial parameters

		active[voiceNr] = 1.0;
		env[voiceNr] = 0.0;
		freq[voiceNr] = *_freq / max((csfloat)1, *_rootFreq);
		velo[voiceNr] = *_vel;
		pos[voiceNr] = *_startpos * wr->rate;

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

		// stop voice on sample edges
		if ((pos[i]<0)||(pos[i]>=wr->datalength))
		{
			active[i] = 0.0;
			continue;
		}

		// get the sample
		wr->getSamplesLinear(pos[i], 1, &o);
		o *= *_amp * env[i];
		*_outm += o;
		*_out[i] = o;

		// increment pos
		pos[i] += freq[i];

	} // end voices

	// limit index cycles
	nrActive = min(nrActive, highestActive+1);


	// picnic!
	if ((lpanic<=0.0)&&(*_panic>0.0)) panic();
	lpanic = *_panic;

}


void CSmodule_SamplerPoly::stepLow()
{
	wr->updateBuffer();
}
