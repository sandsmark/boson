/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONMAINWIDGETMENUINPUT_H
#define BOSONMAINWIDGETMENUINPUT_H

#include "defines.h"
#include "bo3dtools.h"

#include <qobject.h>

class BoUfoActionCollection;

class BosonMainWidgetMenuInputPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonMainWidgetMenuInput : public QObject
{
	Q_OBJECT
public:
	BosonMainWidgetMenuInput(BoUfoActionCollection* parentCollection, QObject* parent);
	virtual ~BosonMainWidgetMenuInput();

	BoUfoActionCollection* actionCollection() const;

signals:
	void signalDebugUfoWidgets();
	void signalDebugTextures();
	void signalDebugModels();
	void signalPreferences();

protected slots:
	void slotToggleSound();
	void slotToggleMusic();
	void slotChangeMaxProfilingEntries();
	void slotChangeMaxProfilingAdvanceEntries();
	void slotChangeMaxProfilingGLEntries();
	void slotProfiling();
	void slotClearProfilingData();
	void slotDebugKGame();
	void slotDebugRequestIdName(int msgid, bool userid, QString& name);
	void slotBoDebugLogDialog();
	void slotSleep1s();
	void slotGrabScreenshot();
	void slotGrabProfiling();
	void slotShowGLStates();
	void slotCrashBoson();
	void slotToggleFullScreen(bool fullScreen);

private:
	void init(BoUfoActionCollection* parentCollection);
	void initUfoActions();

private:
	BosonMainWidgetMenuInputPrivate* d;
};

#endif

