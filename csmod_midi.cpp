#include "csmod_midi.h"
#include <stdio.h>
#include <string>

#ifdef CSMOD_USE_MIDI

CSmidiDevices *CSmidi_newDeviceList()
{
	CSmidiDevices *d = new CSmidiDevices;
	d->nr = 0;
	d->name = 0;
	return d;
}

void CSmidi_deleteDeviceList(CSmidiDevices *d)
{
	if (d->name)
	{
		for (int i=0;i<d->nr;i++) free(d->name[i]);
		free(d->name);
	}
}

int CSmidi_getDevices(CSmidiDevices *d, bool getInputs)
{
	CSmidi_deleteDeviceList(d);

#ifdef WIN32

	if (getInputs)
	{
        d->nr = midiInGetNumDevs();
        if (d->nr==0) return 1;

        d->name = (char**) calloc(d->nr,sizeof(char*));

		MIDIINCAPS c;
		memset(&c, 0, sizeof(MIDIINCAPS));

		for (int i=0;i<d->nr;i++)
		{
			midiInGetDevCaps(i, &c, sizeof(MIDIINCAPS));
            d->name[i] = (char*) calloc(MAXPNAMELEN, 1);
            for (int j=0; j<MAXPNAMELEN; ++j)
                d->name[i][j] = 0;
            for (int j=0; j<MAXPNAMELEN && c.szPname[j] != 0; ++j)
                d->name[i][j] = c.szPname[j];
        }
	}
    else
    {
        d->nr = midiOutGetNumDevs();
        if (d->nr==0) return 1;

        d->name = (char**) calloc(d->nr,sizeof(char*));

        MIDIOUTCAPS c;
        memset(&c, 0, sizeof(MIDIOUTCAPS));

        for (int i=0;i<d->nr;i++)
        {
            midiOutGetDevCaps(i, &c, sizeof(MIDIOUTCAPS));
            d->name[i] = (char*) calloc(MAXPNAMELEN, 1);
            for (int j=0; j<MAXPNAMELEN; ++j)
                d->name[i][j] = 0;
            for (int j=0; j<MAXPNAMELEN && c.szPname[j] != 0; ++j)
                d->name[i][j] = c.szPname[j];
        }
    }
#endif
	return 0;
}

int CSmidi_openInput(int deviceId, DWORD callback, DWORD data)
{
#ifdef WIN32

	HMIDIIN handle;
	int r =
		midiInOpen(
			&handle,
			deviceId,
			(DWORD)callback,
			data,
			CALLBACK_FUNCTION
			// | MIDI_IO_STATUS
			);
	if (r==MMSYSERR_NOERROR)
	{
		midiInStart(handle);
		return (int)handle;
	} else
		return 0;

#else
	return 0;
#endif


}


void CSmidi_closeInput(int handle)
{
#ifdef WIN32
	if (handle) midiInClose((HMIDIIN)handle);
#endif
}

#endif // #ifdef CSMOD_USE_MIDI
