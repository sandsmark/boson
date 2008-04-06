/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann <b_mann@gmx.de>

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
#ifndef BOINFODIALOG_H
#define BOINFODIALOG_H

#include <kdialogbase.h>

class Q3ListViewItem;
class QListViewItemNumber;
class ProfileSlotAdvance;
class ProfileItemAdvance;

class BoInfoDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 * @short Dialog that displays the data from @ref BoInfo
 **/
class BoInfoDialog : public KDialogBase
{
	Q_OBJECT
public:
	BoInfoDialog(QWidget* parent, bool modal = false);
	~BoInfoDialog();

	void loadFromFile(const QString& file);

	void reset();

protected:
	void initBosonPage();
	void initQtPage();
	void initKDEPage();
	void initOpenGLPage();
	void initXPage();
	void initNVidiaPage();
	void initOSPage();
	void initLibsPage();
	void initCompleteDataPage();
	void initFilePage();

	void resetBosonPage();
	void resetQtPage();
	void resetKDEPage();
	void resetOpenGLPage();
	void resetXPage();
	void resetNVidiaPage();
	void resetOSPage();
	void resetLibsPage();
	void resetCompleteDataPage();
	void resetFilePage();
	
protected slots:
	void slotSaveToFile();
	void slotLoadFromFile();
	
private:
	BoInfoDialogPrivate* d;
};

#endif
