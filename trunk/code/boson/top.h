#ifndef __TOP_H__
#define __TOP_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <kmainwindow.h>

class TopPrivate;

class Top : public KMainWindow
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	Top();

	/**
	 * Default Destructor
	 **/
	virtual ~Top();

protected:

protected:
	/**
	 * This function is called when it is time for the app to save its
	 * properties for session management purposes.
	 **/
	void saveProperties(KConfig *);

	/**
	 * This function is called when this app is restored.  The KConfig
	 * object points to the session management config file that was saved
	 * with @ref saveProperties
	 **/
	void readProperties(KConfig *);


private slots:
	void slotGameNew();
	void slotZoom(int index);
	void fileSave();
	void fileSaveAs();

	void optionsShowToolbar();
	void optionsShowStatusbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();

private:
	TopPrivate* d;
};

#endif // __TOP_H__
