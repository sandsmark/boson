/*
    This file is part of the Boson game
    Copyright (C) 2002-2004 Rivo Laks (rivolaks@hot.ee)

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

#ifndef BOUNITEDITOR_H
#define BOUNITEDITOR_H

#include "bouniteditorbase.h"

#include "bosonweapon.h"

class BoUnitEditor : public BoUnitEditorBase
{
	Q_OBJECT
public:
	BoUnitEditor(QWidget* parent = 0);
	~BoUnitEditor();

public slots:
	virtual void slotTypeChanged();
	virtual void slotUnitSelected( int index);
	virtual void slotAddTexture();
	virtual void slotRemoveTexture();
	virtual void slotCurrentTextureChanged();
	virtual void slotAutoPickId();
	virtual void slotSaveUnit();
	virtual void slotNewUnit();
	virtual void slotEditSearchPaths();
	virtual void slotOpenUnit();
	virtual void slotAddWeapon();
	virtual void slotWeaponSelected( int index );
	virtual void slotRemoveWeapon();
	virtual void slotConfigChanged();

protected slots:
	void slotLoadUnit( QString dir );
	void slotHideSearchPaths();

protected:
	QStringList verifyProperties();
	void loadUnitsList();
	void updateUnitProperties();
	void updateWidgets();
	void updateWeaponProperties();
	void updateWeaponWidgets();
	void updateConfigWidgets();

protected:
	bool mUnitLoaded;
	int mCurrentWeapon;
	QValueList<int> mUsedIds;
	BosonSearchPathsWidget* mSearchPaths;
	QMap<int, QString> mUnits;
	UnitProperties* mUnit;
	QPtrList<BosonWeaponProperties> mWeapons;
	bool mConfigChanged;

private:
	void init();
};


#endif

