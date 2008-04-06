/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include "boupgradeableproperty.h"

#include "../bomemory/bodummymemory.h"
#include <bodebug.h>
#include "upgradeproperties.h"
#include "speciestheme.h"

#include <qdom.h>
//Added by qt3to4:
#include <Q3ValueList>

class BaseValueEntryBase
{
public:
	BaseValueEntryBase()
	{
	}
	virtual ~BaseValueEntryBase()
	{
	}
	virtual quint32 toUInt() const = 0;
	virtual qint32 toInt() const = 0;
	virtual bofixed toBoFixed() const = 0;

	virtual void setValue(quint32 v) = 0;
	virtual void setValue(qint32 v) = 0;
	virtual void setValue(bofixed v) = 0;
};

template<class T> class BaseValueEntry : public BaseValueEntryBase
{
public:
	BaseValueEntry() : BaseValueEntryBase()
	{
	}
	BaseValueEntry(const BaseValueEntry& v) : BaseValueEntryBase()
	{
		*this = v;
	}
	BaseValueEntry(const T& v) : BaseValueEntryBase()
	{
		mValue = v;
	}
	const BaseValueEntry& operator=(const BaseValueEntry& e)
	{
		mValue = e.mValue;
		return *this;
	}
	const BaseValueEntry& operator=(const T& v)
	{
		mValue = v;
		return *this;
	}
	operator T() const
	{
		return mValue;
	}

	virtual quint32 toUInt() const { return (quint32)mValue; }
	virtual qint32 toInt() const { return (qint32)mValue; }
	virtual bofixed toBoFixed() const { return (bofixed)mValue; }

	virtual void setValue(quint32 v) { mValue = (T)v; }
	virtual void setValue(qint32 v)  { mValue = (T)v; }
	virtual void setValue(bofixed v)  { mValue = (T)v; }

private:
	T mValue;
};

class BoBaseValueCollectionPrivate
{
public:
	BoBaseValueCollectionPrivate()
	{
	}
	QMap< QString, QMap<QString, BaseValueEntryBase*> > mProperties;
};

BoBaseValueCollection::BoBaseValueCollection()
{
 d = new BoBaseValueCollectionPrivate;
}

BoBaseValueCollection::~BoBaseValueCollection()
{
 QMap< QString, QMap<QString, BaseValueEntryBase*> >::iterator it;
 for (it = d->mProperties.begin(); it != d->mProperties.end(); ++it) {
	QMap<QString, BaseValueEntryBase*>::iterator it2;
	for (it2 = (*it).begin(); it2 != (*it).end(); ++it2) {
		delete *it2;
	}
	(*it).clear();
 }
 delete d;
}

bool BoBaseValueCollection::insertULongBaseValue(quint32 v, const QString& name, const QString& type, bool replace)
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	BaseValueEntryBase* value = new BaseValueEntry<quint32>(v);
	(d->mProperties[type]).insert(name, value);
 }
 if (replace) {
	(d->mProperties[type])[name]->setValue(v);
 }
 return true;
}

bool BoBaseValueCollection::insertLongBaseValue(qint32 v, const QString& name, const QString& type, bool replace)
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	BaseValueEntryBase* value = new BaseValueEntry<qint32>(v);
	(d->mProperties[type]).insert(name, value);
 }
 if (replace) {
	(d->mProperties[type])[name]->setValue(v);
 }
 return true;
}

bool BoBaseValueCollection::insertBoFixedBaseValue(bofixed v, const QString& name, const QString& type, bool replace)
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	BaseValueEntryBase* value = new BaseValueEntry<bofixed>(v);
	(d->mProperties[type]).insert(name, value);
 }
 if (replace) {
	(d->mProperties[type])[name]->setValue(v);
 }
 return true;
}

bool BoBaseValueCollection::getBaseValue(quint32* ret, const QString& name, const QString& type) const
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	boError() << k_funcinfo << "no such property " << name << endl;
	return false;
 }
 *ret = (d->mProperties[type])[name]->toUInt();
 return true;
}

bool BoBaseValueCollection::getBaseValue(qint32* ret, const QString& name, const QString& type) const
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	boError() << k_funcinfo << "no such property " << name << endl;
	return false;
 }
 *ret = (d->mProperties[type])[name]->toInt();
 return true;
}

bool BoBaseValueCollection::getBaseValue(bofixed* ret, const QString& name, const QString& type) const
{
 if (type != "MaxValue" && type != "MinValue") {
	boError() << k_funcinfo << "invalid type " << type << endl;
	return false;
 }
 if (!(d->mProperties[type]).contains(name)) {
	boError() << k_funcinfo << "no such property " << name << endl;
	return false;
 }
 *ret = (d->mProperties[type])[name]->toBoFixed();
 return true;
}

quint32 BoBaseValueCollection::ulongBaseValue(const QString& name, const QString& type, quint32 defaultValue) const
{
 quint32 v = defaultValue;
 if (!getBaseValue(&v, name, type)) {
	return defaultValue;
 }
 return v;
}

qint32 BoBaseValueCollection::longBaseValue(const QString& name, const QString& type, qint32 defaultValue) const
{
 qint32 v = defaultValue;
 if (!getBaseValue(&v, name, type)) {
	return defaultValue;
 }
 return v;
}

bofixed BoBaseValueCollection::bofixedBaseValue(const QString& name, const QString& type, bofixed defaultValue) const
{
 bofixed v = defaultValue;
 if (!getBaseValue(&v, name, type)) {
	return defaultValue;
 }
 return v;
}




BoUpgradesCollection::BoUpgradesCollection()
{
 mUpgrades = new Q3ValueList<const UpgradeProperties*>();

 // all properties' counters are initialized with 0, so when we
 // set this counter to 1, all properties are initiall dirty
 mUpgradesCacheCounter = 1;
}

BoUpgradesCollection::~BoUpgradesCollection()
{
 delete mUpgrades;
}

void BoUpgradesCollection::addUpgrade(const UpgradeProperties* upgrade)
{
 BO_CHECK_NULL_RET(upgrade);
 mUpgrades->append(upgrade);

 // AB: the counter is increased whenever the upgrades of the player change in
 // any way. this causes all values that depend on upgrades to be recalculated
 // when required.
 mUpgradesCacheCounter++;
}

void BoUpgradesCollection::clearUpgrades()
{
 while (!upgrades()->isEmpty()) {
	removeUpgrade(upgrades()->first());
 }
}


const UpgradeProperties* BoUpgradesCollection::findUpgrade(quint32 id) const
{
 Q3ValueList<const UpgradeProperties*>::const_iterator it;
 for (it = upgrades()->begin(); it != upgrades()->end(); ++it) {
	if ((*it)->id() == id) {
		return *it;
	}
 }
 return 0;
}

bool BoUpgradesCollection::removeUpgrade(const UpgradeProperties* upgrade)
{
 if (!upgrade) {
	return false;
 }
 bool ret = false;
 if (mUpgrades->remove(upgrade) > 0) {
	ret = true;
	mUpgradesCacheCounter++;
 }

 return ret;
}

bool BoUpgradesCollection::saveAsXML(QDomElement& root) const
{
 if (root.isNull()) {
	return false;
 }
 QDomDocument doc = root.ownerDocument();
 Q3ValueList<const UpgradeProperties*>::const_iterator it;
 for (it = upgrades()->begin(); it != upgrades()->end(); ++it) {
	QDomElement e = doc.createElement("Upgrade");
	e.setAttribute("Id", QString::number((*it)->id()));
	e.setAttribute("Type", (*it)->type());
	root.appendChild(e);
 }
 return true;
}

bool BoUpgradesCollection::loadFromXML(const SpeciesTheme* speciesTheme, const QDomElement& root)
{
 clearUpgrades();
 if (!speciesTheme) {
	BO_NULL_ERROR(speciesTheme);
	return false;
 }
 if (root.isNull()) {
	return false;
 }
 QDomDocument doc = root.ownerDocument();
 for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Upgrade") {
		continue;
	}
	bool ok = false;
	quint32 id = e.attribute("Id").toUInt(&ok);
	if (!ok) {
		boError() << k_funcinfo << "invalid Id attribute" << endl;
		return false;
	}
	QString type = e.attribute("Type");
	if (type.isEmpty()) {
		boError() << k_funcinfo << "no Type given" << endl;
		return false;
	}

	const UpgradeProperties* upgrade = speciesTheme->upgrade(type, id);
	if (!upgrade) {
		boError() << k_funcinfo << "cannot find upgrade type=" << type << " id=" << id << endl;
		return false;
	}
	addUpgrade(upgrade);
 }
 return true;
}


bool BoUpgradeablePropertyBase::upgradeValue(const Q3ValueList<const UpgradeProperties*>* list, quint32* v) const
{
 Q3ValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}

bool BoUpgradeablePropertyBase::upgradeValue(const Q3ValueList<const UpgradeProperties*>* list, qint32* v) const
{
 Q3ValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}

bool BoUpgradeablePropertyBase::upgradeValue(const Q3ValueList<const UpgradeProperties*>* list, bofixed* v) const
{
 Q3ValueList<const UpgradeProperties*>::const_iterator it;
 for (it = list->begin(); it != list->end(); ++it) {
	if (!(*it)->upgradeValue(name(), v, type())) {
		boError() << k_funcinfo << "upgrade failed" << endl;
		return false;
	}
 }
 return true;
}


