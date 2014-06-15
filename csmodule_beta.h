/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Beta Term
 *
 * PURPOSE:
 * bend a plane
 *
 * @version 2010/10/25 v0.1
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_BETA_H_INCLUDED
#define CSMODULE_BETA_H_INCLUDED

#include "csmodule.h"

class CSmodule_Beta: public CSmodule
{
	csfloat
		*_rad,
		*_X,
		*_Y,
		*_Z,

		*_out;

	CSmoduleConnector
		*__out,*__X,*__Y,*__Z;

	public:
	char *docString();
	CSmodule_Beta *newOne();
    ~CSmodule_Beta() { }
	void init();
	void setDim(int nrDim);
	void propertyCallback(CSmodProperty *prop);

	void step();

};


#endif // CSMODULE_BETA_H_INCLUDED
