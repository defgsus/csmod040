/**
 *
 * RIFF WAVE PCM FORMAT BUFFERED DISK READ TOOL
 *
 * currently only supports 16 bit !!!
 *
 */

#ifndef CSMOD_WAVEREADER_H_INCLUDED
#define CSMOD_WAVEREADER_H_INCLUDED

#include "c++/bits/stl_algobase.h" // only there for min() max()
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** a simple buffered input RIFF WAVE reader */
class CSwaveReader
{
	public:

	FILE
		/** the open file handle */
		*file;

		/** memory buffer */
	char *buf;

		/* typical wave settings */
	short int
		nrchan,
		blockalign,
		bitps;
	unsigned int
		/** sample rate */
		rate,
		byteps,

		/** size in bytes of data on disk */
		datasize,
		/** length of samples on disk */
		datalength,
		/** pointer in file where data starts */
		p_datapos,
		/** pointer in file where data was last read */
		p_bufpos,
		bufsize,
		buflength,
		maxbufsize,
		maxbuflength,
		/** position in samples of buffer in whole file,
			buffer = file(pos -> pos+buflength), or -1 */
		pos,

		/** nr of samples to look 'ahead' for buffer reading */
		look_ahead;

	int
		/** -1 or sample which should be read */
		requestRead;

	/** constructor */
	CSwaveReader();
	/** destructor */
	~CSwaveReader();

	void debugInfo();

	// ------------- buffer ---------------------------------------

	/** release allocated buffer (or do nothing) */
	void releaseBuffer();

	/** alloc an internal buffer space len*sizeof(short int) */
	void createBuffer(unsigned int len);

	/** set within bounds the requested position */
	void setRequest(int sam);

	/** checks for a buffer read request and does so */
	void updateBuffer();

	// ------------ files -----------------------------------------

	/** close a file */
	void close();

	/** open a wave file for reading,
		return true on success */
	bool open(const char *filename);

	/** fill the whole buffer with data from disk */
	void readBuffer(unsigned int bytepos);

	// ------------ samples --------------------------------------

	/** return a number of samples (position @param sample from beginning of file).
		if buffer not ready, request is memorized, false returned and buffer will
		be reread on next @see updateBuffer() */
	bool getSamples(unsigned int sample, short int nrChan, float *samples);

	bool getSamplesLinear(float sample, short int nrChan, float *samples);

	/*void getSampleSec(float sec)
	{
		return getSample(sec*rate);
	}
	*/

};



#endif // CSMOD_WAVEREADER_H_INCLUDED
