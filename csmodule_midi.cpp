#ifdef CSMOD_USE_MIDI

#include <math.h>
#include "csmodule_midi.h"

#include "csmodcontainer.h"

char *CSmodule_NoteIn::docString()
{
	const char *r = "\
monophonic midi note in module.\n\n\
the module outputs the last received midi note number and it's velocity.\n\n\
if a note is already on (gate > 0.0) and the note number changes, the <gate> output will \
be negative for one sample step to reset possible connected envelopes and stuff.\
";
	return const_cast<char*>(r);
}


CSmodule_NoteIn* CSmodule_NoteIn::newOne()
{
	return new CSmodule_NoteIn();
}


void CSmodule_NoteIn::init()
{
	gname = copyString("midi");
	bname = copyString("note in");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_note = &(createOutput("n","note")->value);
	_gate = &(createOutput("g","gate")->value);

	lastNote = 0;

	// ---- properties ----

	createNameProperty();

}




void CSmodule_NoteIn::step()
{
	if (parentRoot)
	{
		*_note = parentRoot->lastNoteMono;
		*_gate = parentRoot->lastNoteVelMono;
		if (parentRoot->lastNoteMonoChanged || (parentRoot->lastNoteMono!=lastNote))
			*_gate = -*_gate;
		lastNote = parentRoot->lastNoteMono;
	}
}












char *CSmodule_NoteInPoly::docString()
{
	const char *r = "\
the module gathers note in midi signals and arranges them in a array of polyphonic note/gate pairs.\n\n\
the number of simultaneous notes is set in the properties. each voice has it's own note/gate outputs.\n\n\
if the midi key is released, it's entry in the array is deleted (note and gate set to 0).\n\
if there are more keys pressed than the module can output, the lowest key will be overwritten (in that case \
the gate signal will be negative for one sample and turn positve in the next sample. that way a connected \
envelope or whatever is forced to restart).\
";
	return const_cast<char*>(r);
}


CSmodule_NoteInPoly* CSmodule_NoteInPoly::newOne()
{
	return new CSmodule_NoteInPoly();
}


void CSmodule_NoteInPoly::init()
{
	gname = copyString("midi");
	bname = copyString("note in (poly)");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_trans = &createInput("tr","transpose")->value;
	_outNr = &createOutput("onr","nr voices")->value;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		char *n = int2char("note",i);
		__note[i] = createOutput(n,"note");
		_note[i] = &(__note[i]->value);
		free(n);

		n = int2char("gate",i);
		__gate[i] = createOutput(n,"gate");
		_gate[i] = &(__gate[i]->value);
		free(n);

	}

	nrMidiVoices = 4;
	nrMidiVoicesActive = 0;

	setNrOut(nrMidiVoices);

	midiPanic();

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nr", "polyphonic voices", nrMidiVoices, 1,CSMOD_MAX_NOTE);
}


void CSmodule_NoteInPoly::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nr")==0)
		setNrOut(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_NoteInPoly::setNrOut(int newNrOut)
{
	nrMidiVoices = newNrOut;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__note[i], (i<nrMidiVoices));
		setConnectorActive(__gate[i], (i<nrMidiVoices));
	}

	arrangeConnectors();
}

void CSmodule_NoteInPoly::midiPanic()
{
	for (int i=0;i<CSMOD_MAX_NOTE;i++)
	{
		noteNr[i] = 0;
		noteVel[i] = 0;
	}
	changed = true;
	changedScr = false;
}

void CSmodule_NoteInPoly::noteOn(unsigned char noteNr, unsigned char velocity)
{
	changed = true;
	changedScr = true;
	if (velocity>0)
	{
		int k;
		csfloat amp = 1.0;
		if (nrMidiVoicesActive>=nrMidiVoices)
		{
			// overwrite lowest
			k = 0;
			for (int i=1;i<nrMidiVoices;i++)
			if (this->noteNr[i]<this->noteNr[k])
				k = i;
			amp = -1.0;
		}
		else
		{
			for (int i=0;i<nrMidiVoices;i++)
			if (noteVel[i]==0)
				{ k = i; i = nrMidiVoices; }
			nrMidiVoicesActive++;
		}

		// write polyphonic buffer
		this->noteNr[k] = noteNr;
		noteVel[k] = amp * (csfloat)velocity/127;
	}
	else
	{
		// wipe out in buffer
		for (int i=0;i<nrMidiVoices;i++)
		if (this->noteNr[i]==noteNr)
		{
			this->noteNr[i]=0;
			noteVel[i]=0;
			nrMidiVoicesActive--;
			i=nrMidiVoices;
		}
	}
}


void CSmodule_NoteInPoly::step()
{
	if (!changed) return;
	changed = false;

	*_outNr = nrMidiVoicesActive;

	for (int i=0;i<nrMidiVoices;i++)
	{
		*_note[i] = *_trans + noteNr[i];
		*_gate[i] = noteVel[i];
	}

	for (int i=0;i<nrMidiVoices;i++)
	{
		// make positive the negative gates
		if (noteVel[i]<0)
		{
			noteVel[i] = -noteVel[i];
			changed = true;
		}
	}


}

void CSmodule_NoteInPoly::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);
	if (!changedScr) return;
	changedScr = false;

	// draw input indicator
	fl_color(FL_GREEN);
	fl_rectf(offx+zoom*(x+width-10),offy+zoom*(y+2),zoom*8,zoom*8);
}















char *CSmodule_ControllerIn::docString()
{
	const char *r = "\
this module outputs the midi control change values of a selected set of controllers.\n\n\
the number of controllers per module is set in the properties.\n\n\
each channel has a <nr> input to set the controller number to listen to and an <output> which \
represents the control change value.\n\n\
in properties the whole module can be set to listen to nrpn control changes. nrpn is a midi standard \
for sending 14 bit values and address controller numbers in 14 bit.\n\
note that to use nrpn controllers, you have to sacrifice the normal control change numbers of 6, 38, 98 and 99.\
";
	return const_cast<char*>(r);
}


CSmodule_ControllerIn* CSmodule_ControllerIn::newOne()
{
	return new CSmodule_ControllerIn();
}


void CSmodule_ControllerIn::init()
{
	gname = copyString("midi");
	bname = copyString("controller in");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	for (int i=0;i<CSMOD_MAX_CONTROLLER;i++)
	{
		char *n = int2char("nr",i);
		__nr[i] = createInput(n,"nr",i);
		_nr[i] = &(__nr[i]->value);
		free(n);

		n = int2char("out",i);
		__out[i] = createOutput(n,"value");
		_out[i] = &(__out[i]->value);
		free(n);

		oldNr[i] = -1;

	}

	setNrIn(1);
	changed = true;
	changedScr = false;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nr", "nr of controllers", nrIn, 1,CSMOD_MAX_CONTROLLER);
	createPropertyInt("nrpn","use NRPN controller?", 0,0,1);
}


void CSmodule_ControllerIn::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nr")==0)
		setNrIn(prop->ivalue);
	else
	if (strcmp(prop->name,"nrpn")==0)
		doNrpn = prop->ivalue;
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_ControllerIn::setNrIn(int newNrIn)
{
	nrIn = newNrIn;

	for (int i=0;i<CSMOD_MAX_CONTROLLER;i++)
	{
		setConnectorActive(__nr[i], (i<nrIn));
		setConnectorActive(__out[i], (i<nrIn));
	}

	arrangeConnectors();
}


void CSmodule_ControllerIn::controlChange(int controller, int value)
{
	if (doNrpn) return;
	changed = true;
	changedScr = true;
}

void CSmodule_ControllerIn::control14Change(int controller, int value)
{
	if (!doNrpn) return;
	changed = true;
	changedScr = true;
}

void CSmodule_ControllerIn::step()
{
	// check for change in controller numbers
	for (int i=0;i<nrIn;i++)
	if (*_nr[i]!=oldNr[i])
	{
		changed = true;
		oldNr[i] = *_nr[i];
	}
	if (!changed) return;
	changed = false;

	if (parentRoot)
	{
		if (doNrpn)
		for (int i=0;i<nrIn;i++)
		{
			int k = *_nr[i];
			if ((k>=0)&&(k<CSMOD_MAX_NRPN_CONTROLLER))
				*_out[i] = parentRoot->controller14Value[k];
		}
		else
		for (int i=0;i<nrIn;i++)
		{
			int k = *_nr[i];
			if ((k>=0)&&(k<CSMOD_MAX_CONTROLLER))
				*_out[i] = parentRoot->controllerValue[k];
		}
	}
}


void CSmodule_ControllerIn::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);
	if (!changedScr) return;
	changedScr = false;

	// draw input indicator
	fl_color(FL_GREEN);
	fl_rectf(offx+zoom*(x+width-10),offy+zoom*(y+2),zoom*8,zoom*8);
}















char *CSmodule_MidiFile::docString()
{
	const char *r = "\
load a midi file\
";
	return const_cast<char*>(r);
}

CSmodule_MidiFile::~CSmodule_MidiFile()
{
	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
	{
		if (track[i].note) free(track[i].note);
	}
}

CSmodule_MidiFile* CSmodule_MidiFile::newOne()
{
	return new CSmodule_MidiFile();
}


void CSmodule_MidiFile::init()
{
	gname = copyString("midi");
	bname = copyString("midi file");
	name = copyString(bname);
	uname = copyString(bname);

	if (!parent) return;

	// --- inputs outputs ----

	_gate = &(createInput("g","gate")->value);
	_rst = &(createInput("rst","reset")->value);
	_rstpos = &(createInput("rstp","position")->value);
	_velcurve = &(createInput("vc","vel.curve",1.0)->value);
	_panic = &(createInput("!","panic!")->value);

	_bpm = &(createOutput("bpm","BPM")->value);
	_len = &(createOutput("len","length")->value);
	_tick = &(createOutput("tick","tick")->value);

	char n[100], n1[100];

	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
	for (int j=0;j<CSMOD_MIDIMOD_MAX_POLY;j++)
	{
		sprintf(n, "n%d_%d",i,j);
		if (j==0)
			sprintf(n1, "track %d note", i);
		else
			sprintf(n1, "note (%d)", j+1);
		__outnote[i][j] = createOutput(n,n1);
		_outnote[i][j] = &(__outnote[i][j]->value);

		sprintf(n, "g%d_%d",i,j);
		if (j==0)
			sprintf(n1, "gate");
		else
			sprintf(n1, "gate (%d)", j+1);
		__outgate[i][j] = createOutput(n,n1);
		_outgate[i][j] = &(__outgate[i][j]->value);
		free(n);
		free(n1);
	}

	oldNrOut = 0;
	setNrOut(CSMOD_MIDIMOD_MAX_CH);

	maxVoices = CSMOD_MIDIMOD_MAX_POLY;
	polyTick = 24;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("maxpoly", "max. polyphony", maxVoices, 1,CSMOD_MIDIMOD_MAX_POLY);
	createPropertyInt("polytick", "polyphony tick delta", polyTick);
	createPropertyInt("shift", "shift simultan. notes", 0, 0,1);
	createPropertyString("fname", "midi file name", "d:\\samples\\mid\\bach\\ausliebe.mid");

	// -- internals --

	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
	{
		track[i].nrNoteAlloc = 128;
		track[i].nrNote = 0;
		track[i].note = (CSmidiNote*) calloc(track[i].nrNoteAlloc, sizeof(CSmidiNote));
		track[i].len = 0;
		track[i].maxPoly = 1;
		for (int j=0;j<CSMOD_MIDIMOD_MAX_POLY;j++)
			lencount[i][j] = -1;
	}

	time = 0;
	len = 0;
	lgate = 0.0;
	lrst = 0.0;
	lpanic = 0.0;

}


void CSmodule_MidiFile::setNrOut(int newNrOut)
{
	if (newNrOut==oldNrOut) return;
	nrOut = newNrOut;
	oldNrOut = nrOut;

	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
	for (int j=0;j<CSMOD_MIDIMOD_MAX_POLY;j++)
	{
		setConnectorActive(__outnote[i][j], (i<nrOut));
		setConnectorActive(__outgate[i][j], (i<nrOut));
	}

	arrangeConnectors();
}

void CSmodule_MidiFile::selectOutputs()
{
	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
	for (int j=0;j<CSMOD_MIDIMOD_MAX_POLY;j++)
	{
		bool b = (track[i].len>0) & (track[i].maxPoly>=j);
		setConnectorActive(__outnote[i][j], b);
		setConnectorActive(__outgate[i][j], b);
	}

	arrangeConnectors();
}

void CSmodule_MidiFile::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"maxpoly")==0)
		maxVoices = max(1, prop->ivalue);
	else
	if (strcmp(prop->name,"polytick")==0)
		polyTick = prop->ivalue;
	else
	if (strcmp(prop->name,"shift")==0)
		doShiftNotes = (prop->ivalue==1);
	else
	if (strcmp(prop->name,"fname")==0)
		loadFile(prop->svalue);

	else CSmodule::propertyCallback(prop);
}


void CSmodule_MidiFile::step()
{
	// reset song
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		time = *_rstpos;
		for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
		{
			track[i].index = findTimeIndex(i, time);
			for (int j=0;j<CSMOD_MIDIMOD_MAX_POLY;j++)
			{
				*_outgate[i][j] = 0.0;
				*_outnote[i][j] = 0.0;
			}
		}
	}
	lrst = *_rst;

	// forward play-pointer
	if ((lgate<=0.0)&&(*_gate>0.0)) time++;
	lgate = *_gate;

	// output current time
	*_tick = time;

	// kill gates on PANIC!
	if ((lpanic<=0.0)&&(*_panic>0.0))
	{
		for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
		for (int j=0;j<CSMOD_MIDIMOD_MAX_POLY;j++)
		{
			*_outgate[i][j] = 0.0;
			lencount[i][j] = -1;
		}
	}
	lpanic = *_panic;

	// send note-off?
	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
	for (int j=0;j<CSMOD_MIDIMOD_MAX_POLY;j++)
	{
		if (lencount[i][j]==0)
		{
			*_outgate[i][j] = 0.0;
			lencount[i][j] = -1;
		}
		else
		if (lencount[i][j]>0) lencount[i][j]--;
	}

	// play notes
	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
	if ((time<=track[i].len)
		&&(track[i].index<track[i].nrNote))
	{
		if (time >= track[i].note[track[i].index].time)
		{
			CSmidiNote *n = &track[i].note[track[i].index];
			int voice = n->voiceNr;
			*_outnote[i][voice] = n->note;
			csfloat v = (csfloat)n->vel / 127;
			*_outgate[i][voice] = pow(v,*_velcurve);
			lencount[i][voice] = n->len;
			track[i].index ++;
		}
	}
}



// ----------------------------------------------

void CSmodule_MidiFile::clearTracks()
{
	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
	{
		track[i].nrNote = 0;
		track[i].len = 0;
		track[i].index = 0;
		track[i].maxPoly = 0;
	}
	len = 0;
}

void CSmodule_MidiFile::addNote(unsigned int time, unsigned char note, unsigned char vel, unsigned char ch)
{
    //printf("note on %d %d %d %d\n", time,note,vel,ch);

	// make space
	if (track[ch].nrNote >= track[ch].nrNoteAlloc)
	{
		track[ch].nrNoteAlloc += 128;
		track[ch].note = (CSmidiNote*) realloc(track[ch].note, track[ch].nrNoteAlloc*sizeof(CSmidiNote));
	}

	// current (new) note index
	int nrn = track[ch].nrNote;

	// when note-off
	if (vel==0)
	{
		if (nrn<1) return;
		// step back through previous notes
		for (int i=nrn-1;i>=0;i--)
		{
			if (note==track[ch].note[i].note)
			{
				// store delta as note length
				track[ch].note[i].len = max(1, (int)time - (int)track[ch].note[i].time);
				// keep length of track
				track[ch].len = max(track[ch].len, time+track[ch].note[i].len);
				return;
			}
		}
		return;
	}

	// assign given note event
	track[ch].note[nrn].time = time;
	track[ch].note[nrn].note = note;
	track[ch].note[nrn].vel = vel;
	track[ch].note[nrn].len = 1;
	track[ch].note[nrn].voiceNr = 0;

	// check for chords (simultaneous notes)
	if (track[ch].nrNote>1)
	{
		unsigned int delta = time - track[ch].note[nrn-1].time;
		if (delta<=polyTick)
			track[ch].note[nrn].voiceNr =
				(track[ch].note[nrn-1].voiceNr + 1) % maxVoices;
		// remember max polyphony
		track[ch].maxPoly = max(track[ch].maxPoly, (int)track[ch].note[nrn].voiceNr);
	}

	// set note-off for previous
	for (int i=nrn-1;i>=0;i--)
	if ((track[ch].note[i].voiceNr==track[ch].note[nrn].voiceNr) &&
		(track[ch].note[i].len >= time-track[ch].note[i].len) )
	{
		track[ch].note[i].len = time-track[ch].note[i].len-1;
		i = -1; // break
	}

	track[ch].nrNote++;

	// keep length of track
	track[ch].len = max(track[ch].len, time);

	// keep song length
	len = max(len, track[ch].len);
	*_len = len;
}

int CSmodule_MidiFile::findTimeIndex(int ch, unsigned int time)
{
	if (track[ch].nrNote<1) return 0;
	int i=0;
	for (;i<track[ch].nrNote;i++)
		if (time<=track[ch].note[i].time) return i;
	return i-1;
}


void CSmodule_MidiFile::shiftNotes()
{
	for (int j=0;j<CSMOD_MIDIMOD_MAX_CH;j++)
	if (track[j].nrNote>0)
	{
		for (int i=1;i<track[j].nrNote;i++)
		{
			if (track[j].note[i].time<=track[j].note[i-1].time)
			{
				track[j].note[i].time=track[j].note[i-1].time+2;
				track[j].note[i-1].len = 1;
			}
		}
		track[j].len = track[j].note[track[j].nrNote-1].time + track[j].note[track[j].nrNote-1].len;

	}
}





unsigned int CSmodule_MidiFile::readQuant(FILE *f)
{
	unsigned int r = 0;
	for (unsigned char b = 0x80; (b & 0x80)!=0;)
	{
		fread(&b, 1, 1, f);
		r = (r << 7) | (b & 0x7f);
	}
	return r;
}


void CSmodule_MidiFile::readTrack(FILE *f)
{
	unsigned int
		head=0,
		chunklen=0,
		len=0,
		time=0,
		tempo=0,
		delta=0;
	unsigned char
		status=0,b=0;
	unsigned short int
		w=0;
	bool
		endoftrack=false;

	// read header
	fread(&head, 4, 1, f);
	fread(&chunklen, 4, 1, f);
    // skip if unknown
    if (head!=0x6b72544D)
    {
        fseek(f, chunklen-4, SEEK_CUR);
        return;
    }

	// loop through following midi commands
    for (;(endoftrack==false)&&(!feof(f));)
    {
		delta = readQuant(f);
		time += delta;

        fread(&status, 1,1,f);

        // skip sysex
        if (status==0xf0)
        {
          chunklen = readQuant(f);
          fseek(f, chunklen, SEEK_CUR);
        }

        // take a look at meta
        if (status==0xff)
        {
        	fread(&b, 1,1,f);
			if (b==0x2f) endoftrack = true;
			// read tempo
			if ((b==0x51) && (tempo==0))
			{
				len = readQuant(f);
				for (int i = len-1;i>=0;i--)
				{
					fread(&b, 1,1,f);
					tempo = tempo | (b << (i*8));
				}
				if (tempo>0)
					*_bpm = 60000000/tempo;
				else
					*_bpm = 0.0;
			}
			else
			// skip all other
			{
				len = readQuant(f);
				fseek(f, len, SEEK_CUR);
			}
        }

        // note on
        if ((status & 0xf0) == 0x90)
        {
        	fread(&w, 2,1,f);
        	addNote(time, w & 0xff, (w>>8) & 0xff, status & 0x0f);
        }

        // note off
        if ((status & 0xf0) == 0x80)
        {
        	fread(&w, 2,1,f);
        	addNote(time, w & 0xff, 0, status & 0x0f);
        }

        // keypressure
        if ((status & 0xf0) == 0xa0) fread(&w, 2,1,f);
        // controllerchange
        if ((status & 0xf0) == 0xb0) fread(&w, 2,1,f);
        // program change
        if ((status & 0xf0) == 0xc0) fread(&b, 1,1,f);
        // channel pressure
        if ((status & 0xf0) == 0xd0) fread(&b, 1,1,f);
        // pitchbend
        if ((status & 0xf0) == 0xe0) fread(&w, 2,1,f);
        // songposition
        if (status == 0xf2) fread(&b, 1,1,f);
        // songselect !!! TODO ???
        if (status == 0xf2) fread(&b, 1,1,f);
        // unoff. bus select
        if (status == 0xf2) fread(&b, 1,1,f);

    }

}


bool CSmodule_MidiFile::loadFile(const char *filename)
{

	FILE *f = fopen(filename, "rb");
	if (!f)
	{
		printf("CAN'T READ %s\n", filename);
		return false;
	}

	unsigned int
		head,
		chunklen;
	unsigned short int
		typ,
		tracks,
		ticks;


	fread(&head, 4,1,f);
	if (head!=0x6468544D)
	{
		printf("%s is no midi file\n", filename);
		fclose(f);
		return false;
	}

	clearTracks();

	fread(&chunklen, 4,1,f);
	fread(&typ, 2,1,f);
	fread(&tracks, 2,1,f);
	fread(&ticks, 2,1,f);

	*_bpm = 0.0;
	for (int t=0;(t<tracks)&&(!feof(f));t++)
	{
		readTrack(f);
	}

	fclose(f);
	printf("LOADED %s\n", filename);

	if (doShiftNotes) shiftNotes();

	for (int i=0;i<CSMOD_MIDIMOD_MAX_CH;i++)
		if (track[i].len)
			printf("track %d: %d notes / length %d\n", i+1, track[i].nrNote, track[i].len);

	selectOutputs();
	return true;
}















char *CSmodule_MidiMerge::docString()
{
	const char *r = "\
the module merges several midi tracks to one single track.\n\n\
internally it has a Last-In-First-Out buffer, so the last gate (and note) that was inserted on one of \
the channels is first output. the module works it's stack until nothings left to send.\
";
	return const_cast<char*>(r);
}

CSmodule_MidiMerge* CSmodule_MidiMerge::newOne()
{
	return new CSmodule_MidiMerge();
}


void CSmodule_MidiMerge::init()
{
	gname = copyString("midi");
	bname = copyString("midi merger");
	name = copyString(bname);
	uname = copyString(bname);

	// --- inputs outputs ----

	_rst = &(createInput("rst","reset")->value);
	_outgate = &(createOutput("outg","GATE")->value);
	_outnote = &(createOutput("outn","note")->value);

	char n[128], n1[128];
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		sprintf(n,"g%d",i);
		sprintf(n1,"#%d GATE",i+1);
		__gate[i] = createInput(n,n1);
		_gate[i] = &(__gate[i]->value);

		sprintf(n,"n%d",i);
		sprintf(n1,"#%d note",i+1);
		__note[i] = createInput(n,n1);
		_note[i] = &(__note[i]->value);
	}

	setNrIn(2);
	for (int i=0;i<CSMOD_MAX_CHAN;i++)
		lgate[i] = 0.0;
	bufpos = 0;
	toSend = 0;
	waitFrame = 0;

	// ---- properties ----

	createNameProperty();
	createPropertyInt("nrin", "nr of channels", nrIn, 2,CSMOD_MAX_CHAN);
}

void CSmodule_MidiMerge::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"nrin")==0)
		setNrIn(prop->ivalue);
	else
	CSmodule::propertyCallback(prop);
}

void CSmodule_MidiMerge::setNrIn(int newNrIn)
{
	nrIn = newNrIn;

	for (int i=0;i<CSMOD_MAX_CHAN;i++)
	{
		setConnectorActive(__gate[i], (i<nrIn));
		setConnectorActive(__note[i], (i<nrIn));
	}

	arrangeConnectors();
}


void CSmodule_MidiMerge::step()
{
	// go through all channel inputs
	for (int i=0;i<nrIn;i++)
	{
		// check for gate
		if ((lgate[i]<=0.0) && (*_gate[i]>0.0))
		{
			// store in buffer
			buf[bufpos++] = *_gate[i];
			buf[bufpos++] = *_note[i];
			bufpos = bufpos & (CSMOD_MIDIMERGE_MAX_BUF_SIZE-1);
			toSend++;
		}
		lgate[i] = *_gate[i];
	}

	// reset
	if ((lrst<=0.0)&&(*_rst>0.0))
	{
		for (int i=0;i<nrIn;i++)
			lgate[i] = 0.0;
		*_outgate = 0.0;
		*_outnote = 0.0;
		bufpos = 0;
		toSend = 0;
		waitFrame = 0;

		// reset wins!
		return;
	}

	// skip every next frame when sending
	if (waitFrame) {
		// reset gate-out to make ready for next gate
		*_outgate = 0.0;
		waitFrame = 0;
		return;
		}

	// send off the buffer
	if (toSend)
	{
		bufpos = (bufpos-2) & (CSMOD_MIDIMERGE_MAX_BUF_SIZE-1);
		*_outgate = buf[bufpos];
		*_outnote = buf[bufpos+1];
		toSend--;
		waitFrame = 1;
	}

}

#endif // #ifdef CSMOD_USE_MIDI
