/*
    This file is part of the Boson game
    Copyright (C) 2004 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosoncommandframeinterface.h"
#include "bosoncommandframeinterface.moc"

BosonCommandFrameFactoryBase* BosonCommandFrameInterface::mFactory = 0;

BosonCommandFrameInterface::BosonCommandFrameInterface(QWidget* parent, const char* name)
	: QFrame(parent, name)
{
}

BosonCommandFrameInterface::~BosonCommandFrameInterface()
{
}

BosonCommandFrameInterface* BosonCommandFrameInterface::createCommandFrame(QWidget* parent, bool game)
{
 if (!mFactory) {
	return 0;
 }
 return mFactory->createCommandFrame(parent, game);
}

void BosonCommandFrameInterface::setFactory(BosonCommandFrameFactoryBase* f)
{
 delete mFactory;
 mFactory = f;
}

