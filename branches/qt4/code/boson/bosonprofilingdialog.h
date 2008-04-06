/*
    This file is part of the Boson game
    Copyright (C) 2002-2006 Andreas Beckermann (b_mann@gmx.de)

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef BOSONPROFILINGDIALOG_H
#define BOSONPROFILINGDIALOG_H

#include <KDialog>

#include "global.h"

class BosonProfilingItem;
class ProfilingItem;
class Q3ListViewItem;
class QListViewItemNumberTime;

class BosonProfilingDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonProfilingDialog : public KDialog
{
	Q_OBJECT
public:
	BosonProfilingDialog(QWidget* parent, bool modal = false);
	~BosonProfilingDialog();

	void loadFromFile(const QString& file);

protected:
	void reset();
	void resetEventsPage();
	void resetEventLeafsPage();
	void resetRawTreePage();
	void resetFilesPage();

	void initProfilingItem(QListViewItemNumberTime*, ProfilingItem*, long int totalTime);
	void initRawTreeProfilingItem(QListViewItemNumberTime*, BosonProfilingItem*, long int totalTime);

protected slots:
	void slotUpdateFromGlobalProfiling();
	void slotUpdate();
	void slotSaveToFile();
	void slotLoadFromFile();
	void slotShowSumForEvent(Q3ListViewItem*);

private:
	BosonProfilingDialogPrivate* d;
};

#endif
