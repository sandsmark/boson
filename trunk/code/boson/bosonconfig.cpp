/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#include "bosonconfig.h"
#include "defines.h"
#include "boglobal.h"
#include "bodebug.h"
#include "bo3dtools.h"

#include <qptrlist.h>
#include <qdict.h>

#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>

#include <GL/gl.h>
#include <stdlib.h>

class BoGlobalConfigObject : public BoGlobalObject<BosonConfig>
{
public:
	BoGlobalConfigObject()
		: BoGlobalObject<BosonConfig>(BoGlobalObjectBase::BoGlobalConfig, true)
	{
		mPostInitFunction = 0;
	}
	virtual ~BoGlobalConfigObject()
	{
	}

	virtual bool loadObject()
	{
		bool ret = BoGlobalObject<BosonConfig>::loadObject();
		if (ret && pointer() && mPostInitFunction) {
			mPostInitFunction();
		}
		return ret;
	}

	void setPostInitFunction(void (*func)())
	{
		mPostInitFunction = func;
	}

private:
	void (*mPostInitFunction)();
};

static BoGlobalConfigObject globalConfig;


BoConfigEntry::BoConfigEntry(BosonConfig* parent, const QString& key)
{
 mKey = key;
 parent->addConfigEntry(this);
}

BoConfigEntry::~BoConfigEntry()
{
}

void BoConfigEntry::activate(KConfig* conf)
{
 conf->setGroup("Boson");
}

BoConfigBoolEntry::BoConfigBoolEntry(BosonConfig* parent, const QString& key, bool defaultValue)
		: BoConfigEntry(parent, key)
{
 mValue = defaultValue;
}

void BoConfigBoolEntry::save(KConfig* conf)
{
 activate(conf);
 conf->writeEntry(key(), mValue);
}

void BoConfigBoolEntry::load(KConfig* conf)
{
 activate(conf);
 mValue = conf->readBoolEntry(key(), mValue);
}

BoConfigIntEntry::BoConfigIntEntry(BosonConfig* parent, const QString& key, int defaultValue)
		: BoConfigEntry(parent, key)
{
 mValue = defaultValue;
}

void BoConfigIntEntry::save(KConfig* conf)
{
 activate(conf);
 conf->writeEntry(key(), mValue);
}

void BoConfigIntEntry::load(KConfig* conf)
{
 activate(conf);
 mValue = conf->readNumEntry(key(), mValue);
}

BoConfigUIntEntry::BoConfigUIntEntry(BosonConfig* parent, const QString& key, unsigned int defaultValue)
		: BoConfigEntry(parent, key)
{
 mValue = defaultValue;
}

void BoConfigUIntEntry::save(KConfig* conf)
{
 activate(conf);
 conf->writeEntry(key(), mValue);
}

void BoConfigUIntEntry::load(KConfig* conf)
{
 activate(conf);
 mValue = conf->readUnsignedNumEntry(key(), mValue);
}

BoConfigDoubleEntry::BoConfigDoubleEntry(BosonConfig* parent, const QString& key, double defaultValue)
		: BoConfigEntry(parent, key)
{
 mValue = defaultValue;
}

void BoConfigDoubleEntry::save(KConfig* conf)
{
 activate(conf);
 conf->writeEntry(key(), mValue);
}

void BoConfigDoubleEntry::load(KConfig* conf)
{
 activate(conf);
 mValue = conf->readDoubleNumEntry(key(), mValue);
}

BoConfigStringEntry::BoConfigStringEntry(BosonConfig* parent, const QString& key, QString defaultValue)
		: BoConfigEntry(parent, key)
{
 mValue = defaultValue;
}

void BoConfigStringEntry::save(KConfig* conf)
{
 activate(conf);
 conf->writeEntry(key(), mValue);
}

void BoConfigStringEntry::load(KConfig* conf)
{
 activate(conf);
 mValue = conf->readEntry(key(), mValue);
}

class BoConfigIntListEntry : public BoConfigEntry
{
public:
	BoConfigIntListEntry(BosonConfig* parent, const QString& key, QValueList<int> defaultValue) : BoConfigEntry(parent, key)
	{
		mValue = defaultValue;
	}
	~BoConfigIntListEntry() {}

	virtual void load(KConfig* conf)
	{
		activate(conf);
		conf->writeEntry(key(), mValue);
	}
	virtual void save(KConfig* conf)
	{
		activate(conf);
		mValue = conf->readIntListEntry(key());
	}

	virtual int type() const { return IntList; }

	void setValue(QValueList<int> list) { mValue = list; }
	QValueList<int> value() const { return mValue; }

	void append(int e)
	{
		if (!contains(e)) {
			mValue.append(e);
		}
	}
	void remove(int e) { mValue.remove(e); }
	bool contains(int e) { return mValue.contains(e); }

private:
	QValueList<int> mValue;
};

BoConfigColorEntry::BoConfigColorEntry(BosonConfig* parent, const QString& key, const QColor& defaultValue)
		: BoConfigEntry(parent, key)
{
 mRGBValue = defaultValue.rgb();
}

void BoConfigColorEntry::save(KConfig* conf)
{
 activate(conf);
 conf->writeEntry(key(), value());
}

void BoConfigColorEntry::load(KConfig* conf)
{
 activate(conf);
 QColor def = value();
 mRGBValue = conf->readColorEntry(key(), &def).rgb();
}

void BoConfigColorEntry::setValue(const QColor& v)
{
 setValue(v.rgb());
}

QColor BoConfigColorEntry::value() const
{
 return QColor(mRGBValue);
}



class BosonConfig::BosonConfigPrivate
{
public:
	BosonConfigPrivate()
	{
	}

	QPtrList<BoConfigEntry> mConfigEntries;
	QDict<BoConfigEntry> mDynamicEntries; // added dynamically.
};

BosonConfig::BosonConfig(KConfig* conf)
{
 d = new BosonConfigPrivate;
 d->mConfigEntries.setAutoDelete(true);
 // don't delete them - they are in mConfigEntries, too and will get deleted
 // there
 d->mDynamicEntries.setAutoDelete(false);

 mSound = new BoConfigBoolEntry(this, "Sound", DEFAULT_SOUND);
 mMusic = new BoConfigBoolEntry(this, "Music", DEFAULT_MUSIC);
 mMMBMove = new BoConfigBoolEntry(this, "MMBMove", DEFAULT_USE_MMB_MOVE);
 mRMBMove = new BoConfigBoolEntry(this, "RMBMove", DEFAULT_USE_RMB_MOVE);
 mShowMenubarInGame = new BoConfigBoolEntry(this, "ShowMenubarInGame", true);
 mShowMenubarOnStartup = new BoConfigBoolEntry(this, "ShowMenubarOnStartup", false);
 mArrowKeyStep = new BoConfigUIntEntry(this, "ArrowKeyStep", DEFAULT_ARROW_SCROLL_SPEED);
 mCursorEdgeSensity = new BoConfigUIntEntry(this, "CursorEdgeSensity", DEFAULT_CURSOR_EDGE_SENSITY);
 mUpdateInterval = new BoConfigUIntEntry(this, "GLUpdateInterval", DEFAULT_UPDATE_INTERVAL);
 mMiniMapScale = new BoConfigDoubleEntry(this, "MiniMapScale", DEFAULT_MINIMAP_SCALE);
 mMiniMapZoom = new BoConfigDoubleEntry(this, "MiniMapZoom", DEFAULT_MINIMAP_ZOOM);
 mChatScreenRemoveTime = new BoConfigUIntEntry(this, "ChatScreenRemoveTime", DEFAULT_CHAT_SCREEN_REMOVE_TIME);
 mChatScreenMaxItems = new BoConfigIntEntry(this, "ChatScreenMaxItems", DEFAULT_CHAT_SCREEN_MAX_ITEMS);
 mUnitSoundsDeactivated = new BoConfigIntListEntry(this, "DeactivateUnitSounds", QValueList<int>());
 mAlignSelectionBoxes = new BoConfigBoolEntry(this, "AlignSelectionBoxes", DEFAULT_ALIGN_SELECTION_BOXES);
 mRMBMovesWithAttack = new BoConfigBoolEntry(this, "RMBMovesWithAttack", DEFAULT_RMB_MOVES_WITH_ATTACK);
 mMouseWheelAction = new BoConfigIntEntry(this, "MouseWheelAction", DEFAULT_MOUSE_WHEEL_ACTION);
 mMouseWheelShiftAction = new BoConfigIntEntry(this, "MouseWheelShiftAction", DEFAULT_MOUSE_WHEEL_SHIFT_ACTION);
 mDeactivateWeaponSounds = new BoConfigBoolEntry(this, "DeactivateWeaponSounds", DEFAULT_DEACTIVATE_WEAPON_SOUNDS);
 mUseLight = new BoConfigBoolEntry(this, "UseLight", DEFAULT_USE_LIGHT);
 mUseMaterials = new BoConfigBoolEntry(this, "UseMaterials", DEFAULT_USE_MATERIALS);
 mCursorMode = new BoConfigIntEntry(this, "CursorMode", (int)DEFAULT_CURSOR);
 mCursorDir = new BoConfigStringEntry(this, "CursorDir", DEFAULT_CURSOR_DIR);
 mToolTipUpdatePeriod = new BoConfigIntEntry(this, "ToolTipUpdatePeriod", DEFAULT_TOOLTIP_UPDATE_PERIOD);
 mToolTipCreator = new BoConfigIntEntry(this, "ToolTipCreator", DEFAULT_TOOLTIP_CREATOR);
 mGameLogInterval = new BoConfigIntEntry(this, "GameLogInterval", 10);
 mUseLOD = new BoConfigBoolEntry(this, "UseLOD", DEFAULT_USE_LOD);
 mUseVBO = new BoConfigBoolEntry(this, "UseVBO", DEFAULT_USE_VBO);
 mWaterWaves = new BoConfigBoolEntry(this, "WaterWaves", DEFAULT_WATER_WAVES);
 mWaterReflections = new BoConfigBoolEntry(this, "WaterReflections", DEFAULT_WATER_REFLECTIONS);
 mWaterTranslucency = new BoConfigBoolEntry(this, "WaterTranslucency", DEFAULT_WATER_TRANSLUCENCY);
 mWaterBumpmapping = new BoConfigBoolEntry(this, "WaterBumpmapping", DEFAULT_WATER_BUMPMAPPING);
 mWaterAnimatedBumpmaps = new BoConfigBoolEntry(this, "WaterAnimatedBumpmaps", DEFAULT_WATER_ANIMATED_BUMPMAPS);
 mTextureFilter = new BoConfigIntEntry(this, "TextureFilter", DEFAULT_TEXTURE_FILTER);
 mTextureCompression = new BoConfigBoolEntry(this, "TextureCompression", DEFAULT_TEXTURE_COMPRESSION);
 mTextureColorMipmaps = new BoConfigBoolEntry(this, "TextureColorMipmaps", false);
 mTextureAnisotropy = new BoConfigIntEntry(this, "TextureAnisotropy", 1);

 mDebugMode = DebugNormal;

 mDisableSound = true; // disabled by default, to make bounit and other non-sound applications work correctly, without calling setDisableSound(true) explicitly.
 mAIDelay = 3.0;
 mWantDirect = true;
 mWireFrames = false;
 mDebugOpenGLMatrices = false;
 mDebugMapCoordinates = false;
 mDebugPFData = false;
 mDebugShowCellGrid = false;
 mDebugItemWorkStatistics = false;
 mDebugOpenGLCamera = false;
 mDebugRenderCounts = false;
 mDebugBoundingBoxes = false;
 mDebugFPS = false;
 mDebugAdvanceCalls = false;
 mDebugTextureMemory = false;
 mShowResources = true;
 mEnableColormap = false;
 mDisableModelLoading = false;
 mTextureFOW = true;
 mDefaultLodCount = 5;

 // these are dynamic entries. usually they are added in the class where they
 // get used, but sometimes it is also handy to add them here (e.g. when it
 // isn't 100% clear which class will reference it first).
 addDynamicEntry(new BoConfigUIntEntry(this, "GroundRenderer", 0)); // obsolete
 addDynamicEntry(new BoConfigUIntEntry(this, "DefaultLOD", 0));
 addDynamicEntry(new BoConfigBoolEntry(this, "EnableATIDepthWorkaround", false));
 addDynamicEntry(new BoConfigDoubleEntry(this, "ATIDepthWorkaroundValue", 0.00390625));
 addDynamicEntry(new BoConfigStringEntry(this, "GLFont", QString::null));
 addDynamicEntry(new BoConfigBoolEntry(this, "SmoothShading", true));
 addDynamicEntry(new BoConfigStringEntry(this, "MeshRenderer", "BoMeshRendererVertexArray"));
 addDynamicEntry(new BoConfigStringEntry(this, "GroundRendererClass", DEFAULT_GROUND_RENDERER));

 // load from config
 reset(conf);
}

BosonConfig::~BosonConfig()
{
 d->mDynamicEntries.clear();
 d->mConfigEntries.clear();
 delete d;
}

BosonConfig* BosonConfig::bosonConfig()
{
 return BoGlobal::boGlobal()->bosonConfig();
}

void BosonConfig::setPostInitFunction(void (*func)())
{
 if (!func) {
	return;
 }
 globalConfig.setPostInitFunction(func);
}

BoVector3Float BosonConfig::readBoVector3FloatEntry(const KConfig* cfg, const QString& key, const BoVector3Float& aDefault)
{
 QValueList<float> list = BosonConfig::readFloatNumList(cfg, key);
 if (list.count() != 3) {
	if (list.count() != 0) {
		boError() << k_funcinfo
				<< "BoVector3 entry must have 3 floats, not "
				<< list.count() << endl;
	}
	return aDefault;
 }
 return BoVector3Float(list[0], list[1], list[2]);
}

BoVector3Float BosonConfig::readBoVector3FloatEntry(const KConfig* cfg, const QString& key)
{
 return readBoVector3FloatEntry(cfg, key, BoVector3Float());
}

BoVector3Fixed BosonConfig::readBoVector3FixedEntry(const KConfig* cfg, const QString& key, const BoVector3Fixed& aDefault)
{
 QValueList<float> list = BosonConfig::readFloatNumList(cfg, key);
 if (list.count() != 3) {
	if (list.count() != 0) {
		boError() << k_funcinfo
				<< "BoVector3 entry must have 3 floats, not "
				<< list.count() << endl;
	}
	return aDefault;
 }
 return BoVector3Fixed(list[0], list[1], list[2]);
}

BoVector3Fixed BosonConfig::readBoVector3FixedEntry(const KConfig* cfg, const QString& key)
{
 return readBoVector3FixedEntry(cfg, key, BoVector3Fixed());
}

BoVector4Float BosonConfig::readBoVector4FloatEntry(const KConfig* cfg, const QString& key)
{
 return readBoVector4FloatEntry(cfg, key, BoVector4Float());
}

BoVector4Float BosonConfig::readBoVector4FloatEntry(const KConfig* cfg, const QString& key, const BoVector4Float& aDefault)
{
 QValueList<float> list = BosonConfig::readFloatNumList(cfg, key);
 if (list.count() != 4) {
	if (list.count() != 0) {
		boError() << k_funcinfo
				<< "BoVector4 entry must have 4 floats, not "
				<< list.count() << endl;
	}
	return aDefault;
 }
 return BoVector4Float(list[0], list[1], list[2], list[3]);
}

BoVector4Fixed BosonConfig::readBoVector4FixedEntry(const KConfig* cfg, const QString& key)
{
 return readBoVector4FixedEntry(cfg, key, BoVector4Fixed());
}

BoVector4Fixed BosonConfig::readBoVector4FixedEntry(const KConfig* cfg, const QString& key, const BoVector4Fixed& aDefault)
{
 QValueList<float> list = BosonConfig::readFloatNumList(cfg, key);
 if (list.count() != 4) {
	if (list.count() != 0) {
		boError() << k_funcinfo
				<< "BoVector4 entry must have 4 floats, not "
				<< list.count() << endl;
	}
	return aDefault;
 }
 return BoVector4Fixed(list[0], list[1], list[2], list[3]);
}

void BosonConfig::writeEntry(KConfig* cfg, const QString& key, const BoVector3Float& value)
{
  QValueList<float> list;
  list.append(value[0]);
  list.append(value[1]);
  list.append(value[2]);
  BosonConfig::writeFloatNumList(list, cfg, key);
}

void BosonConfig::writeEntry(KConfig* cfg, const QString& key, const BoVector3Fixed& value)
{
  QValueList<float> list;
  list.append(value[0]);
  list.append(value[1]);
  list.append(value[2]);
  BosonConfig::writeFloatNumList(list, cfg, key);
}

void BosonConfig::writeEntry(KConfig* cfg, const QString& key, const BoVector4Float& value)
{
  QValueList<float> list;
  list.append(value[0]);
  list.append(value[1]);
  list.append(value[2]);
  list.append(value[3]);
  BosonConfig::writeFloatNumList(list, cfg, key);
}

void BosonConfig::writeEntry(KConfig* cfg, const QString& key, const BoVector4Fixed& value)
{
  QValueList<float> list;
  list.append(value[0]);
  list.append(value[1]);
  list.append(value[2]);
  list.append(value[3]);
  BosonConfig::writeFloatNumList(list, cfg, key);
}

void BosonConfig::addConfigEntry(BoConfigEntry* c)
{
 if (!c) {
	return;
 }
 d->mConfigEntries.append(c);
}

QString BosonConfig::readLocalPlayerName(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 QString name = conf->readEntry("LocalPlayer", getenv("LOGNAME"));
 conf->setGroup(oldGroup);
 return name;
}

void BosonConfig::saveLocalPlayerName(const QString& name, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("LocalPlayer", name);
 conf->setGroup(oldGroup);
}

QString BosonConfig::readComputerPlayerName(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 QString name = conf->readEntry("ComputerPlayer", i18n("Computer"));
 conf->setGroup(oldGroup);
 return name;
}

void BosonConfig::saveComputerPlayerName(const QString& name, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("ComputerPlayer", name);
 conf->setGroup(oldGroup);
}

QColor BosonConfig::readLocalPlayerColor(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 QColor color = conf->readColorEntry("LocalPlayerColor", &Qt::red);
 conf->setGroup(oldGroup);
 return color;
}

void BosonConfig::saveLocalPlayerColor(const QColor& color, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("LocalPlayerColor", color);
 conf->setGroup(oldGroup);
}

QString BosonConfig::readLocalPlayerMap(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 QString name = conf->readEntry("LocalPlayerMap", QString::null);
 conf->setGroup(oldGroup);
 return name;
}

void BosonConfig::saveLocalPlayerMap(const QString& id, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("LocalPlayerMap", id);
 conf->setGroup(oldGroup);
}

QString BosonConfig::readEditorMap(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Editor");
 QString name = conf->readEntry("EditorMap", QString::null);
 conf->setGroup(oldGroup);
 return name;
}

void BosonConfig::saveEditorMap(const QString& id, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Editor");
 conf->writeEntry("EditorMap", id);
 conf->setGroup(oldGroup);
}

bool BosonConfig::readEditorCreateNewMap(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Editor");
 bool createnew = conf->readBoolEntry("CreateNewMap", true);
 conf->setGroup(oldGroup);
 return createnew;
}

void BosonConfig::saveEditorCreateNewMap(bool createnew, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Editor");
 conf->writeEntry("CreateNewMap", createnew);
 conf->setGroup(oldGroup);
}

void BosonConfig::saveGameSpeed(int speed, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("Speed", speed);
 conf->setGroup(oldGroup);
}

int BosonConfig::readGameSpeed(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 int speed = conf->readNumEntry("Speed", DEFAULT_GAME_SPEED);
 conf->setGroup(oldGroup);
 return speed;
}

void BosonConfig::reset(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 // the old group is already stored here so we don't have to re-set it in every
 // read function
 QPtrListIterator<BoConfigEntry> it(d->mConfigEntries);
 for (; it.current(); ++it) {
	it.current()->load(conf);
 }

 conf->setGroup(oldGroup);
}

void BosonConfig::save(bool /*editor*/, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 // the old group is already stored here so we don't have to re-set it in every
 // save function
 QPtrListIterator<BoConfigEntry> it(d->mConfigEntries);
 for (; it.current(); ++it) {
	it.current()->save(conf);
 }

 conf->setGroup(oldGroup);
}

void BosonConfig::setUnitSoundActivated(UnitSoundEvent e, bool activated)
{
 if (activated) {
	mUnitSoundsDeactivated->remove((int)e);
 } else {
	mUnitSoundsDeactivated->append((int)e);
 }
}

bool BosonConfig::unitSoundActivated(UnitSoundEvent e) const
{
 return !mUnitSoundsDeactivated->contains((int)e);
}

QValueList<unsigned long int> BosonConfig::readUnsignedLongNumList(const KConfig* cfg, const QString key)
{
 QValueList<unsigned long int> list;
 QValueList<int> tmplist = cfg->readIntListEntry(key);
 QValueList<int>::Iterator it;
 for (it = tmplist.begin(); it != tmplist.end(); it++) {
	list.append((unsigned long int)(*it));
 }
 return list;
}

void BosonConfig::writeUnsignedLongNumList(KConfig* cfg, const QString key, QValueList<unsigned long int> list)
{
 QValueList<int> tmplist;
 QValueList<unsigned long int>::Iterator it;
 for (it = list.begin(); it != list.end(); it++) {
	tmplist.append((int)(*it));
 }
	cfg->writeEntry(key, tmplist);
}

QValueList<float> BosonConfig::readFloatNumList(const KConfig* cfg, const QString key)
{
 QStringList strlist = cfg->readListEntry(key);
 QValueList<float> list;
 for (QStringList::Iterator it = strlist.begin(); it != strlist.end(); it++) {
	list.append((*it).toFloat());
 }
 return list;
}

void BosonConfig::writeFloatNumList(QValueList<float> list, KConfig* cfg, const QString key)
{
 QStringList strlist;
 QString str;
 for (QValueList<float>::Iterator it = list.begin(); it != list.end(); it++) {
	strlist.append(str.setNum(*it));
 }
 cfg->writeEntry(key, strlist);
}

void BosonConfig::addDynamicEntry(BoConfigEntry* entry, KConfig* conf)
{
 BO_CHECK_NULL_RET(entry);
 if (entry->key().isEmpty()) {
	boError() << k_funcinfo << "Cannot add empty key" << endl;
	delete entry;
	return;
 }
 if (hasKey(entry->key())) {
	boError() << k_funcinfo << "entry " << entry->key() << " already there" << endl;
	delete entry;
	return;
 }
 d->mDynamicEntries.insert(entry->key(), entry);
 if (!conf) {
	conf = kapp->config();
 }
 entry->load(conf);
}

bool BosonConfig::hasKey(const QString& key) const
{
 return (d->mDynamicEntries.find(key) != 0);
}

BoConfigEntry* BosonConfig::value(const QString& key) const
{
 return d->mDynamicEntries[key];
}

void BosonConfig::setBoolValue(const QString& key, bool v)
{
 BoConfigEntry* entry = value(key);
 BO_CHECK_NULL_RET(entry);
 if (entry->type() != BoConfigEntry::Bool) {
	boError() << k_funcinfo << key << "is not a boolean entry. type=" << entry->type() << endl;
	return;
 }
 ((BoConfigBoolEntry*)entry)->setValue(v);
}

void BosonConfig::setIntValue(const QString& key, int v)
{
 BoConfigEntry* entry = value(key);
 BO_CHECK_NULL_RET(entry);
 if (entry->type() != BoConfigEntry::Int) {
	boError() << k_funcinfo << key << "is not an integer entry. type=" << entry->type() << endl;
	return;
 }
 ((BoConfigIntEntry*)entry)->setValue(v);
}

void BosonConfig::setUIntValue(const QString& key, unsigned int v)
{
 BoConfigEntry* entry = value(key);
 BO_CHECK_NULL_RET(entry);
 if (entry->type() != BoConfigEntry::UInt) {
	boError() << k_funcinfo << key << "is not an unsigned integer entry. type=" << entry->type() << endl;
	return;
 }
 ((BoConfigUIntEntry*)entry)->setValue(v);
}

void BosonConfig::setStringValue(const QString& key, const QString& v)
{
 BoConfigEntry* entry = value(key);
 BO_CHECK_NULL_RET(entry);
 if (entry->type() != BoConfigEntry::String) {
	boError() << k_funcinfo << key << "is not a string entry. type=" << entry->type() << endl;
	return;
 }
 ((BoConfigStringEntry*)entry)->setValue(v);
}

void BosonConfig::setDoubleValue(const QString& key, double v)
{
 BoConfigEntry* entry = value(key);
 BO_CHECK_NULL_RET(entry);
 if (entry->type() != BoConfigEntry::Double) {
	boError() << k_funcinfo << key << "is not a double entry. type=" << entry->type() << endl;
	return;
 }
 ((BoConfigDoubleEntry*)entry)->setValue(v);
}

void BosonConfig::setColorValue(const QString& key, const QColor& v)
{
 BoConfigEntry* entry = value(key);
 BO_CHECK_NULL_RET(entry);
 if (entry->type() != BoConfigEntry::Color) {
	boError() << k_funcinfo << key << "is not a color entry. type=" << entry->type() << endl;
	return;
 }
 ((BoConfigColorEntry*)entry)->setValue(v);
}

bool BosonConfig::boolValue(const QString& key, bool _default) const
{
 BoConfigEntry* entry = value(key);
 if (!entry) {
	boError() << k_funcinfo << "no key " << key << endl;
	return _default;
 }
 if (entry->type() != BoConfigEntry::Bool) {
	boError() << k_funcinfo << key << "is not a bool entry. type=" << entry->type() << endl;
	return _default;
 }
 return ((BoConfigBoolEntry*)entry)->value();
}

int BosonConfig::intValue(const QString& key, int _default) const
{
 BoConfigEntry* entry = value(key);
 if (!entry) {
	boError() << k_funcinfo << "no key " << key << endl;
	return _default;
 }
 if (entry->type() != BoConfigEntry::Int) {
	boError() << k_funcinfo << key << "is not an integer entry. type=" << entry->type() << endl;
	return _default;
 }
 return ((BoConfigIntEntry*)entry)->value();
}

unsigned int BosonConfig::uintValue(const QString& key, unsigned int _default) const
{
 BoConfigEntry* entry = value(key);
 if (!entry) {
	boError() << k_funcinfo << "no key " << key << endl;
	return _default;
 }
 if (entry->type() != BoConfigEntry::UInt) {
	boError() << k_funcinfo << key << "is not an unsigned integer entry. type=" << entry->type() << endl;
	return _default;
 }
 return ((BoConfigUIntEntry*)entry)->value();
}

const QString&BosonConfig::stringValue(const QString& key, const QString& _default) const
{
 BoConfigEntry* entry = value(key);
 if (!entry) {
	boError() << k_funcinfo << "no key " << key << endl;
	return _default;
 }
 if (entry->type() != BoConfigEntry::String) {
	boError() << k_funcinfo << key << "is not a string entry. type=" << entry->type() << endl;
	return _default;
 }
 return ((BoConfigStringEntry*)entry)->value();
}

double BosonConfig::doubleValue(const QString& key, double _default) const
{
 BoConfigEntry* entry = value(key);
 if (!entry) {
	boError() << k_funcinfo << "no key " << key << endl;
	return _default;
 }
 if (entry->type() != BoConfigEntry::Double) {
	boError() << k_funcinfo << key << "is not a double entry. type=" << entry->type() << endl;
	return _default;
 }
 return ((BoConfigDoubleEntry*)entry)->value();
}

QColor BosonConfig::colorValue(const QString& key) const
{
 return colorValue(key, Qt::black);
}

QColor BosonConfig::colorValue(const QString& key, const QColor& _default) const
{
 BoConfigEntry* entry = value(key);
 if (!entry) {
	boError() << k_funcinfo << "no key " << key << endl;
	return _default;
 }
 if (entry->type() != BoConfigEntry::Color) {
	boError() << k_funcinfo << key << "is not a color entry. type=" << entry->type() << endl;
	return _default;
 }
 return ((BoConfigColorEntry*)entry)->value();
}

