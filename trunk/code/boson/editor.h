#ifndef __EDITOR_H__
#define __EDITOR_H__

#ifdef HAVE_CONFIG_H
//#include <config.h>
#endif 

#include <kmainwindow.h>

class KPlayer;

class EditorPrivate;

class Editor : public KMainWindow
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 **/
	Editor();

	/**
	 * Default Destructor
	 **/
	virtual ~Editor();

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
	void slotFileNew();

	void slotSaveMapAs();
	void slotSaveScenarioAs();
	void slotCreateUnit();

	void optionsShowToolbar();
	void optionsShowStatusbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();

	void slotChangePlayer(int index);
	void slotChangeUnitConstruction(int index);

	void slotPlayerJoinedGame(KPlayer* p);
	void slotPlayerLeftGame(KPlayer* p);

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();

private:
	EditorPrivate* d;
};

#endif // __EDITOR_H__
