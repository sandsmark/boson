/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef __EDITOR_H__
#define __EDITOR_H__

#include "topbase.h"

class KPlayer;

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Editor : public TopBase
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

	void createTiles(bool debug);

private slots:
	void slotFileNew();

	void slotSavePlayFieldAs();
	void slotCreateUnit();
	void slotCreateTiles();
	void slotCreateDebugTiles();

	void slotConfigureKeys();

	void slotPlayerJoinedGame(KPlayer* p);
	void slotPlayerLeftGame(KPlayer* p);
	void slotGameStarted();

	void slotPlaceFacilities();
	void slotPlaceMobiles();
	void slotPlaceCellSmall();
	void slotPlaceCellPlain();
	void slotPlaceCellBig1();
	void slotPlaceCellBig2();

private:
	void initKAction();
	void initStatusBar();

private:
	class EditorPrivate;
	EditorPrivate* d;
};

#endif // __EDITOR_H__
