/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

class QListViewItem;
class QListViewItemNumber;
class ProfileSlotAdvance;
class ProfileItemAdvance;

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
	void initLoadUnitPage();
	void initRenderPage();
	void initSlotAdvancePage();
	void initSlotAdvanceWidget(QWidget* slotAdvanceWidget);
	void initItemAdvanceWidget(QWidget* itemAdvanceWidget);
	void initEventsPage();
	void initFilesPage();

	void reset();
	void resetLoadUnitPage();
	void resetRenderPage();
	void resetSlotAdvanceWidget();
	void resetItemAdvanceWidget();
	void addItemAdvance(ProfileSlotAdvance*);
	void resetEventsPage();
	void resetFilesPage();

	void initRenderItem(QListViewItemNumber* item, const QString& type, long int time, long int function);

	void initSlotAdvanceItem(QListViewItemNumber* item, int advanceCount, const QString& type, long int time, long int function);
	void initItemAdvanceItem(QListViewItemNumber* item, ProfileItemAdvance* a, unsigned int advanceCount, const QString& type, unsigned long int time, unsigned long int function);
	void initItemAdvanceItemSummary(QListViewItemNumber* item, const QString& description, const QString& type, unsigned long int time, unsigned long int function);

	QString profilingName(int profilingEvent) const;

protected slots:
	void slotUpdate();
	void slotSaveToFile();
	void slotLoadFromFile();
	
	void slotResetSlotAdvancePage();

private:
	BosonProfilingDialogPrivate* d;
};

#endif
