/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Container
 *
 * PURPOSE:
 * used to stash modules within modules !
 *
 * DEFINES:
 *   CSmodule_Container
 *   CSmodule_ContainerInput
 *   CSmodule_ContainerOutput
 *
 * @version 2010/09/26 v0.1
 * @version 2011/03/19 v0.2 container activity !!
 * @version 2011/03/23 v0.3 fileVersion 2 (connector names)
 * @author def.gsus-
 *
 */
#ifndef CSMODULE_CONTAINER_H_INCLUDED
#define CSMODULE_CONTAINER_H_INCLUDED


#include "csmodule.h"
#include "csmod.h"

// forward decl. (see below)
class CSmodule_ContainerInput;
class CSmodule_ContainerOutput;


/** the CSmodContainer holding module */
class CSmodule_Container: public CSmodule
{
	public:

	csfloat
		*_active, lactive;

	CSmodContainer
		*container;

	//char **conNames;

	bool active;

	char *docString();

    ~CSmodule_Container();

	CSmodule_Container* newOne();
	/** override copyFrom method to copy all the container contents */
	void copyFrom(CSmodule *mod);
	void init();
	void onAddModule();

	void storeAddBeforeValue(FILE *buf);
	void restoreAddBeforeValue(FILE *buf);
	void storeAdd(FILE *buf);
	void restoreAdd(FILE *buf);

	void arrangeConnectors();
	int nrContainerInputs();

	/** excange input with previous one (dir<0) or next one (dir>0).
		return new index */
	int swapContainerInput(int index, int dir);

	void step();
	void stepLow();

	void setActive(bool act);

	/** create a new connector for this module.
		(create a module input AND a CSmodule_ContainerInput in the container and link them) */
	CSmodule *newContainerCon(
		int dir, const char *Name=0, const char *Uname=0);

};


class CSmodule_ContainerInput: public CSmodule
{
	public:

	csfloat
		*_out;

	CSmodule_Container
		/** the container module for which this input is created */
		*containerModule;

	CSmoduleConnector
		/** the particular connector with which this input is linked */
		*containerConnector;

	CSmodule_ContainerInput();
	/** destructor takes care, that the input of the container module
		will be removed as well */
	~CSmodule_ContainerInput();

	CSmodule_ContainerInput* newOne();

	void init();
	/** change name of input of the container module as well */
	void propertyCallback(CSmodProperty *prop);
	/** pass input value from container module through */
	void step();

};



class CSmodule_ContainerOutput: public CSmodule
{
	public:

	csfloat
		*_in;

	CSmodule_Container
		/** the container module for which this output is created */
		*containerModule;

	CSmoduleConnector
		/** the particular connector with which this output is linked */
		*containerConnector;

	CSmodule_ContainerOutput();
	/** destructor takes care, that the output of the container module
		will be removed as well */
	~CSmodule_ContainerOutput();

	CSmodule_ContainerOutput* newOne();

	void init();
	/** change name of output of the container module as well */
	void propertyCallback(CSmodProperty *prop);
	/** pass through to output of container module */
	void step();

};



#endif // CSMODULE_CONTAINER_H_INCLUDED
