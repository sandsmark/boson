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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef BOUNITEDITOR_H
#define BOUNITEDITOR_H

#include "ui_bouniteditorbase.h"

#include "bosonweapon.h"
#include "unitproperties.h"
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3PtrList>

class BoUnitEditor;
class BosonSearchPathsWidget;
class BosonWeaponPropertiesEditor;

class EditorUnitProperties : public UnitProperties
{
public:
	EditorUnitProperties(SpeciesTheme* theme);

	TerrainType terrainType() const { return mTerrain; }


	// Methods to set values. They are only meant to be used by unit
	//  editor. Don't use them unless you know what you are doing
	void setName(const QString& name);
	void setTypeId(quint32 id)  { mTypeId = id; }
	void setIsFacility(bool f) { mIsFacility = f; }
	void setUnitWidth(bofixed unitWidth)  { mUnitWidth = unitWidth; }
	void setUnitHeight(bofixed unitHeight)  { mUnitHeight = unitHeight; }
	void setUnitDepth(bofixed unitDepth)  { mUnitDepth = unitDepth; }

	void setProducer(unsigned int producer)  { mProducer = producer; }
	void setTerrainType(TerrainType terrain)  { mTerrain = terrain; }
	void setSupportMiniMap(bool supportMiniMap)  { mSupportMiniMap = supportMiniMap; }
	void setRequirements(Q3ValueList<quint32> requirements);
	void setDestroyedEffectIds(Q3ValueList<quint32> ids);
	void setConstructedEffectIds(Q3ValueList<quint32> ids);
	void setExplodingDamageRange(bofixed range)  { mExplodingDamageRange = range; }
	void setExplodingDamage(long int damage)  { mExplodingDamage = damage; }
	void setHitPoint(const BoVector3Fixed& hitpoint);
	void setRemoveWreckageImmediately(bool remove)  { mRemoveWreckageImmediately = remove; }

	// These only have effect if there is mobile or facility properties
	void setConstructionSteps(unsigned int steps);
	void setRotationSpeed(int speed);
	void setCanGoOnLand(bool c);
	void setCanGoOnWater(bool c);
	void setMaxSlope(bofixed);
	void setCrushDamage(unsigned int);
	void setWaterDepth(bofixed);
	void setIsHelicopter(bool);
	void setPreferredAltitude(bofixed);

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
	bool saveMobileProperties(KConfig* conf);
	bool saveFacilityProperties(KConfig* conf);
	bool saveAllPluginProperties(KConfig* conf);
	bool saveTextureNames(KConfig* conf);
	bool saveSoundNames(KConfig* conf);
};


class BoGeneralPageHandler : public QObject
{
	Q_OBJECT
public:
	BoGeneralPageHandler(BoUnitEditor* parent);

	void updateUnitProperties();
	void updateWidget();

public slots:
	void slotAutoPickId();

private:
	BoUnitEditor* mEditor;
};

class BoPropertiesPageHandler : public QObject
{
	Q_OBJECT
public:
	BoPropertiesPageHandler(BoUnitEditor* parent);

	void updateUnitProperties();
	void updateWidget();

public slots:

protected:
	UnitProperties::TerrainType currentTerrain() const;
	void setCurrentTerrain(UnitProperties::TerrainType);

private:
	BoUnitEditor* mEditor;
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

class BoMappingPageHandler : public QObject
{
	Q_OBJECT
public:
	BoMappingPageHandler(BoUnitEditor* parent);

	void updateUnitProperties();
	void updateWidget();

public slots:
	void slotAddTexture();
	void slotRemoveTexture();
	void slotCurrentTextureChanged();

private:
	BoUnitEditor* mEditor;
};

class BoWeaponPageHandler : public QObject
{
	Q_OBJECT
public:
	BoWeaponPageHandler(BoUnitEditor* parent);
	~BoWeaponPageHandler();

	void updateUnitProperties();
	void updateWidget();

	void updateWeaponProperties();
	void updateWeaponWidgets();

public slots:
	void slotAddWeapon();
	void slotWeaponSelected( int index );
	void slotRemoveWeapon();

private:
	BoUnitEditor* mEditor;
	int mCurrentWeapon;
	Q3PtrList<BosonWeaponPropertiesEditor>* mWeapons;
};

class BoPluginsPageHandler : public QObject
{
	Q_OBJECT
public:
	BoPluginsPageHandler(BoUnitEditor* parent);

	void updateUnitProperties();
	void updateWidget();

public slots:

private:
	BoUnitEditor* mEditor;
};

class BoOtherPageHandler : public QObject
{
	Q_OBJECT
public:
	BoOtherPageHandler(BoUnitEditor* parent);

	void updateUnitProperties();
	void updateWidget();

public slots:

private:
	BoUnitEditor* mEditor;
};


class BoUnitEditor : public QWidget, public Ui::BoUnitEditorBase
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
	Q3ValueList<int> mUsedIds;
	BosonSearchPathsWidget* mSearchPaths;
	QMap<int, QString> mUnits;
	EditorUnitProperties* mUnit;
	bool mConfigChanged;

	friend class BoGeneralPageHandler;
	friend class BoPropertiesPageHandler;
	friend class BoWeaponPageHandler;
	friend class BoPluginsPageHandler;
	friend class BoProducerPageHandler;
	friend class BoMappingPageHandler;
	friend class BoOtherPageHandler;
	BoGeneralPageHandler* mGeneralPageHandler;
	BoPropertiesPageHandler* mPropertiesPageHandler;
	BoWeaponPageHandler* mWeaponPageHandler;
	BoPluginsPageHandler* mPluginsPageHandler;
	BoProducerPageHandler* mProducerPageHandler;
	BoMappingPageHandler* mMappingPageHandler;
	BoOtherPageHandler* mOtherPageHandler;

private:
	void init();
};


#endif

