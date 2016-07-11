#include "csmod_wavereader.h"


CSwaveReader::CSwaveReader()
{
	file = 0;
	buf = 0;
	pos = -1;
	look_ahead = 44100*5;
	createBuffer(44100*60*2);
}

CSwaveReader::~CSwaveReader()
{
	close();
}




void CSwaveReader::debugInfo()
{
	printf("file 0x%p, buf 0x%p\n", file, buf);
	printf("rate %d, nrchan %d, bitps %d, byteps %d, blockalign %d\n", rate, nrchan, bitps, byteps, blockalign);
	printf("datasize %d, datalength %d, p_datapos %d\n",datasize,datalength,p_datapos);
	printf("bufsize %d, buflength %d, p_bufpos %d\n", bufsize, buflength, p_bufpos);
	printf("maxbufsize %d, requestRead %d\n", maxbufsize, requestRead);
	printf("pos %d\n", pos);
}


	// ------------- buffer ---------------------------------------

void CSwaveReader::releaseBuffer()
{
	if (buf) free(buf);
	buflength = 0;
	bufsize = 0;
	p_bufpos = 0;
	requestRead = -1;
}

void CSwaveReader::createBuffer(unsigned int len)
{
	if (buf) free(buf);
	maxbufsize = len;
	buf = (char*) calloc(maxbufsize, sizeof(short int));
	buflength = 0;
	bufsize = 0;
	maxbuflength = 0;
	p_bufpos = 0;
	requestRead = -1;
}

void CSwaveReader::setRequest(int sam)
{
	requestRead = std::max(0,sam);
	requestRead = std::min((unsigned int)requestRead, datalength-maxbuflength);
}
void CSwaveReader::updateBuffer()
{
	if ((!file)||(requestRead<0)) return;
	readBuffer(requestRead*nrchan*bitps/8);
	requestRead = -1;
}

	// ------------ files -----------------------------------------



void CSwaveReader::close()
{
	if (file) fclose(file);
	file = 0;
	pos = -1;
	p_datapos = 0;
	datasize = 0;
	datalength = 0;
	nrchan = 0;
	blockalign = 0;
	byteps = 0;
	bitps = 0;
	rate = 0;
	p_bufpos = 0;
}



bool CSwaveReader::open(const char *filename)
{
	close();

	file = fopen(filename, "rb");
	if (!file) { printf("waveReader: CAN NOT OPEN: %s\n", filename); return false; }

	char head[5]; memset(&head, 0, 5);
	unsigned int e = 0, chunksize = 0;

	// start reading
	e = fread(&head, 1, 4, file);
	printf("%s\n",head);
	if ((e!=4)||(strcmp(head, "RIFF")))
		{ printf("waveReader: SEEMS TO BE NO WAVE (no RIFF): %s\n", filename); close(); return false; }

	fread(&chunksize, 4, 1, file);

	e = fread(&head, 1, 4, file);
	if ((e!=4)||(strcmp(head, "WAVE")))
		{ printf("waveReader: SEEMS TO BE NO WAVE (no WAVE): %s\n", filename); close(); return false; }



	// ------ read chunk ------

	e = fread(&head, 1, 4, file);

	if ((e!=4)||(strcmp(head, "fmt ")))
		{ printf("waveReader: SEEMS TO BE NO WAVE (no fmt): %s\n", filename); close(); return false; }

	// chunksize
	unsigned int cs = 0;
	// compression code
	short int cc = 0;

	fread(&cs, 4, 1, file);
	fread(&cc, 2, 1, file);
	if (cc>1)
		{ printf("waveReader: CAN NOT READ COMPRESSED WAVE: %s\n", filename); close(); return false; }

	fread(&nrchan, 2, 1, file);
	fread(&rate, 4, 1, file);
	fread(&byteps, 4, 1, file);
	fread(&blockalign, 2, 1, file);
	fread(&bitps, 2, 1, file);

	// skip any chunks that are not data
	e = fread(&head, 1, 4, file);
	while ((e==4)&&(strcmp(head, "data")))
	{
		fread(&datalength, 4, 1, file);
		fseek(file, datalength, SEEK_CUR);
		e = fread(&head, 1, 4, file);
	}
	if ((e!=4) || (strcmp(head, "data")))
		{ printf("waveReader: FOUND NO DATA CHUNK: %s\n", filename); close(); return false; }

	// size of wave data
	fread(&datasize, 4,1,file);

	// fileposition of wavedata
	p_datapos = ftell(file);

	if ((nrchan<1)||(bitps<16))
		{ printf("waveReader: BOGUS OR UNSUPPORTED FORMAT: %s\n", filename); close(); return false; }

	// true length in samples
	datalength = datasize / nrchan * 8 / bitps;
	maxbuflength = maxbufsize / nrchan * 8 / bitps;

	printf("waveReader: LOADED WAVE: %s\n",filename);
	printf("16bit / %dhz / %d chan / %g sec\n", rate, nrchan, (float)datalength / rate);

	return true;
}


void CSwaveReader::readBuffer(unsigned int bytepos)
{
	if ((!file)||(!buf)) return;

	if (bytepos>=datasize) { buflength = 0; return; }

	p_bufpos = p_datapos + bytepos;
	if (fseek(file, p_bufpos, SEEK_SET))
	{
		printf("waveReader: WAVE BUFFER SEEK ERROR (bufpos=%d)\n", p_bufpos);
		buflength = 0;
		return;
	}

	bufsize = datasize - bytepos;
	if (bufsize>maxbufsize) bufsize = maxbufsize;
	size_t e =
	fread(buf, 1, bufsize, file);
	buflength = e / nrchan * 8 / bitps;
	if (e == 0)
	{
        printf("waveReader: WAVE BUFFER READ ERROR "
               "(buflength=%d, datalength=%d, bytepos=%d, bufpos=%d, read=%"
               //"l"
               "d)\n",
            buflength, datalength, bytepos, p_bufpos, int(e));
	}
	// update position
	pos = bytepos / nrchan * 8 / bitps;

}









	// ------------ samples --------------------------------------

bool CSwaveReader::getSamples(unsigned int sample, short int nrChan, float *samples)
{
	if ((!file)||(!buf)) return false;

    //if (sample<0) return false;
	if (sample>=datalength) return false;

	if ((bufsize==0)||((int)pos==-1)) {
		setRequest(sample-maxbuflength/2); return false; }

	if ((sample<pos+look_ahead)&&(pos>0)) {
		setRequest(sample-maxbuflength/2); }

	if (sample>=pos+buflength-look_ahead) {
		setRequest(sample-maxbuflength/2); }

	unsigned int po = (sample-pos) * nrchan * bitps / 8;
	short int *si = (short int*)&buf[po];
	for (int i=0;i<nrChan;i++)
	{
		*samples = (float)*si / 0x8000;
		samples++;
		si++;
	}
	return true;
}

bool CSwaveReader::getSamplesLinear(float sample, short int nrChan, float *samples)
{
	if ((!file)||(!buf)) return false;

	if (sample<0) return false;
	if (sample>=datalength) return false;

	if ((bufsize==0)||((int)pos==-1)) {
		setRequest(sample-maxbuflength/2); return false; }

	if ((sample<pos+look_ahead)&&(pos>0)) {
		setRequest(sample-maxbuflength/2); }

	if (sample>=pos+buflength-look_ahead) {
		setRequest(sample-maxbuflength/2); }

	float fpo = (sample-pos);
	unsigned int po = (unsigned int)fpo;
	short int *si = (short int*)&buf[po*nrchan*bitps/8];
	float *sams = samples;
	for (int i=0;i<nrChan;i++)
	{
		*sams = (float)*si / 0x8000;
		sams++;
		si++;
	}

	// interpolate with next sample
	fpo -= po;
	po++; if (po>=maxbuflength-byteps) return false;

	si = (short int*)&buf[po*nrchan*bitps/8];
	sams = samples;
	for (int i=0;i<nrChan;i++)
	{
		*sams = *samples * (1.0-fpo) + fpo * (float)*si / 0x8000;
		sams++;
		si++;
	}

	return true;
}
