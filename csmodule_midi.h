/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: midi in/out
 *
 * PURPOSE:
 * note in/out
 *
 * @version 2010/12/05 v0.1 note in
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_MIDI_H_INCLUDED
#define CSMODULE_MIDI_H_INCLUDED

#include "csmodule.h"
#include "csmodcontainer.h"

#define CSMOD_MIDIMOD_MAX_POLY 8

#define CSMOD_MIDIMOD_MAX_CH 16

class CSmodule_NoteIn: public CSmodule
{
	csfloat
		*_gate,
		*_note;

	int lastNote;

	public:
	char *docString();
	CSmodule_NoteIn *newOne();
	void init();

	void step();

};


class CSmodule_NoteInPoly: public CSmodule
{
	csfloat
		*_trans,
		*_note[CSMOD_MAX_CHAN],
		*_gate[CSMOD_MAX_CHAN],

		*_outNr;

	CSmoduleConnector
		*__note[CSMOD_MAX_CHAN],
		*__gate[CSMOD_MAX_CHAN];

	csfloat
		/** last occured note's velocity */
		noteVel[CSMOD_MAX_NOTE];
	int
		/** buffer of polyphonic note-on numbers */
		noteNr[CSMOD_MAX_NOTE],
		/** nr of polyphonic voices of midi notes */
		nrMidiVoices,
		/** nr of active polyphonic voices of midi notes */
		nrMidiVoicesActive;

	bool
		changed,
		changedScr;

	public:
	char *docString();
	CSmodule_NoteInPoly *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrOut(int newNrOut);

	void midiPanic();
	/** container calls this to acknowledge midi notes */
	void noteOn(unsigned char noteNr, unsigned char velocity);

	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);
};



class CSmodule_ControllerIn: public CSmodule
{
	csfloat
		*_nr[CSMOD_MAX_CONTROLLER],
		*_out[CSMOD_MAX_CONTROLLER],

		oldNr[CSMOD_MAX_CONTROLLER];

	CSmoduleConnector
		*__nr[CSMOD_MAX_CONTROLLER],
		*__out[CSMOD_MAX_CONTROLLER];

	int
		nrIn,
		doNrpn;
	bool
		changed,
		changedScr;

	public:
	char *docString();
	CSmodule_ControllerIn *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);
	void setNrIn(int newNrIn);

	void controlChange(int controller, int value);
	void control14Change(int controller, int value);
	void step();

	void draw(int offx=0, int offy=0, float zoom=1.0);
};



// ------------------ midi file player ----------------------------


struct CSmidiNote
{
	unsigned int
		/** time of event in midi ticks */
		time,
		/** length of note in ticks */
		len;
	unsigned char
		/** midi note */
		note,
		/** midi velocity */
		vel,
		/** polyphonic voice nr */
		voiceNr;
};

struct CSmidiTrack
{
	CSmidiNote
		/** all notes in track */
		*note;
	int
		/** nr of notes in track */
		nrNote,
		/** nr of allocated notes in track */
		nrNoteAlloc,
		/** last played note index */
		index,
		/** maximum simultaneous notes in track */
		maxPoly;

	unsigned int
		/** length of track in ticks */
		len;
};


class CSmodule_MidiFile: public CSmodule
{
	csfloat
		*_gate,
		*_rst,
		*_rstpos,
		*_velcurve,
		*_panic,

		*_bpm,
		*_tick,
		*_len,

		lgate,
		lrst,
		lpanic,

		*_outnote[CSMOD_MIDIMOD_MAX_CH][CSMOD_MIDIMOD_MAX_POLY],
		*_outgate[CSMOD_MIDIMOD_MAX_CH][CSMOD_MIDIMOD_MAX_POLY];

	CSmoduleConnector
		*__outnote[CSMOD_MIDIMOD_MAX_CH][CSMOD_MIDIMOD_MAX_POLY],
		*__outgate[CSMOD_MIDIMOD_MAX_CH][CSMOD_MIDIMOD_MAX_POLY];

	int nrOut, oldNrOut,
		/** counter till note-off */
		lencount[CSMOD_MIDIMOD_MAX_CH][CSMOD_MIDIMOD_MAX_POLY],
;

	// ---

	CSmidiTrack
		track[CSMOD_MIDIMOD_MAX_CH];

	unsigned int
		maxVoices,
		polyTick,
		len,
		time;

	bool
		doShiftNotes;

	// ---

	public:
	char *docString();
	~CSmodule_MidiFile();
	CSmodule_MidiFile *newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrOut(int newNrIn);

	void selectOutputs();

	void step();

	// ----- note handling

	void clearTracks();

	/** if vel==0 do a noteoff */
	void addNote(unsigned int time, unsigned char note, unsigned char vel, unsigned char ch);

	/** find index in track[ch] that is at time */
	int findTimeIndex(int ch, unsigned int time);

	/** shift note positions so that there are no simultaneous notes */
	void shiftNotes();

	// --- file read ----

	/** read variable length quantity */
	unsigned int readQuant(FILE *f);

	void readTrack(FILE *f);

	bool loadFile(const char *filename);

};





#define CSMOD_MIDIMERGE_MAX_BUF_SHIFT 13
#define CSMOD_MIDIMERGE_MAX_BUF (1<<CSMOD_MIDIMERGE_MAX_BUF_SHIFT)
#define CSMOD_MIDIMERGE_MAX_BUF_SIZE (CSMOD_MIDIMERGE_MAX_BUF<<1)

class CSmodule_MidiMerge: public CSmodule
{
	csfloat
		*_rst,lrst,
		*_gate[CSMOD_MAX_CHAN],
		*_note[CSMOD_MAX_CHAN],
		*_outgate,
		*_outnote,

		lgate[CSMOD_MAX_CHAN];

	CSmoduleConnector
		*__gate[CSMOD_MAX_CHAN],
		*__note[CSMOD_MAX_CHAN];

	csfloat
		buf[CSMOD_MIDIMERGE_MAX_BUF_SIZE];

	int
		nrIn,
		bufpos,
		toSend,
		waitFrame;

	public:
	char *docString();
	CSmodule_MidiMerge* newOne();
	void init();
	void propertyCallback(CSmodProperty *prop);

	void setNrIn(int newNrIn);

	void step();

};



#endif // CSMODULE_MIDI_H_INCLUDED
