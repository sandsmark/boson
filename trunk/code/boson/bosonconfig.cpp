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
#include <klocale.h>
#include <kdebug.h>

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
};

BosonConfig::BosonConfig(KConfig* conf)
{
 d = new BosonConfigPrivate;
 
 // set the initial defaults
 d->mSound = true;
 d->mMusic = true;
 d->mCommandButtonsPerRow = 3;

 // load from config
 reset(conf);
}

BosonConfig::~BosonConfig()
{
 delete d;
}

void BosonConfig::initBosonConfig()
{
 mBosonConfig = new BosonConfig;
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

int BosonConfig::readCommandFramePosition(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 int pos = conf->readNumEntry("CommandFramePosition", Qt::DockLeft);
 conf->setGroup(oldGroup);
 return pos;
}

void BosonConfig::saveCommandFramePosition(int pos, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("CommandFramePosition", pos);
 conf->setGroup(oldGroup);
}

void BosonConfig::saveChatFramePosition(int pos, KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 conf->writeEntry("ChatFramePosition", pos);
 conf->setGroup(oldGroup);
}

int BosonConfig::readChatFramePosition(KConfig* conf)
{
 if (!conf) {
	conf = kapp->config();
 }
 QString oldGroup = conf->group();
 conf->setGroup("Boson");
 int pos = conf->readNumEntry("ChatFramePosition", 0);
 conf->setGroup(oldGroup);
 return pos;
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


