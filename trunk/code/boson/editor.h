/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __EDITOR_H__
#define __EDITOR_H__

#ifdef HAVE_CONFIG_H
//#include <config.h>
#endif 

#include <kmainwindow.h>

class KPlayer;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
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
	void slotCreateTiles();

	void optionsShowToolbar();
	void optionsShowStatusbar();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();

	void slotChangePlayer(int index);
	void slotChangeUnitConstruction(int index);

	void slotPlayerJoinedGame(KPlayer* p);
	void slotPlayerLeftGame(KPlayer* p);

	void slotZoom(int index);

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();

private:
	class EditorPrivate;
	EditorPrivate* d;
};

#endif // __EDITOR_H__
