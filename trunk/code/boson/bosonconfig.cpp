/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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
#include "../bomemory/bodummymemory.h"
#include "boglobal.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include <bogl.h>

#include <qptrlist.h>
#include <qdict.h>

#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>

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


BoConfigEntry::BoConfigEntry(BosonConfig* parent, const QString& key, bool saveConfig)
{
 mKey = key;
 mSaveConfig = saveConfig;
 parent->addConfigEntry(this);
}

BoConfigEntry::~BoConfigEntry()
{
}

void BoConfigEntry::activate(KConfig* conf)
{
 conf->setGroup("Boson");
}

BoConfigBoolEntry::BoConfigBoolEntry(BosonConfig* parent, const QString& key, bool defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
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

BoConfigIntEntry::BoConfigIntEntry(BosonConfig* parent, const QString& key, int defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
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

BoConfigUIntEntry::BoConfigUIntEntry(BosonConfig* parent, const QString& key, unsigned int defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
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

BoConfigDoubleEntry::BoConfigDoubleEntry(BosonConfig* parent, const QString& key, double defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
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

BoConfigStringEntry::BoConfigStringEntry(BosonConfig* parent, const QString& key, QString defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
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
	BoConfigIntListEntry(BosonConfig* parent, const QString& key, QValueList<int> defaultValue, bool saveConfig = true)
		: BoConfigEntry(parent, key, saveConfig)
	{
		mDefaultValue = defaultValue;
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
	QValueList<int> defaultValue() const { return mDefaultValue; }

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
	QValueList<int> mDefaultValue;
};

BoConfigColorEntry::BoConfigColorEntry(BosonConfig* parent, const QString& key, const QColor& defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultRGBValue = defaultValue.rgb();
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

QColor BoConfigColorEntry::defaultValue() const
{
 return QColor(mDefaultRGBValue);
}



class BosonConfig::BosonConfigPrivate
{
public:
	BosonConfigPrivate()
		: mDynamicEntries(QDict<BoConfigEntry>(101))
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


 addDynamicEntryBool("Sound", false);
 addDynamicEntryBool("Music", true);
 addDynamicEntryBool("MMBMove", true);
 addDynamicEntryBool("RMBMove", true);
 addDynamicEntryBool("WheelMoveZoom", false); // whether we center the screen on the mouse on mouse wheel zoom
 addDynamicEntryUInt("ArrowKeyStep", 10);
 addDynamicEntryUInt("CursorEdgeSensity", 20);
 addDynamicEntryUInt("GLUpdateInterval", 20);
 addDynamicEntryDouble("MiniMapScale", 2.0);
 addDynamicEntryDouble("MiniMapZoom", 1.0);
 addDynamicEntryUInt("ChatScreenRemoveTime", 10);
 addDynamicEntryInt("ChatScreenMaxItems", 5);
 addDynamicEntryIntList("DeactivateUnitSounds", QValueList<int>());
 addDynamicEntryBool("AlignSelectionBoxes", true);
 addDynamicEntryBool("RMBMovesWithAttack", true);
 addDynamicEntryInt("MouseWheelAction", CameraZoom);
 addDynamicEntryInt("MouseWheelShiftAction", CameraRotate);
 addDynamicEntryBool("DeactivateWeaponSounds", false);
 addDynamicEntryBool("UseLight", true);
 addDynamicEntryBool("UseMaterials", false);
 addDynamicEntryInt("CursorMode", (int)CursorOpenGL);
 addDynamicEntryString("CursorDir", QString::null); // QString::null means use BosonCursor::defaultTheme()
 addDynamicEntryInt("ToolTipUpdatePeriod", 300);
 addDynamicEntryInt("ToolTipCreator", 1); // FIXME: should be BoToolTipCreator::Extended, but I don't want to include the file here
 addDynamicEntryInt("GameLogInterval", 10);
 addDynamicEntryBool("UseLOD", true);
 addDynamicEntryBool("UseVBO", false); // NVidia drivers don't properly support VBOs
 addDynamicEntryBool("WaterShaders", true);
 addDynamicEntryBool("WaterReflections", true);
 addDynamicEntryBool("WaterTranslucency", true);
 addDynamicEntryBool("WaterBumpmapping", true);
 addDynamicEntryBool("WaterAnimatedBumpmaps", true);
 addDynamicEntryInt("TextureFilter", GL_LINEAR_MIPMAP_LINEAR);
 addDynamicEntryBool("TextureCompression", true);
 addDynamicEntryBool("TextureColorMipmaps", false);
 addDynamicEntryInt("TextureAnisotropy", 1);
 addDynamicEntryUInt("MaxProfilingEntriesGL", 1000);
 addDynamicEntryUInt("MaxProfilingEntriesAdvance", 200);
 addDynamicEntryUInt("MaxProfilingEntries", 1000);
 addDynamicEntryBool("UseGroundShaders", false);
 addDynamicEntryBool("UseUnitShaders", false);
 addDynamicEntryString("ShaderSuffixes", "-med,-low");
 addDynamicEntryInt("ShadowMapResolution", 2048);

 addDynamicEntry(new BoConfigUIntEntry(this, "GroundRenderer", 0)); // obsolete
 addDynamicEntry(new BoConfigUIntEntry(this, "DefaultLOD", 0));
 addDynamicEntry(new BoConfigBoolEntry(this, "EnableATIDepthWorkaround", false));
 addDynamicEntry(new BoConfigDoubleEntry(this, "ATIDepthWorkaroundValue", 0.00390625));
 addDynamicEntry(new BoConfigStringEntry(this, "GLFont", QString::null));
 addDynamicEntry(new BoConfigBoolEntry(this, "SmoothShading", true));
 addDynamicEntry(new BoConfigStringEntry(this, "MeshRenderer", "BoMeshRendererVertexArray"));
 addDynamicEntry(new BoConfigStringEntry(this, "GroundRendererClass", "BoDefaultGroundRenderer"));
 addDynamicEntry(new BoConfigStringEntry(this, "GameViewPlugin", "BosonGameViewDefault"));
 addDynamicEntry(new BoConfigBoolEntry(this, "EditorShowRandomMapGenerationWidget", false));
 addDynamicEntry(new BoConfigBoolEntry(this, "ShowUnitDebugWidget", false));
 addDynamicEntry(new BoConfigIntEntry(this, "GameSpeed", DEFAULT_GAME_SPEED));


 // the following are NOT stored into the config file
 addDynamicEntryBool("debug_fps", false, false);

 // sound is disabled by default, to make bounit and other non-sound applications work correctly, without calling setDisableSound(true) explicitly.
 addDynamicEntryBool("ForceDisableSound", true, false); // command line arg! do NOT save to config

 addDynamicEntryInt("DebugMode", (int)DebugNormal, false);
 addDynamicEntryDouble("AIDelay", 3.0, false);
 addDynamicEntryBool("ForceWantDirect", true, false); // command line arg! do NOT save to config
 addDynamicEntryBool("debug_wireframes", false, false);
 addDynamicEntryBool("debug_matrices", false, false);
 addDynamicEntryBool("debug_map_coordinates", false, false);
 addDynamicEntryBool("debug_pf_data", false, false);
 addDynamicEntryBool("debug_cell_grid", false, false);
 addDynamicEntryBool("debug_works", false, false);
 addDynamicEntryBool("debug_camera", false, false);
 addDynamicEntryBool("debug_rendercounts", false, false);
 addDynamicEntryBool("debug_boundingboxes", false, false);
 addDynamicEntryBool("debug_advance_calls", false, false);
 addDynamicEntryBool("debug_texture_memory", false, false);
 addDynamicEntryBool("debug_memory_usage", false, false);
 addDynamicEntryBool("debug_memory_vmdata_only", false, false);
 addDynamicEntryBool("debug_cpu_usage", false, false);
 addDynamicEntryBool("debug_groundrenderer_debug", false, false);
 addDynamicEntryBool("show_resources", true, false);
 addDynamicEntryBool("debug_profiling_graph", false, false);
 addDynamicEntryBool("debug_rendering_config", false, false);
 addDynamicEntryBool("debug_network_traffic", false, false);
 addDynamicEntryBool("debug_glfinish_before_profiling", false, false);
 addDynamicEntryBool("ForceDisableModelLoading", false, false); // command line arg! do NOT save to config
 addDynamicEntryBool("ForceDisableTextureCompression", false, false); // command line arg! do NOT save to config
 addDynamicEntryBool("TextureFOW", true, false);
 addDynamicEntryInt("DefaultLodCount", 5, false);
 addDynamicEntryBool("debug_render_ground", true, false);
 addDynamicEntryBool("debug_render_items", true, false);
 addDynamicEntryBool("debug_render_water", true, false);
 addDynamicEntryBool("debug_render_particles", true, false);

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
	if (it.current()->saveConfig()) {
		it.current()->load(conf);
	}
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
	if (it.current()->saveConfig()) {
		it.current()->save(conf);
	}
 }

 conf->setGroup(oldGroup);
}

void BosonConfig::setUnitSoundActivated(UnitSoundEvent e, bool activated)
{
 QValueList<int> l = intListValue("DeactivateUnitSounds");
 if (activated) {
	l.remove((int)e);
 } else {
	l.append((int)e);
 }
 setIntListValue("DeactivateUnitSounds", l);
}

bool BosonConfig::unitSoundActivated(UnitSoundEvent e) const
{
 return !intListValue("DeactivateUnitSounds").contains((int)e);
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

BoConfigBoolEntry* BosonConfig::addDynamicEntryBool(const QString& configKey, bool defaultValue, bool saveConfig)
{
 BoConfigBoolEntry* e = new BoConfigBoolEntry(this, configKey, defaultValue, saveConfig);
 addDynamicEntry(e);
 return e;
}

BoConfigIntEntry* BosonConfig::addDynamicEntryInt(const QString& configKey, int defaultValue, bool saveConfig)
{
 BoConfigIntEntry* e = new BoConfigIntEntry(this, configKey, defaultValue, saveConfig);
 addDynamicEntry(e);
 return e;
}

BoConfigUIntEntry* BosonConfig::addDynamicEntryUInt(const QString& configKey, unsigned int defaultValue, bool saveConfig)
{
 BoConfigUIntEntry* e = new BoConfigUIntEntry(this, configKey, defaultValue, saveConfig);
 addDynamicEntry(e);
 return e;
}

BoConfigDoubleEntry* BosonConfig::addDynamicEntryDouble(const QString& configKey, double defaultValue, bool saveConfig)
{
 BoConfigDoubleEntry* e = new BoConfigDoubleEntry(this, configKey, defaultValue, saveConfig);
 addDynamicEntry(e);
 return e;
}

BoConfigStringEntry* BosonConfig::addDynamicEntryString(const QString& configKey, QString defaultValue, bool saveConfig)
{
 BoConfigStringEntry* e = new BoConfigStringEntry(this, configKey, defaultValue, saveConfig);
 addDynamicEntry(e);
 return e;
}

BoConfigColorEntry* BosonConfig::addDynamicEntryColor(const QString& configKey, QColor defaultValue, bool saveConfig)
{
 BoConfigColorEntry* e = new BoConfigColorEntry(this, configKey, defaultValue, saveConfig);
 addDynamicEntry(e);
 return e;
}

BoConfigIntListEntry* BosonConfig::addDynamicEntryIntList(const QString& configKey, const QValueList<int>& defaultValue, bool saveConfig)
{
 BoConfigIntListEntry* e = new BoConfigIntListEntry(this, configKey, defaultValue, saveConfig);
 addDynamicEntry(e);
 return e;
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

void BosonConfig::setIntListValue(const QString& key, const QValueList<int>& v)
{
 BoConfigEntry* entry = value(key);
 BO_CHECK_NULL_RET(entry);
 if (entry->type() != BoConfigEntry::IntList) {
	boError() << k_funcinfo << key << "is not an IntList entry. type=" << entry->type() << endl;
	return;
 }
 ((BoConfigIntListEntry*)entry)->setValue(v);
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

QValueList<int> BosonConfig::intListValue(const QString& key) const
{
 return intListValue(key, QValueList<int>());
}

QValueList<int> BosonConfig::intListValue(const QString& key, const QValueList<int>& _default) const
{
 BoConfigEntry* entry = value(key);
 if (!entry) {
	boError() << k_funcinfo << "no key " << key << endl;
	return _default;
 }
 if (entry->type() != BoConfigEntry::IntList) {
	boError() << k_funcinfo << key << "is not a IntList entry. type=" << entry->type() << endl;
	return _default;
 }
 return ((BoConfigIntListEntry*)entry)->value();
}

bool BosonConfig::boolDefaultValue(const QString& key, bool _default) const
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
 return ((BoConfigBoolEntry*)entry)->defaultValue();
}

int BosonConfig::intDefaultValue(const QString& key, int _default) const
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
 return ((BoConfigIntEntry*)entry)->defaultValue();
}

unsigned int BosonConfig::uintDefaultValue(const QString& key, unsigned int _default) const
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
 return ((BoConfigUIntEntry*)entry)->defaultValue();
}

const QString&BosonConfig::stringDefaultValue(const QString& key, const QString& _default) const
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
 return ((BoConfigStringEntry*)entry)->defaultValue();
}

double BosonConfig::doubleDefaultValue(const QString& key, double _default) const
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
 return ((BoConfigDoubleEntry*)entry)->defaultValue();
}

QColor BosonConfig::colorDefaultValue(const QString& key) const
{
 return colorDefaultValue(key, Qt::black);
}

QColor BosonConfig::colorDefaultValue(const QString& key, const QColor& _default) const
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
 return ((BoConfigColorEntry*)entry)->defaultValue();
}

QValueList<int> BosonConfig::intListDefaultValue(const QString& key) const
{
 return intListDefaultValue(key, QValueList<int>());
}

QValueList<int> BosonConfig::intListDefaultValue(const QString& key, const QValueList<int>& _default) const
{
 BoConfigEntry* entry = value(key);
 if (!entry) {
	boError() << k_funcinfo << "no key " << key << endl;
	return _default;
 }
 if (entry->type() != BoConfigEntry::IntList) {
	boError() << k_funcinfo << key << "is not a IntList entry. type=" << entry->type() << endl;
	return _default;
 }
 return ((BoConfigIntListEntry*)entry)->defaultValue();
}

