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
#include "unitproperties.h"

class BoUnitEditor;

class EditorUnitProperties : public UnitProperties
{
public:
	EditorUnitProperties(SpeciesTheme* theme, bool fullMode);


	// Methods to set values. They are only meant to be used by unit
	//  editor. Don't use them unless you know what you are doing
	void setName(const QString& name);
	void setTypeId(unsigned long int id)  { mTypeId = id; }
	void setIsFacility(bool f) { mIsFacility = f; }
	void setUnitWidth(bofixed unitWidth)  { mUnitWidth = unitWidth; }
	void setUnitHeight(bofixed unitHeight)  { mUnitHeight = unitHeight; }
	void setUnitDepth(bofixed unitDepth)  { mUnitDepth = unitDepth; }

	void setProducer(unsigned int producer)  { mProducer = producer; }
	void setTerrainType(TerrainType terrain)  { mTerrain = terrain; }
	void setSupportMiniMap(bool supportMiniMap)  { mSupportMiniMap = supportMiniMap; }
	void setRequirements(QValueList<unsigned long int> requirements);
	void setDestroyedEffectIds(QValueList<unsigned long int> ids);
	void setConstructedEffectIds(QValueList<unsigned long int> ids);
	void setExplodingDamageRange(bofixed range)  { mExplodingDamageRange = range; }
	void setExplodingDamage(long int damage)  { mExplodingDamage = damage; }
	void setHitPoint(const BoVector3Fixed& hitpoint);
	void setRemoveWreckageImmediately(bool remove)  { mRemoveWreckageImmediately = remove; }

	// These only have effect if there is mobile or facility properties
	void setConstructionSteps(unsigned int steps);
	void setRotationSpeed(int speed);
	void setCanGoOnLand(bool c);
	void setCanGoOnWater(bool c);

	void reset();
	void clearPlugins(bool deleteweapons = true);

	void addPlugin(PluginProperties* prop);
	void addTextureMapping(QString shortname, QString longname);
	void addSound(int event, QString filename);

	/**
	 * Save UnitProperties to the file. This sets all values of UnitProperties. All values are
	 * readOnly, as UnitProperties is meant to change never.
	 *
	 * The file should contain units/your_unit_dir/index.desktop at the end
	 * and should be an absolute path.
	 **/
	bool saveUnitType(const QString& fileName);

private:
	bool saveMobileProperties(KSimpleConfig* conf);
	bool saveFacilityProperties(KSimpleConfig* conf);
	bool saveAllPluginProperties(KSimpleConfig* conf);
	bool saveTextureNames(KSimpleConfig* conf);
	bool saveSoundNames(KSimpleConfig* conf);
};

class BoProducerPageHandler : public QObject
{
	Q_OBJECT
public:
	BoProducerPageHandler(BoUnitEditor* parent);

	void updateUnitProperties();
	void updateWidget();

private:
	BoUnitEditor* mEditor;
};

class BoUnitEditor : public BoUnitEditorBase
{
	Q_OBJECT
public:
	BoUnitEditor(QWidget* parent = 0);
	~BoUnitEditor();

	EditorUnitProperties* unit() const
	{
		return mUnit;
	}

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
	EditorUnitProperties* mUnit;
	QPtrList<BosonWeaponProperties> mWeapons;
	bool mConfigChanged;

	friend class BoProducerPageHandler;
	BoProducerPageHandler* mProducerPageHandler;

private:
	void init();
};


#endif

