/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosondebugtextures.h"
#include "bosondebugtextures.moc"

#include "botexture.h"
#include "bodebug.h"

#include <klocale.h>

#include <qlistview.h>

class BosonDebugTexturesPrivate
{
public:
	BosonDebugTexturesPrivate()
	{
	}
};

BosonDebugTextures::BosonDebugTextures(QWidget* parent)
	: BosonDebugTexturesBase(parent, 0, Qt::WDestructiveClose)
{
 d = new BosonDebugTexturesPrivate();
}

BosonDebugTextures::~BosonDebugTextures()
{
 boDebug() << k_funcinfo << endl;
 delete d;
}

void BosonDebugTextures::slotUpdate()
{
 mTextureList->clear();
 if (!boTextureManager) {
	BO_NULL_ERROR(boTextureManager);
	return;
 }
 QPtrList<const BoTexture> textures = boTextureManager->allTextures();
 for (QPtrListIterator<const BoTexture> it(textures); it.current(); ++it) {
	QString file = it.current()->filePath();
	int approximateMemory = it.current()->memoryUsed();

	QString memory = i18n("%1 KB").arg(((float)approximateMemory) / 1024.0f);

	new QListViewItem(mTextureList, file, memory);
 }
}

