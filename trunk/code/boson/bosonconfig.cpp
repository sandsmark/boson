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

#include <kconfig.h>
#include <kapplication.h>
#include <kstaticdeleter.h>
#include <klocale.h>
#include <kdebug.h>

static KStaticDeleter<BosonConfig> sd;
BosonConfig* BosonConfig::mBosonConfig = 0;

class BosonConfig::BosonConfigPrivate
{
public:
	BosonConfigPrivate()
	{
	}

	bool mSound;
	bool mMusic;
	int mCommandButtonsPerRow;

	// don't save this to the config
	DebugMode mDebugMode;
};

BosonConfig::BosonConfig(KConfig* conf)
{
 d = new BosonConfigPrivate;
 
 // set the initial defaults
 d->mSound = true;
 d->mMusic = true;
 d->mCommandButtonsPerRow = 3;
 d->mDebugMode = DebugNormal;

 // load from config
 reset(conf);
}

BosonConfig::~BosonConfig()
{
 delete d;
}

void BosonConfig::initBosonConfig()
{
 if (mBosonConfig) {
	return;
 }
 sd.setObject(mBosonConfig, new BosonConfig);
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
 int speed = conf->readNumEntry("Speed", 1);
 conf->setGroup(oldGroup);
 return speed;
}

CommandFramePosition BosonConfig::readCommandFramePosition(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 int pos = conf->readNumEntry("CommandFramePosition", CmdFrameLeft);
 conf->setGroup(oldGroup);
 return (CommandFramePosition)pos;
}

void BosonConfig::saveCommandFramePosition(CommandFramePosition pos, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("CommandFramePosition", (int)pos);
 conf->setGroup(oldGroup);
}

CursorMode BosonConfig::readCursorMode(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 int mode = conf->readNumEntry("CursorMode", CursorSprite);
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

GroupMoveMode BosonConfig::readGroupMoveMode(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 int mode = conf->readNumEntry("GroupMoveMode", GroupMoveOld);
 conf->setGroup(oldGroup);
 return (GroupMoveMode)mode;
}

void BosonConfig::saveGroupMoveMode(GroupMoveMode mode, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("GroupMoveMode", (int)mode);
 conf->setGroup(oldGroup);
}

void BosonConfig::saveChatFramePosition(ChatFramePosition pos, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("ChatFramePosition", (int)pos);
 conf->setGroup(oldGroup);
}

ChatFramePosition BosonConfig::readChatFramePosition(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 int pos = conf->readNumEntry("ChatFramePosition", ChatFrameBottom);
 conf->setGroup(oldGroup);
 return (ChatFramePosition)pos;
}

void BosonConfig::saveSound(KConfig* conf)
{
 conf->setGroup("Boson");
 conf->writeEntry("Sound", sound());
}

bool BosonConfig::readSound(KConfig* conf)
{
 conf->setGroup("Boson");
 bool s = conf->readBoolEntry("Sound", sound());
 return s;
}

void BosonConfig::saveMusic(KConfig* conf)
{
 conf->setGroup("Boson");
 conf->writeEntry("Music", music());
}

bool BosonConfig::readMusic(KConfig* conf)
{
 conf->setGroup("Boson");
 bool m = conf->readBoolEntry("Music", music());
 return m;
}

void BosonConfig::setMusic(bool m)
{
 d->mMusic = m;
}

bool BosonConfig::music() const
{
 return d->mMusic;
}

void BosonConfig::setSound(bool s)
{
 d->mSound = s;
}

bool BosonConfig::sound() const
{
 return d->mSound;
}

int BosonConfig::readCommandButtonsPerRow(KConfig* conf)
{
 conf->setGroup("Boson");
 int b = conf->readNumEntry("CommandButtonsPerRow", commandButtonsPerRow());
 return b;
}

void BosonConfig::saveCommandButtonsPerRow(KConfig* conf)
{
 conf->setGroup("Boson");
 conf->writeEntry("CommandButtonsPerRow", commandButtonsPerRow());
}

void BosonConfig::setCommandButtonsPerRow(int b)
{
 d->mCommandButtonsPerRow = b;
}

int BosonConfig::commandButtonsPerRow() const
{
 return d->mCommandButtonsPerRow;
}

void BosonConfig::reset(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 // the old group is already stored here so we don't have to re-set it in every
 // read function
 setMusic(readMusic(conf));
 setSound(readSound(conf));
 setCommandButtonsPerRow(readCommandButtonsPerRow(conf));

 conf->setGroup(oldGroup);
}

void BosonConfig::save(bool editor, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 // the old group is already stored here so we don't have to re-set it in every
 // save function
 saveMusic(conf);
 saveSound(conf);
 saveCommandButtonsPerRow(conf);
 if (!editor) {
	// place configs here that should not be saved in editor mode
 }

 conf->setGroup(oldGroup);
}

void BosonConfig::setDebugMode(DebugMode m)
{
 d->mDebugMode = m;
}

BosonConfig::DebugMode BosonConfig::debugMode() const
{
 return d->mDebugMode;
}
