/*
    This file is part of the Boson game
    Copyright (C) 2002-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONPROFILINGDIALOG_H
#define BOSONPROFILINGDIALOG_H

#include <kdialogbase.h>

#include "global.h"

class BosonProfilingItem;
class ProfilingItem;
class QListViewItem;
class QListViewItemNumberTime;

class BosonProfilingDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfilingDialog : public KDialogBase
{
	Q_OBJECT
public:
	BosonProfilingDialog(QWidget* parent, bool modal = false);
	~BosonProfilingDialog();

	void loadFromFile(const QString& file);

protected:
	void initEventsPage();
	void initRawTreePage();
	void initFilesPage();

	void reset();
	void resetEventsPage();
	void resetRawTreePage();
	void resetFilesPage();

	void initProfilingItem(QListViewItemNumberTime*, ProfilingItem*, long int totalTime);
	void initRawTreeProfilingItem(QListViewItemNumberTime*, BosonProfilingItem*, long int totalTime);

protected slots:
	void slotUpdateFromGlobalProfiling();
	void slotUpdate();
	void slotSaveToFile();
	void slotLoadFromFile();

private:
	BosonProfilingDialogPrivate* d;
};

#endif
