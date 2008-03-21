/*
    This file is part of the Boson game
    Copyright (C) 2003-2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonaudio.h"

#include "boaudiocommand.h"
#include "bodebug.h"

#include <config.h>

#ifdef BOSON_USE_OPENAL
#include "openal/bosonaudioal.h"
#endif

bool BosonAudio::mCreated = false;

class BosonAudioPrivate
{
public:
	BosonAudioPrivate()
	{
	}


	bool mPlayMusic;
	bool mPlaySound;
};

BosonAudio* BosonAudio::create()
{
 if (!mCreated) {
	mCreated = true;
#ifdef BOSON_USE_OPENAL
	return BosonAudioAL::create();
#else
#warning OpenAL not available - sound disabled
	BosonAudio* a = new BosonAudio;
	a->setMusic(false);
	a->setSound(false);
	boWarning(200) << k_funcinfo << "boson was compiled without OpenAL support. sound disabled." << endl;
#endif
 }
 return 0;
}

BosonAudio::BosonAudio()
{
 d = new BosonAudioPrivate;
 d->mPlayMusic = true;
 d->mPlaySound = true;
}

BosonAudio::~BosonAudio()
{
 boDebug(200) << k_funcinfo << endl;
 delete d;
 mCreated = false;
}

bool BosonAudio::isNull() const
{
 return false;
}

void BosonAudio::setMusic(bool m)
{
 d->mPlayMusic = m;
}

void BosonAudio::setSound(bool s)
{
 d->mPlaySound = s;
}

bool BosonAudio::music() const
{
 if (isNull()) {
	return false;
 }
 return d->mPlayMusic;
}

bool BosonAudio::sound() const
{
 if (isNull()) {
	return false;
 }
 return d->mPlaySound;
}

void BosonAudio::executeCommand(BoAudioCommand* command)
{
 delete command;
}
