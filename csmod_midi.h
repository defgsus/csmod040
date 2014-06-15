/**
 *
 * CS MODULAR EDITOR
 *
 * MIDI handling
 *
 *
 * @version 2011/2/20 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMOD_MIDI_H_INCLUDED
#define CSMOD_MIDI_H_INCLUDED

#ifdef CSMOD_USE_MIDI

#ifdef WIN32
#	include <windows.h>
#	include <mmsystem.h>
#else
#   include <inttypes.h>
#   include "system.h"
#   define CALLBACK
#   define HMIDIIN int
#   define HMIDIOUT int
#endif

/** structure to hold device list with names */
struct CSmidiDevices
{
	int
		/** nr of devices */
		nr;

	char
		/** list of names */
		**name;
};


/** create new device list */
CSmidiDevices *CSmidi_newDeviceList();

/** delete and dealloc a device list */
void CSmidi_deleteDeviceList(CSmidiDevices *d);

/** get and handle the structure and return success with 0 */
int CSmidi_getDevices(CSmidiDevices *d, bool getInputs = true);

/** returns handle, or null */
int CSmidi_openInput(int deviceId, DWORD callback, DWORD data);

void CSmidi_closeInput(int handle);

#endif // CSMOD_USE_MIDI

#endif // CSMOD_MIDI_H_INCLUDED
