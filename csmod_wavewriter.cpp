#include "csmod_wavewriter.h"


CSwaveWriter::CSwaveWriter()
{
	file = 0;
}

CSwaveWriter::~CSwaveWriter()
{
	close();
}

void CSwaveWriter::close()
{
	if (!file) return;

	filesize = ftell(file) - 8;

	fseek(file, p_filesize, SEEK_SET);
	fwrite(&filesize, 4, 1, file);

	fseek(file, p_datasize, SEEK_SET);
	fwrite(&datasize, 4, 1, file);

	fclose(file);
}



bool CSwaveWriter::open(const char *name, int rate, int depth, int channels)
{
	close();

	filename = name;
	this->rate = rate;
	this->depth = depth;
	this->channels = channels;

	short int blockalign = channels * depth / 8;
	unsigned int byteps = blockalign * rate;

	file = fopen(filename, "wb");
	if (!file) return false;

	int buf = 0;

	filesize = 0;
	datasize = 0;

	// header
	fwrite("RIFF", 1, 4, file);
	p_filesize = ftell(file);
	fwrite(&filesize, 4, 1, file); // temp dummy
	fwrite("WAVE", 1, 4, file);

	// format chunk

	fwrite("fmt ", 1, 4, file);
	// header length
	buf = 16; fwrite(&buf, 4,1,file);
	// format tag
	buf = 0x01; fwrite(&buf, 1,1,file);
	// compression tag
	buf = 0x00; fwrite(&buf, 1,1,file);

	fwrite(&channels, 2, 1, file);
	fwrite(&rate, 4, 1, file);
	fwrite(&byteps, 4, 1, file);
	fwrite(&blockalign, 2, 1, file);
	fwrite(&depth, 2, 1, file);


	// data chunk

	fwrite("data", 1,4, file);
	p_datasize = ftell(file);
	fwrite(&datasize, 4,1,file); // temp dummy

	// start data here...

	return true;
}



void CSwaveWriter::addVoid(void *buf, unsigned int len)
{
	if (!file) return;

	fwrite(buf, 1, len, file);
	datasize += len;
}


void CSwaveWriter::addFloat(float *buf, unsigned int samples, int skip)
{
	if (!file) return;

	float *b1 = buf;
	short int
		*b = (short int*) calloc(samples, 2),
		*b2 = b;

	for (unsigned int i=0;i<samples;i++)
	{
		*b2 = (short int)( *b1 * 0x7fff );
		b1+=skip;
		b2++;
	}

	addVoid(b, 2*samples);

	free(b);
}
