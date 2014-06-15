#include "csmodule_ca.h"

char *CSmodule_CA::docString()
{
	const char *r = "\
the module performs logic operations on the inputs, which are interpretated as bits.\n\n\
<in> is an input which in interpretated as false/off for <= 0.0 and true/on for > 0.0.\n\
<AND>, <OR> or <XOR> is the output result of the logic operation for all inputs (0 or 1).\n\
<!AND>, <!OR> a.s.o. is the inverted <output> for convenience.\
";
	return const_cast<char*>(r);
}

CSmodule_CA* CSmodule_CA::newOne()
{
	return new CSmodule_CA();
}


void CSmodule_CA::init()
{
	gname = copyString("CA");
	bname = copyString("cellular automata");
	name = copyString(bname);
	uname = copyString(bname);

	// --- look ----

	sx = 80;
	sy = headerHeight+5;
	sw = 128;
	sh = 128;
	minWidth = sx+sw+80;
	minHeight = sy+sh+10;

	// --- inputs outputs ----

	_writeGate = &(createInput("wg","gate (write)")->value);
	_in = &(createInput("in","input",1.0)->value);
	_inx = &(createInput("inx","input x")->value);
	_iny = &(createInput("iny","input y")->value);
	_resetMap = &(createInput("mg","gate (reset map)")->value);
	_stepGate = &(createInput("sg","gate (step)")->value);
	_outx = &(createInput("ox","output x")->value);
	_outy = &(createInput("oy","output y")->value);

	_b[0] = &(createInput("b0","b 0")->value);
	_b[1] = &(createInput("b1","b 1")->value);
	_b[2] = &(createInput("b2","b 2")->value);
	_b[3] = &(createInput("b3","b 3",1.0)->value);
	_b[4] = &(createInput("b4","b 4")->value);
	_b[5] = &(createInput("b5","b 5")->value);
	_b[6] = &(createInput("b6","b 6")->value);
	_b[7] = &(createInput("b7","b 7")->value);
	_b[8] = &(createInput("b8","b 8")->value);

	_s[0] = &(createInput("s0","s 0")->value);
	_s[1] = &(createInput("s1","s 1")->value);
	_s[2] = &(createInput("s2","s 2",1.0)->value);
	_s[3] = &(createInput("s3","s 3",1.0)->value);
	_s[4] = &(createInput("s4","s 4")->value);
	_s[5] = &(createInput("s5","s 5")->value);
	_s[6] = &(createInput("s6","s 6")->value);
	_s[7] = &(createInput("s7","s 7")->value);
	_s[8] = &(createInput("s8","s 8")->value);

	_out = &(createOutput("out","output")->value);
	_outn = &(createOutput("outn","neighbours")->value);
	_wi = &(createOutput("wi","width")->value);
	_he = &(createOutput("he","height")->value);

	// ---- properties ----

	wi = 8;
	he = 8;

	createNameProperty();
	createPropertyInt("width", "map width", wi, 2,CSMOD_MAX_CAWIDTH);
	createPropertyInt("height", "map height", he, 2,CSMOD_MAX_CAWIDTH);

	lwriteGate = lresetMap = lstepGate = 0.0;

	fillMap(0.0);
}


void CSmodule_CA::propertyCallback(CSmodProperty *prop)
{
	if (strcmp(prop->name,"width")==0)
		wi = prop->ivalue;
	else
	if (strcmp(prop->name,"height")==0)
		he = prop->ivalue;
	else
	CSmodule::propertyCallback(prop);
}



void CSmodule_CA::step()
{
	// reset the map
	if ((lresetMap<=0.0)&&(*_resetMap>0.0))
		fillMap(*_in);
	lresetMap = *_resetMap;

	// write onto map
	if ((lwriteGate<=0.0)&&(*_writeGate>0.0))
		map[ (max(0,min(he-1, (int)*_iny )) << CSMOD_CA_SHIFT ) +
			 max(0,min(wi-1, (int)*_inx )) ] = *_in;
	lwriteGate = *_writeGate;

	// step map
	if ((lstepGate<=0.0)&&(*_stepGate>0.0))
	{
		// fill buffer[] with number of neighbours
		getNeighbours();

		for (int j=0;j<he;j++)
		{
			int j1 = j<<CSMOD_CA_SHIFT;
			for (int i=0;i<wi;i++)
			{
				csfloat v = map[j1+i];
				int	n = max(0,min(8, (int)(0.5+buffer[j1+i]) ));
				if (v<0.5)
					map[j1+i] = *_b[n];
				else
					map[j1+i] = *_s[n];
			}
		}
	}
	lstepGate = *_stepGate;

	*_wi = wi;
	*_he = he;
	int k = (max(0,min(he-1, (int)*_outy )) << CSMOD_CA_SHIFT ) +
			 max(0,min(wi-1, (int)*_outx ));
	*_out = map[ k ];
	*_outn = buffer[ k ];
}


void CSmodule_CA::fillMap(csfloat value)
{
	for (int i=0;i<CSMOD_MAX_CASIZE;i++)
	{
		map[i] = value;
		buffer[i] = 0.0;
	}
}

void CSmodule_CA::getNeighbours()
{
	memset(buffer, 0, CSMOD_MAX_CASIZE*sizeof(csfloat));

	csfloat v;
	int
		j,j0,j1,j2,
		i,i0,i2;
	for (j=0;j<he;j++)
	{
		j1 = (j<<CSMOD_CA_SHIFT);
		j0 = (j>0)? (j1 - CSMOD_MAX_CAWIDTH) : ((he-1) << CSMOD_CA_SHIFT);
		j2 = (j<he-1)? (j1 + CSMOD_MAX_CAWIDTH) : 0;

		for (i=0;i<wi;i++)
		{
			i0 = (i>0)? (i - 1) : (wi-1);
			i2 = (i<wi-1)? (i+1) : (0);

			v = map[j1+i];
			buffer[j0+i0] += v;
			buffer[j0+i] += v;
			buffer[j0+i2] += v;

			buffer[j1+i0] += v;
			buffer[j1+i2] += v;

			buffer[j2+i0] += v;
			buffer[j2+i] += v;
			buffer[j2+i2] += v;

		}
	}

	/*

	int i,j,j0,j1,j2,j3,i1;
	csfloat v;

	// middle square
	for (j=1;j<he-1;j++)
	{
		j3 = (j<<CSMOD_CA_SHIFT);
		j2 = j3-CSMOD_CA_SHIFT;
		for (i=1;i<wi-1;i++)
		{
			v = map[j3+i];
			j1 = j2;
			buffer[j1+i-1] += v;
			buffer[j1+i] += v;
			buffer[j1+i+1] += v;
			j1 += CSMOD_MAX_CAWIDTH;
			buffer[j1+i-1] += v;
			buffer[j1+i+1] += v;
			j1 += CSMOD_MAX_CAWIDTH;
			buffer[j1+i-1] += v;
			buffer[j1+i] += v;
			buffer[j1+i+1] += v;
		}
	}

	// boarders
	j1 = (he-1)<<CSMOD_CA_SHIFT;
	for (i=1;i<wi-1;i++)
	{
		// top line
		csfloat v = map[i];
		buffer[j1+i-1] += v;
		buffer[j1+i] += v;
		buffer[j1+i+1] += v;
		buffer[i-1] += v;
		buffer[i+1] += v;
		j2 = CSMOD_MAX_CAWIDTH;
		buffer[j2+i-1] += v;
		buffer[j2+i] += v;
		buffer[j2+i+1] += v;

		// bottom line
		v = map[j1+i];
		j2 = j1-CSMOD_MAX_CAWIDTH;
		buffer[j2+i-1] += v;
		buffer[j2+i] += v;
		buffer[j2+i+1] += v;
		buffer[i-1] += v;
		buffer[i] += v;
		buffer[i+1] += v;
		buffer[j1+i-1] += v;
		buffer[j1+i+1] += v;
	}

	// left and right line
	i1=wi-1;
	for (j=1;j<he-1;j++)
	{
		j1 = j<<CSMOD_CA_SHIFT;
		j0 = j-CSMOD_MAX_CAWIDTH;
		j2 = j+CSMOD_MAX_CAWIDTH;
		csfloat v = map[j1];
		buffer[j0+i1] += v;
		buffer[j0] += v;
		buffer[j0+1] += v;
		buffer[j1+i1] += v;
		buffer[j1+1] += v;
		buffer[j2+i1] += v;
		buffer[j2] += v;
		buffer[j2+1] += v;

		v = map[j1+i1];
		buffer[j0+i1-1] += v;
		buffer[j0+i1] += v;
		buffer[j0] += v;
		buffer[j1+i1-1] += v;
		buffer[j1] += v;
		buffer[j2+i1-1] += v;
		buffer[j2+i1] += v;
		buffer[j2] += v;
	}

	// corners
	j1 = (he-1)<<CSMOD_CA_SHIFT;
	v = map[0];
	buffer[j1+i1] += v;
	buffer[j1] += v;
	buffer[j1+1] += v;
	buffer[i1] += v;
	buffer[1] += v;
	buffer[CSMOD_MAX_CAWIDTH+i1] += v;
	buffer[CSMOD_MAX_CAWIDTH] += v;
	buffer[CSMOD_MAX_CAWIDTH+1] += v;

	v = map[i1];
	buffer[j1+i1-1] += v;
	buffer[j1+i1] += v;
	buffer[j1] += v;
	buffer[i1-1] += v;
	buffer[0] += v;
	buffer[CSMOD_MAX_CAWIDTH+i1-1] += v;
	buffer[CSMOD_MAX_CAWIDTH+i1] += v;
	buffer[CSMOD_MAX_CAWIDTH] += v;

	v = map[j1];
	j2 = j1-CSMOD_MAX_CAWIDTH;
	buffer[j2+i1] += v;
	buffer[j2] += v;
	buffer[j2+1] += v;
	buffer[j1+i1] += v;
	buffer[j1+1] += v;
	buffer[i1] += v;
	buffer[0] += v;
	buffer[1] += v;

	v = map[j1+i1];
	buffer[j2+i1-1] += v;
	buffer[j2+i1] += v;
	buffer[j2] += v;
	buffer[j1+i1-1] += v;
	buffer[j1] += v;
	buffer[i1-1] += v;
	buffer[i1] += v;
	buffer[0] += v;
	*/
}







void CSmodule_CA::draw(int offx, int offy, float zoom)
{
	CSmodule::draw(offx,offy,zoom);

	float
		scx = (float)sw / wi,
		scy = (float)sh / he;
	int
		rx = max(1,(int)(scx*zoom)),
		ry = max(1,(int)(scy*zoom));

	for (int j=0;j<he;j++)
	{
		int j1 = j<<CSMOD_CA_SHIFT;
		for (int i=0;i<wi;i++)
		{
			uchar c = 255*map[j1+i];
			fl_color((c<<8)|(c<<16)|(c<<24));
			fl_rectf(
				offx + zoom*( x + sx + i * scx ),
				offy + zoom*( y + sy + j * scy ),
				rx,ry);
		}
	}
}
