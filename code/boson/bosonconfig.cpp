/*
    This file is part of the Boson game
    Copyright (C) 1999-2000 Thomas Capricelli (capricel@email.enst.fr)
    Copyright (C) 2001-2006 Andreas Beckermann (b_mann@gmx.de)

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
#include "bosonconfig.h"

#include "defines.h"
#include "../bomemory/bodummymemory.h"
#include "boglobal.h"
#include "bodebug.h"
#include "bo3dtools.h"
#include <bogl.h>

#include <q3ptrlist.h>
#include <q3dict.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <KConfig>
#include <KConfigGroup>
#include <kconfig.h>
#include <kapplication.h>
#include <klocale.h>

#include <stdlib.h>
#include <kglobal.h>

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
 if (parent) {
	parent->addConfigEntry(this);
 }
}

BoConfigEntry::~BoConfigEntry()
{
}

KConfigGroup BoConfigEntry::retrieveGroup(KConfig* conf)
{
 return conf->group("Boson");
}

BoConfigBoolEntry::BoConfigBoolEntry(BosonConfig* parent, const QString& key, bool defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
 mValue = defaultValue;
}

void BoConfigBoolEntry::save(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 group.writeEntry(key(), mValue);
}

void BoConfigBoolEntry::load(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 mValue = group.readEntry(key(), mValue);
}

BoConfigIntEntry::BoConfigIntEntry(BosonConfig* parent, const QString& key, int defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
 mValue = defaultValue;
}

void BoConfigIntEntry::save(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 group.writeEntry(key(), mValue);
}

void BoConfigIntEntry::load(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 mValue = group.readEntry(key(), mValue);
}

BoConfigUIntEntry::BoConfigUIntEntry(BosonConfig* parent, const QString& key, unsigned int defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
 mValue = defaultValue;
}

void BoConfigUIntEntry::save(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 group.writeEntry(key(), mValue);
}

void BoConfigUIntEntry::load(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 mValue = group.readEntry(key(), mValue);
}

BoConfigDoubleEntry::BoConfigDoubleEntry(BosonConfig* parent, const QString& key, double defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
 mValue = defaultValue;
}

void BoConfigDoubleEntry::save(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 group.writeEntry(key(), mValue);
}

void BoConfigDoubleEntry::load(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 mValue = group.readEntry(key(), mValue);
}

BoConfigStringEntry::BoConfigStringEntry(BosonConfig* parent, const QString& key, QString defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultValue = defaultValue;
 mValue = defaultValue;
}

void BoConfigStringEntry::save(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 group.writeEntry(key(), mValue);
}

void BoConfigStringEntry::load(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 mValue = group.readEntry(key(), mValue);
}

class BoConfigIntListEntry : public BoConfigEntry
{
public:
	BoConfigIntListEntry(BosonConfig* parent, const QString& key, Q3ValueList<int> defaultValue, bool saveConfig = true)
		: BoConfigEntry(parent, key, saveConfig)
	{
		mDefaultValue = defaultValue;
		mValue = defaultValue;
	}
	~BoConfigIntListEntry() {}

	virtual void load(KConfig* conf)
	{
		KConfigGroup group = retrieveGroup(conf);
		group.writeEntry(key(), mValue);
	}
	virtual void save(KConfig* conf)
	{
		KConfigGroup group = retrieveGroup(conf);
		mValue = group.readEntry(key(), mValue);
	}

	virtual int type() const { return IntList; }

	void setValue(QList<int> list) { mValue = list; }
	QList<int> value() const { return mValue; }
	QList<int> defaultValue() const { return mDefaultValue; }

	void append(int e)
	{
		if (!contains(e)) {
			mValue.append(e);
		}
	}
	void remove(int e) { mValue.removeAll(e); }
	bool contains(int e) { return mValue.contains(e); }

private:
	QList<int> mValue;
	QList<int> mDefaultValue;
};

BoConfigColorEntry::BoConfigColorEntry(BosonConfig* parent, const QString& key, const QColor& defaultValue, bool saveConfig)
		: BoConfigEntry(parent, key, saveConfig)
{
 mDefaultRGBValue = defaultValue.rgb();
 mRGBValue = defaultValue.rgb();
}

void BoConfigColorEntry::save(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 group.writeEntry(key(), value());
}

void BoConfigColorEntry::load(KConfig* conf)
{
 KConfigGroup group = retrieveGroup(conf);
 QColor def = value();
 mRGBValue = group.readEntry(key(), def).rgb();
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


class BosonConfigScriptPrivate
{
public:
	BosonConfigScriptPrivate()
	{
	}

	Q3PtrList<BoConfigEntry> mEntries;
};

BosonConfigScript::BosonConfigScript(const QString& name)
{
 d = new BosonConfigScriptPrivate;
 mName = name;
}

BosonConfigScript::~BosonConfigScript()
{
 d->mEntries.setAutoDelete(false);
 for (Q3PtrListIterator<BoConfigEntry> it(d->mEntries); it.current(); ++it) {
	delete it.current();
	++it;
 }
 d->mEntries.clear();
 delete d;
}

void BosonConfigScript::copyScript(const BosonConfigScript& script)
{
 for (Q3PtrListIterator<BoConfigEntry> it(script.d->mEntries); it.current(); ++it) {
	BoConfigEntry* s = it.current();
	switch ((BoConfigEntry::Type)s->type()) {

#define CASE(x) case BoConfigEntry::x: \
		{ \
			BoConfig##x##Entry* e; \
			e = new BoConfig##x##Entry(0, s->key(), ((BoConfig##x##Entry*)s)->value()); \
			addValue(e); \
			break; \
		}

		CASE(Bool);
		CASE(Int);
		CASE(UInt);
		CASE(Double);
		CASE(String);
		CASE(IntList);
		CASE(Color);

#undef CASE

	}
 }
}

void BosonConfigScript::execute(BosonConfig* config) const
{
 BO_CHECK_NULL_RET(config);
 for (Q3PtrListIterator<BoConfigEntry> it(d->mEntries); it.current(); ++it) {
	QString key = it.current()->key();
	BoConfigEntry* myValue = it.current();
	BoConfigEntry* value = config->value(key);
	if (!value) {
		boError() << k_funcinfo << "no config entry registered for key " << key << endl;
		continue;
	}
	apply(myValue, value);
 }
}

const BoConfigEntry* BosonConfigScript::valueForKey(const QString& key) const
{
 for (Q3PtrListIterator<BoConfigEntry> it(d->mEntries); it.current(); ++it) {
	if (it.current()->key() == key) {
		return it.current();
	}
 }
 return 0;
}

const BoConfigBoolEntry* BosonConfigScript::boolValue(const QString& key) const
{
 const BoConfigEntry* entry = valueForKey(key);
 if (!entry) {
	return 0;
 }
 if (entry->type() != BoConfigEntry::Bool) {
	return 0;
 }
 return (const BoConfigBoolEntry*)entry;
}

const BoConfigIntEntry* BosonConfigScript::intValue(const QString& key) const
{
 const BoConfigEntry* entry = valueForKey(key);
 if (!entry) {
	return 0;
 }
 if (entry->type() != BoConfigEntry::Int) {
	return 0;
 }
 return (const BoConfigIntEntry*)entry;
}

const BoConfigUIntEntry* BosonConfigScript::uintValue(const QString& key) const
{
 const BoConfigEntry* entry = valueForKey(key);
 if (!entry) {
	return 0;
 }
 if (entry->type() != BoConfigEntry::UInt) {
	return 0;
 }
 return (const BoConfigUIntEntry*)entry;
}

const BoConfigDoubleEntry* BosonConfigScript::doubleValue(const QString& key) const
{
 const BoConfigEntry* entry = valueForKey(key);
 if (!entry) {
	return 0;
 }
 if (entry->type() != BoConfigEntry::Double) {
	return 0;
 }
 return (const BoConfigDoubleEntry*)entry;
}

const BoConfigStringEntry* BosonConfigScript::stringValue(const QString& key) const
{
 const BoConfigEntry* entry = valueForKey(key);
 if (!entry) {
	return 0;
 }
 if (entry->type() != BoConfigEntry::String) {
	return 0;
 }
 return (const BoConfigStringEntry*)entry;
}

const BoConfigIntListEntry* BosonConfigScript::intListValue(const QString& key) const
{
 const BoConfigEntry* entry = valueForKey(key);
 if (!entry) {
	return 0;
 }
 if (entry->type() != BoConfigEntry::IntList) {
	return 0;
 }
 return (const BoConfigIntListEntry*)entry;
}

const BoConfigColorEntry* BosonConfigScript::colorValue(const QString& key) const
{
 const BoConfigEntry* entry = valueForKey(key);
 if (!entry) {
	return 0;
 }
 if (entry->type() != BoConfigEntry::Color) {
	return 0;
 }
 return (const BoConfigColorEntry*)entry;
}

void BosonConfigScript::addValue(BoConfigEntry* entry)
{
 BO_CHECK_NULL_RET(entry);
 if (entry->key().isEmpty()) {
	boError() << k_funcinfo << "cannot add config entry with empty key" << endl;
	delete entry;
	return;
 }
 if (valueForKey(entry->key())) {
	boError() << k_funcinfo << "already a value with key " << entry->key() << " added" << endl;
	delete entry;
	return;
 }
 d->mEntries.append(entry);
}

void BosonConfigScript::apply(BoConfigEntry* scriptValue, BoConfigEntry* configValue) const
{
 BO_CHECK_NULL_RET(scriptValue);
 BO_CHECK_NULL_RET(configValue);
 if (scriptValue->type() != configValue->type()) {
	boError() << k_funcinfo << "script type: " << scriptValue->type() << " does not match config type: " << configValue->type() << " for key " << scriptValue->key() << endl;
	return;
 }

 switch (scriptValue->type()) {
	case BoConfigEntry::Bool:
	{
		BoConfigBoolEntry* src = (BoConfigBoolEntry*)scriptValue;
		BoConfigBoolEntry* dest = (BoConfigBoolEntry*)configValue;
		dest->setValue(src->value());
		break;
	}
	case BoConfigEntry::Int:
	{
		BoConfigIntEntry* src = (BoConfigIntEntry*)scriptValue;
		BoConfigIntEntry* dest = (BoConfigIntEntry*)configValue;
		dest->setValue(src->value());
		break;
	}
	case BoConfigEntry::UInt:
	{
		BoConfigUIntEntry* src = (BoConfigUIntEntry*)scriptValue;
		BoConfigUIntEntry* dest = (BoConfigUIntEntry*)configValue;
		dest->setValue(src->value());
		break;
	}
	case BoConfigEntry::Double:
	{
		BoConfigDoubleEntry* src = (BoConfigDoubleEntry*)scriptValue;
		BoConfigDoubleEntry* dest = (BoConfigDoubleEntry*)configValue;
		dest->setValue(src->value());
		break;
	}
	case BoConfigEntry::String:
	{
		BoConfigStringEntry* src = (BoConfigStringEntry*)scriptValue;
		BoConfigStringEntry* dest = (BoConfigStringEntry*)configValue;
		dest->setValue(src->value());
		break;
	}
	case BoConfigEntry::IntList:
	{
		BoConfigIntListEntry* src = (BoConfigIntListEntry*)scriptValue;
		BoConfigIntListEntry* dest = (BoConfigIntListEntry*)configValue;
		dest->setValue(src->value());
		break;
	}
	case BoConfigEntry::Color:
	{
		BoConfigColorEntry* src = (BoConfigColorEntry*)scriptValue;
		BoConfigColorEntry* dest = (BoConfigColorEntry*)configValue;
		dest->setValue(src->value());
		break;
	}
	default:
	{
		boError() << k_funcinfo << "unhandled config entry type " << scriptValue->type() << endl;
		break;
	}
 }
}

void BosonConfigScript::addDefaultValueOf(const QString& key, BosonConfig* config)
{
 BO_CHECK_NULL_RET(config);
 BoConfigEntry* entry = config->value(key);
 if (!entry) {
	boError() << k_funcinfo << "no config entry with key " << key << " found" << endl;
	return;
 }
 switch (entry->type()) {
	case BoConfigEntry::Bool:
	{
		BoConfigBoolEntry* v = (BoConfigBoolEntry*)entry;
		addBoolValue(key, v->defaultValue());
		break;
	}
	case BoConfigEntry::Int:
	{
		BoConfigIntEntry* v = (BoConfigIntEntry*)entry;
		addIntValue(key, v->defaultValue());
		break;
	}
	case BoConfigEntry::UInt:
	{
		BoConfigUIntEntry* v = (BoConfigUIntEntry*)entry;
		addUIntValue(key, v->defaultValue());
		break;
	}
	case BoConfigEntry::Double:
	{
		BoConfigDoubleEntry* v = (BoConfigDoubleEntry*)entry;
		addDoubleValue(key, v->defaultValue());
		break;
	}
	case BoConfigEntry::String:
	{
		BoConfigStringEntry* v = (BoConfigStringEntry*)entry;
		addStringValue(key, v->defaultValue());
		break;
	}
	case BoConfigEntry::IntList:
	{
		BoConfigIntListEntry* v = (BoConfigIntListEntry*)entry;
		addIntListValue(key, v->defaultValue());
		break;
	}
	case BoConfigEntry::Color:
	{
		BoConfigColorEntry* v = (BoConfigColorEntry*)entry;
		addColorValue(key, v->defaultValue());
		break;
	}
	default:
	{
		boError() << k_funcinfo << "unhandled config entry type " << entry->type() << endl;
		break;
	}
 }
}

void BosonConfigScript::addBoolValue(const QString& key, bool v)
{
 addValue(new BoConfigBoolEntry(0, key, v, false));
}

void BosonConfigScript::addIntValue(const QString& key, int v)
{
 addValue(new BoConfigIntEntry(0, key, v, false));
}

void BosonConfigScript::addUIntValue(const QString& key, unsigned int v)
{
 addValue(new BoConfigUIntEntry(0, key, v, false));
}

void BosonConfigScript::addDoubleValue(const QString& key, double v)
{
 addValue(new BoConfigDoubleEntry(0, key, v, false));
}

void BosonConfigScript::addStringValue(const QString& key, const QString& v)
{
 addValue(new BoConfigStringEntry(0, key, v, false));
}

void BosonConfigScript::addIntListValue(const QString& key, const Q3ValueList<int>& v)
{
 addValue(new BoConfigIntListEntry(0, key, v, false));
}

void BosonConfigScript::addColorValue(const QString& key, const QColor& v)
{
 addValue(new BoConfigColorEntry(0, key, v, false));
}





class BosonConfig::BosonConfigPrivate
{
public:
	BosonConfigPrivate()
		: mDynamicEntries(Q3Dict<BoConfigEntry>(101))
	{
	}

	Q3PtrList<BoConfigEntry> mConfigEntries;
	Q3Dict<BoConfigEntry> mDynamicEntries; // added dynamically.
	Q3PtrList<BosonConfigScript> mConfigScripts;
};

BosonConfig::BosonConfig(KConfig* conf)
{
 d = new BosonConfigPrivate;
 d->mConfigEntries.setAutoDelete(true);
 // don't delete them - they are in mConfigEntries, too and will get deleted
 // there
 d->mDynamicEntries.setAutoDelete(false);

 initConfigEntries();

 // load from config
 reset(conf);

 initScripts();
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

BoVector3Float BosonConfig::readBoVector3FloatEntry(const KConfigGroup* cfg, const QString& key, const BoVector3Float& aDefault)
{
 Q3ValueList<float> list = BosonConfig::readFloatNumList(cfg, key);
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

BoVector3Float BosonConfig::readBoVector3FloatEntry(const KConfigGroup* cfg, const QString& key)
{
 return readBoVector3FloatEntry(cfg, key, BoVector3Float());
}

BoVector3Fixed BosonConfig::readBoVector3FixedEntry(const KConfigGroup* cfg, const QString& key, const BoVector3Fixed& aDefault)
{
 Q3ValueList<float> list = BosonConfig::readFloatNumList(cfg, key);
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

BoVector3Fixed BosonConfig::readBoVector3FixedEntry(const KConfigGroup* cfg, const QString& key)
{
 return readBoVector3FixedEntry(cfg, key, BoVector3Fixed());
}

BoVector4Float BosonConfig::readBoVector4FloatEntry(const KConfigGroup* cfg, const QString& key)
{
 return readBoVector4FloatEntry(cfg, key, BoVector4Float());
}

BoVector4Float BosonConfig::readBoVector4FloatEntry(const KConfigGroup* cfg, const QString& key, const BoVector4Float& aDefault)
{
 Q3ValueList<float> list = BosonConfig::readFloatNumList(cfg, key);
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

BoVector4Fixed BosonConfig::readBoVector4FixedEntry(const KConfigGroup* cfg, const QString& key)
{
 return readBoVector4FixedEntry(cfg, key, BoVector4Fixed());
}

BoVector4Fixed BosonConfig::readBoVector4FixedEntry(const KConfigGroup* cfg, const QString& key, const BoVector4Fixed& aDefault)
{
 Q3ValueList<float> list = BosonConfig::readFloatNumList(cfg, key);
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

void BosonConfig::writeEntry(KConfigGroup* cfg, const QString& key, const BoVector3Float& value)
{
  Q3ValueList<float> list;
  list.append(value[0]);
  list.append(value[1]);
  list.append(value[2]);
  BosonConfig::writeFloatNumList(list, cfg, key);
}

void BosonConfig::writeEntry(KConfigGroup* cfg, const QString& key, const BoVector3Fixed& value)
{
  Q3ValueList<float> list;
  list.append(value[0]);
  list.append(value[1]);
  list.append(value[2]);
  BosonConfig::writeFloatNumList(list, cfg, key);
}

void BosonConfig::writeEntry(KConfigGroup* cfg, const QString& key, const BoVector4Float& value)
{
  Q3ValueList<float> list;
  list.append(value[0]);
  list.append(value[1]);
  list.append(value[2]);
  list.append(value[3]);
  BosonConfig::writeFloatNumList(list, cfg, key);
}

void BosonConfig::writeEntry(KConfigGroup* cfg, const QString& key, const BoVector4Fixed& value)
{
  Q3ValueList<float> list;
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
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return getenv("LOGNAME");
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Boson");
 QString name = group.readEntry("LocalPlayer", QString::fromLatin1(getenv("LOGNAME")));
 return name;
}

void BosonConfig::saveLocalPlayerName(const QString& name, KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
 	globalConfig = KGlobal::config();
	conf = globalConfig.data();
}
 KConfigGroup group = conf->group("Boson");
 group.writeEntry("LocalPlayer", name);
}

QString BosonConfig::readComputerPlayerName(KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return "Computer";
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Boson");
 QString name = group.readEntry("ComputerPlayer", i18n("Computer"));
 return name;
}

void BosonConfig::saveComputerPlayerName(const QString& name, KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Boson");
 group.writeEntry("ComputerPlayer", name);
}

QColor BosonConfig::readLocalPlayerColor(KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return Qt::red;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Boson");
 QColor color = group.readEntry("LocalPlayerColor", QColor(Qt::red));
 return color;
}

void BosonConfig::saveLocalPlayerColor(const QColor& color, KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Boson");
 group.writeEntry("LocalPlayerColor", color);
}

QString BosonConfig::readLocalPlayerMap(KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return QString();
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Boson");
 QString name = group.readEntry("LocalPlayerMap", QString());
 return name;
}

void BosonConfig::saveLocalPlayerMap(const QString& id, KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Boson");
 group.writeEntry("LocalPlayerMap", id);
}

QString BosonConfig::readEditorMap(KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return QString();
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Editor");
 QString name = group.readEntry("EditorMap", QString());
 return name;
}

void BosonConfig::saveEditorMap(const QString& id, KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Boson");
 group.writeEntry("EditorMap", id);
}

bool BosonConfig::readEditorCreateNewMap(KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return true;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Editor");
 bool createnew = group.readEntry("CreateNewMap", true);
 return createnew;
}

void BosonConfig::saveEditorCreateNewMap(bool createnew, KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 KConfigGroup group = conf->group("Editor");
 group.writeEntry("CreateNewMap", createnew);
}

void BosonConfig::reset(KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 // the old group is already stored here so we don't have to re-set it in every
 // read function
 Q3PtrListIterator<BoConfigEntry> it(d->mConfigEntries);
 for (; it.current(); ++it) {
	if (it.current()->saveConfig()) {
		it.current()->load(conf);
	}
 }
}

void BosonConfig::save(bool /*editor*/, KConfig* conf)
{
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
 }
 Q3PtrListIterator<BoConfigEntry> it(d->mConfigEntries);
 for (; it.current(); ++it) {
	if (it.current()->saveConfig()) {
		it.current()->save(conf);
	}
 }
}

void BosonConfig::setUnitSoundActivated(UnitSoundEvent e, bool activated)
{
 Q3ValueList<int> l = intListValue("DeactivateUnitSounds");
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

// obsolete. use KConfigGroup::readEntry() directly!
QList<quint32> BosonConfig::readUnsignedLongNumList(const KConfigGroup* cfg, const QString key)
{
 return cfg->readEntry(key, QList<quint32>());
}

// obsolete. Use KConfigGroup::writeEntry() directly!
void BosonConfig::writeUnsignedLongNumList(KConfigGroup* cfg, const QString key, QList<quint32> list)
{
 cfg->writeEntry(key, list);
}

// obsolete. use KConfigGroup::readEntry() directly!
QList<float> BosonConfig::readFloatNumList(const KConfigGroup* cfg, const QString key)
{
 return cfg->readEntry(key, QList<float>());
}

// obsolete. Use KConfigGroup::writeEntry() directly!
void BosonConfig::writeFloatNumList(QList<float> list, KConfigGroup* cfg, const QString key)
{
 cfg->writeEntry(key, list);
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
 KSharedPtr<KSharedConfig> globalConfig;
 if (!conf) {
	if (!kapp) {
		return;
	}
	globalConfig = KGlobal::config();
	conf = globalConfig.data();
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

BoConfigIntListEntry* BosonConfig::addDynamicEntryIntList(const QString& configKey, const Q3ValueList<int>& defaultValue, bool saveConfig)
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

void BosonConfig::setIntListValue(const QString& key, const Q3ValueList<int>& v)
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

Q3ValueList<int> BosonConfig::intListValue(const QString& key) const
{
 return intListValue(key, Q3ValueList<int>());
}

Q3ValueList<int> BosonConfig::intListValue(const QString& key, const Q3ValueList<int>& _default) const
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

Q3ValueList<int> BosonConfig::intListDefaultValue(const QString& key) const
{
 return intListDefaultValue(key, Q3ValueList<int>());
}

Q3ValueList<int> BosonConfig::intListDefaultValue(const QString& key, const Q3ValueList<int>& _default) const
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

void BosonConfig::addConfigScript(BosonConfigScript* script)
{
 BO_CHECK_NULL_RET(script);
 if (script->name().isEmpty()) {
	boError() << k_funcinfo << "empty names are not allowed" << endl;
	delete script;
	return;
 }
 if (configScript(script->name())) {
	boError() << k_funcinfo << "script named " << script->name() << " already added" << endl;
	delete script;
	return;
 }
 d->mConfigScripts.append(script);
}

const BosonConfigScript* BosonConfig::configScript(const QString& name) const
{
 for (Q3PtrListIterator<BosonConfigScript> it(d->mConfigScripts); it.current(); ++it) {
	if (it.current()->name() == name) {
		return it.current();
	}
 }
 return 0;
}



