/**
 *
 * CS MODULAR EDITOR
 *
 * MODULE: Abstract bitmap buffer containing module class
 *
 * PURPOSE: connecting to different buffer servers
 *
 * @version 2011/04/01 v0.1
 * @author def.gsus-
 *
 */


#ifndef CSMODULE_DISPLAY_H_INCLUDED
#define CSMODULE_DISPLAY_H_INCLUDED

/**
	abstract class that implements some functions to handle and draw
	a bitmap buffer and to find other modules based on this abstract class
*/
class CSmodule_AbstractDisplay: public CSmodule
{
	public:

	/** the image buffer,
		it's considered 8 bits per color, 32 bits per pixel, RGBx */
	uchar *img;

	Fl_Color
		oldBodyColor,
		oldBodySelColor;

	unsigned int
		/** width and height of the buffer */
		w,h;
	bool
		/** this is true when module is client a client and connected to a server */
		isClient,
		/** true when a server is requested AND found */
		isServer,
		/** true when new server connection is requested */
		requestServerChange,
		wasScreenUpdate;

	CSmodule_AbstractDisplay
		/** the actual used server, or 0 */
		*server,
		/** the new server when change is requested */
		*server1;

	public:
	~CSmodule_AbstractDisplay();
	/** called from container when the module is added */
	void onAddModule();
	/** inits some variables and calls parent init() */
	void init();
	/** buffer size and server change */
	void propertyCallback(CSmodProperty *prop);
	/** sets the window title + size info */
	void setWindowCaption();
	/** alloc the buffer */
	void setImage(int w,int h);

	/** returns the scope module within container (or subcontainers) that has the matching
		server name, or NULL */
	CSmodule_AbstractDisplay *findServer(CSmodContainer *container, const char *servername);
	/** set into client mode and connect to the display with serverName (if any) */
	virtual void connectToServer(const char *serverName);
	/** update all display's server/clients in the container (and subcontainers) */
	void checkAllServer(CSmodContainer *container);

	/** will connect to new server if requested and found,
		make sure to call this before using @see server */
	virtual void step();

	/** add the buffer @param img1 on current buffer */
	void addBuffer(const uchar *img1, unsigned int w1, unsigned int h1);

	/** resize buffer on window change */
	void onResizeWindow(int newW, int newH);

};


#endif // CSMODULE_DISPLAY_H_INCLUDED
