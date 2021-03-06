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

#include "bosongameviewpluginbase.h"
#include "bosongameviewpluginbase.moc"

#include "../../bomemory/bodummymemory.h"

#include <bodebug.h>

class BosonGameViewPluginBasePrivate
{
public:
	BosonGameViewPluginBasePrivate()
	{
	}
	bool mInitialized;
};

BosonGameViewPluginBase::BosonGameViewPluginBase()
	: QObject(0, "gameviewplugin")
{
 d = new BosonGameViewPluginBasePrivate;
 d->mInitialized = false;
 mGameGLMatrices = 0;
 mUfoWidget = 0;
 mCanvas = 0;
}

BosonGameViewPluginBase::~BosonGameViewPluginBase()
{
 delete d;
}

void BosonGameViewPluginBase::init()
{
 if (d->mInitialized) {
	return;
 }
 mUfoWidget = createUfoWidget();
}

void BosonGameViewPluginBase::slotSelectionChanged(BoSelection* selection)
{
 mSelection = selection;
}

