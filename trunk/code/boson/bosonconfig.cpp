/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <kconfig.h>
#include <kapplication.h>
#include <kstaticdeleter.h>
#include <klocale.h>
#include <kdebug.h>

#include <GL/gl.h>

static KStaticDeleter<BosonConfig> sd;
BosonConfig* BosonConfig::mBosonConfig = 0;

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



class BosonConfig::BosonConfigPrivate
{
public:
	BosonConfigPrivate()
	{
	}

	QPtrList<BoConfigEntry> mConfigEntries;
};

BosonConfig::BosonConfig(KConfig* conf)
{
 d = new BosonConfigPrivate;
 d->mConfigEntries.setAutoDelete(true);

 mSound = new BoConfigBoolEntry(this, "Sound", DEFAULT_SOUND);
 mMusic = new BoConfigBoolEntry(this, "Music", DEFAULT_MUSIC);
 mMMBMove = new BoConfigBoolEntry(this, "MMBMove", DEFAULT_USE_MMB_MOVE);
 mRMBMove = new BoConfigBoolEntry(this, "RMBMove", DEFAULT_USE_RMB_MOVE);
 mShowMenubarInGame = new BoConfigBoolEntry(this, "ShowMenubarInGame", true);
 mShowMenubarOnStartup = new BoConfigBoolEntry(this, "ShowMenubarOnStartup", false);
 mCommandButtonsPerRow = new BoConfigIntEntry(this, "CommandButtonsPerRow", DEFAULT_CMD_BUTTONS_PER_ROW);
 mArrowKeyStep = new BoConfigUIntEntry(this, "ArrowKeyStep", DEFAULT_ARROW_SCROLL_SPEED);
 mCursorEdgeSensity = new BoConfigUIntEntry(this, "CursorEdgeSensity", DEFAULT_CURSOR_EDGE_SENSITY);
 mUpdateInterval = new BoConfigUIntEntry(this, "GLUpdateInterval", DEFAULT_UPDATE_INTERVAL);
 mMiniMapScale = new BoConfigDoubleEntry(this, "MiniMapScale", DEFAULT_MINIMAP_SCALE);
 mMiniMapZoom = new BoConfigDoubleEntry(this, "MiniMapZoom", DEFAULT_MINIMAP_ZOOM);
 mChatScreenRemoveTime = new BoConfigUIntEntry(this, "ChatScreenRemoveTime", DEFAULT_CHAT_SCREEN_REMOVE_TIME);
 mChatScreenMaxItems = new BoConfigIntEntry(this, "ChatScreenMaxItems", DEFAULT_CHAT_SCREEN_MAX_ITEMS);
 mModelTexturesMipmaps = new BoConfigBoolEntry(this, "ModelTexturesMipmaps", true);
 mUnitSoundsDeactivated = new BoConfigIntListEntry(this, "DeactivateUnitSounds", QValueList<int>());
 mMagnificationFilter = new BoConfigIntEntry(this, "MagnificationFilter", DEFAULT_MAGNIFICATION_FILTER);
 mMinificationFilter = new BoConfigIntEntry(this, "MinificationFilter", DEFAULT_MINIFICATION_FILTER);
 mMipmapMinificationFilter = new BoConfigIntEntry(this, "MipmapMinificationFilter", DEFAULT_MIPMAP_MINIFICATION_FILTER);

 mDebugMode = DebugNormal;

 mDisableSound = false;

 // load from config
 reset(conf);
}

BosonConfig::~BosonConfig()
{
 d->mConfigEntries.clear();
 delete d;
}

void BosonConfig::initBosonConfig()
{
 if (mBosonConfig) {
	return;
 }
 sd.setObject(mBosonConfig, new BosonConfig);
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
 QString name = conf->readEntry("LocalPlayer", i18n("You"));
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

CursorMode BosonConfig::readCursorMode(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 int mode = conf->readNumEntry("CursorMode", DEFAULT_CURSOR);
 conf->setGroup(oldGroup);
 return (CursorMode)mode;
}

void BosonConfig::saveCursorMode(CursorMode mode, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("CursorMode", (int)mode);
 conf->setGroup(oldGroup);
}

QString BosonConfig::readCursorDir(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 QString dir = conf->readEntry("CursorDir", QString::null); // QString::null causes slotChangeCursor() to use BosonCursor::defaultTheme
 conf->setGroup(oldGroup);
 return dir;
}

void BosonConfig::saveCursorDir(const QString& dir, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("CursorDir", dir);
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

void BosonConfig::setDebugMode(DebugMode m)
{
 mDebugMode = m;
}

BosonConfig::DebugMode BosonConfig::debugMode() const
{
 return mDebugMode;
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

