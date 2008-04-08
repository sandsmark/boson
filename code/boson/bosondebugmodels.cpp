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

#include "bosondebugmodels.h"
#include "bosondebugmodels.moc"

#include "bodebug.h"
#include "qlistviewitemnumber.h"
//Added by qt3to4:
#include <Q3PtrList>
#include "modelrendering/bosonmodel.h"
#include "modelrendering/bomesh.h"
#include "speciesdata.h"

#include <klocale.h>

#include <q3listview.h>
#include <qlabel.h>


class BosonDebugModelsPrivate
{
public:
	BosonDebugModelsPrivate()
	{
	}
};

BosonDebugModels::BosonDebugModels(QWidget* parent)
	: QWidget(parent)
{
 d = new BosonDebugModelsPrivate();
}

BosonDebugModels::~BosonDebugModels()
{
 boDebug() << k_funcinfo << endl;
 delete d;
}

void BosonDebugModels::slotUpdate()
{
 mModelList->clear();
 long int pointArrayMemorySum = 0;
 long int indexArrayMemorySum = 0;

 Q3PtrList<BosonModel> models = SpeciesData::allLoadedModelsInAllSpecies();
 for (Q3PtrListIterator<BosonModel> it(models); it.current(); ++it) {
	QString file = it.current()->file();
	int pointArrayMemory = it.current()->pointArraySize() * BoMesh::pointSize() * sizeof(float);
	int indexArrayMemory = it.current()->indexArraySize();
	if (it.current()->indexArrayType() == GL_UNSIGNED_SHORT) {
		indexArrayMemory *= sizeof(unsigned short);
	} else {
		indexArrayMemory *= sizeof(unsigned int);
	}
	pointArrayMemorySum += pointArrayMemory;
	indexArrayMemorySum += indexArrayMemory;

	QListViewItemNumberPrefix* item = new QListViewItemNumberPrefix(mModelList);
	item->setText(0, file);
	item->setText(1, memoryString(pointArrayMemory));
	item->setText(2, memoryString(indexArrayMemory));
 }

 mModelCount->setText(i18n("%1", models.count()));
 mPointArrayMemorySum->setText(memoryString(pointArrayMemorySum));
 mIndexArrayMemorySum->setText(memoryString(indexArrayMemorySum));
}

QString BosonDebugModels::memoryString(long int bytes) const
{
 QString string;
 if (bytes < 0) {
	return string;
 }
 if (bytes < 1024) {
	string = i18n("%1 B", bytes);
 } else if (bytes < 1024 * 1024) {
	string = i18n("%1 KB", ((float)bytes) / 1024.0f);
 } else {
	string = i18n("%1 MB", ((float)bytes) / (1024.0f * 1024.0f));
 }
 return string;
}

