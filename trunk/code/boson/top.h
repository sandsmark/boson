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
