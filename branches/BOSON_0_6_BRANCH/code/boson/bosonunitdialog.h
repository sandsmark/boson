/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOSONUNITDIALOG_H
#define BOSONUNITDIALOG_H

#include <kdialogbase.h>

class KSimpleConfig;

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUnitDialog : public KDialogBase
{
	Q_OBJECT
public:
	BosonUnitDialog(QWidget* parent = 0);
	~BosonUnitDialog();

protected slots:
	void slotChangeUnitDir();
	void slotCreateUnit();
	void slotTypeChanged(int);

protected:
	void loadConfig(const QString& file);

	void loadProperties();
	void loadMobileProperties();
	void loadFacilityProperties();
	void loadPixmaps();

	void saveProperties(KSimpleConfig* config);
	void saveMobileProperties(KSimpleConfig* config);
	void saveFacilityProperties(KSimpleConfig* config);

private:
	void initDirectoriesPage();
	void initPropertiesPage();
	void initMobileProperties(QWidget* page);
	void initFacilityProperties(QWidget* page);
	void initPixmapsPage();

private:
	class BosonUnitDialogPrivate;
	BosonUnitDialogPrivate* d;
};

#endif
