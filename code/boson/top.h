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
#ifndef TOP_H
#define TOP_H

#include "topbase.h"

/**
 * @author Thomas Capricelli <capricel@email.enst.fr>, Andreas Beckermann <b_mann@gmx.de>
 **/
class Top : public TopBase
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
	void slotConfigureKeys();

private:
	void initKAction();
	void initStatusBar();

private:
};

#endif
