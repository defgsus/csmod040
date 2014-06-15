/**
 *
 * RIFF WAVE PCM FORMAT DISK WRITER TOOL
 *
 * currently only supports 16 bit !!!
 *
 */

#ifndef CSMOD_WAVEWRITER_H_INCLUDED
#define CSMOD_WAVEWRITER_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

/** a simple blocking output RIFF WAVE writer */
class CSwaveWriter
{
	public:

	FILE
		*file;

	int
		rate;
	short int
		depth,
		channels;

	const char
		*filename;

	// different points in file
	unsigned int
		p_filesize,
		p_datasize,
		filesize,
		datasize;

	/** constructor */
	CSwaveWriter();
	/** destructor */
	~CSwaveWriter();

	/** close a file */
	void close();

	/** create a wave file for writing,
		return true on success */
	bool open(const char *name, int rate = 44100, int depth = 16, int channels = 2);

	/** add @param len bytes of any kind of data */
	void addVoid(void *buf, unsigned int len);

	/** add @param samples samples of float data,
		the number should result from nrOfChannels * nrOfActualSamples,
		the number of channels should match with the wavewriter's settings,
		otherwise you can use the @param skip parameter to set the number of words
		that should be skipped in the input buffer. */
	void addFloat(float *buf, unsigned int samples, int skip=1);

};


#endif // CSMOD_WAVEWRITER_H_INCLUDED
