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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bosondebugtextures.h"
#include "bosondebugtextures.moc"

#include "botexture.h"
#include "bodebug.h"
#include "qlistviewitemnumber.h"
//Added by qt3to4:
#include <Q3PtrList>

#include <klocale.h>

#include <q3listview.h>
#include <qlabel.h>


class BosonDebugTexturesPrivate
{
public:
	BosonDebugTexturesPrivate()
	{
	}
};

BosonDebugTextures::BosonDebugTextures(QWidget* parent)
	: QWidget(parent)
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
 long int memorySum = 0;
 Q3PtrList<const BoTexture> textures = boTextureManager->allTextures();
 for (Q3PtrListIterator<const BoTexture> it(textures); it.current(); ++it) {
	QString file = it.current()->filePath();
	int approximateMemory = it.current()->memoryUsed();
	memorySum += approximateMemory;

	QString memory = i18n("%1 KB", ((float)approximateMemory) / 1024.0f);

	QListViewItemNumberPrefix* item = new QListViewItemNumberPrefix(mTextureList);
	item->setText(0, file);
	item->setText(1, memory);
 }
 mTextureCount->setText(i18n("%1", textures.count()));

 QString sum;
 if (memorySum < 1024) {
	sum = i18n("%1 B", memorySum);
 } else if (memorySum < 1024 * 1024) {
	sum = i18n("%1 KB", ((float)memorySum) / 1024.0f);
 } else {
	sum = i18n("%1 MB", ((float)memorySum) / (1024.0f * 1024.0f));
 }
 mTextureMemorySum->setText(sum);
}

