/*
    This file is part of the Boson game
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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

#include "kgamecelldebug.h"
#include "kgamecelldebug.moc"

#include "../bomemory/bodummymemory.h"
#include "bosonmap.h"
#include "cell.h"
#include "unit.h"
#include "bodebug.h"

#include <klocale.h>
#include <klistview.h>

#include <qptrdict.h>
#include <qlayout.h>
#include <qpushbutton.h>

class KGameCellDebug::KGameCellDebugPrivate
{
public:
	KGameCellDebugPrivate()
	{
		mMap = 0;
	}

	BosonMap* mMap;
	KListView* mCellList;
	int mCellXId;
	int mCellYId;
	int mCellGroundTypeId;
	int mCellVersionId;
	int mCellTileId;
	int mCellUnitCountId;

	KListView* mCellView;

	QPtrDict<QListViewItem> mCells;
};

KGameCellDebug::KGameCellDebug(QWidget* parent) : QWidget(parent)
{
 d = new KGameCellDebugPrivate;
 QVBoxLayout* topLayout = new QVBoxLayout(this);

 QHBoxLayout* mainLayout = new QHBoxLayout(topLayout);

 d->mCellList = new KListView(this);
 mainLayout->addWidget(d->mCellList);
 connect(d->mCellList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotUpdateCell(QListViewItem*)));
 d->mCellXId = d->mCellList->addColumn(i18n("X"));
 d->mCellYId = d->mCellList->addColumn(i18n("Y"));
 d->mCellGroundTypeId = d->mCellList->addColumn(i18n("Ground"));
 d->mCellVersionId = d->mCellList->addColumn(i18n("Version"));
 d->mCellTileId = d->mCellList->addColumn(i18n("Tile"));
 d->mCellUnitCountId = d->mCellList->addColumn(i18n("Units"));

 d->mCellView = new KListView(this);
 mainLayout->addWidget(d->mCellView);
 d->mCellView->addColumn(i18n("Unit Id"));

 QPushButton* update = new QPushButton(this);
 topLayout->addWidget(update);
 update->setText(i18n("Update"));
 connect(update, SIGNAL(clicked()), this, SLOT(slotUpdate()));
}

KGameCellDebug::~KGameCellDebug()
{
 d->mCellView->clear();
 d->mCellList->clear();
 d->mCells.clear();
 delete d;
}

void KGameCellDebug::setMap(BosonMap* m)
{
 d->mMap = m;
 for (unsigned int i = 0; i < d->mMap->width(); i++) {
	for (unsigned int j = 0; j < d->mMap->height(); j++) {
		Cell* c = d->mMap->cell(i, j);
		if (!c) {
			boError() << k_funcinfo << "NULL cell" << endl;
			continue;
		}
		QListViewItem* item = new QListViewItem(d->mCellList);
		item->setText(d->mCellXId, QString::number(i));
		item->setText(d->mCellYId, QString::number(j));
#if 0
		item->setText(d->mCellGroundTypeId, QString::number(c->groundType()));
		item->setText(d->mCellVersionId, QString::number(c->version()));
		item->setText(d->mCellTileId, QString::number(c->tile()));
#else
		item->setText(d->mCellTileId, i18n("TileId is obsolete"));
		item->setText(d->mCellGroundTypeId, i18n("GroundType is obsolete"));
		item->setText(d->mCellVersionId, i18n("version is obsolete"));
#endif
		d->mCells.insert(c, item);
	}
 }
 slotUpdate();
}

void KGameCellDebug::slotUpdate()
{
 if (!d->mMap) {
	return;
 }
 for (unsigned int i = 0; i < d->mMap->width(); i++) {
	for (unsigned int j = 0; j < d->mMap->height(); j++) {
		Cell* c = d->mMap->cell(i, j);
		if (!c) {
			boError() << k_funcinfo << "NULL cell" << endl;
			continue;
		}
		QListViewItem* item = d->mCells[c];
		if (!item) {
			boError() << k_funcinfo << "NULL list item" << endl;
			return;
		}
		item->setText(d->mCellUnitCountId, QString::number(c->unitCount()));
	}
 }
 // update the selected cell
 slotUpdateCell(d->mCellList->selectedItem());
}

void KGameCellDebug::slotUpdateCell(QListViewItem* item)
{
 d->mCellView->clear();
 if (!item) {
	return;
 }
 Cell* c = 0;
 {
	QPtrDictIterator<QListViewItem> it(d->mCells);
	while (it.current() && !c) {
		if (it.current() == item) {
			c = (Cell*)it.currentKey();
		}
		++it;
	}
 }
 if (!c) {
	boError() << k_funcinfo << "Could not find the cell" << endl;
	return;
 }
 const BoItemList* list = c->items();
 BoItemList::ConstIterator it = list->begin();
 for (; it != list->end(); ++it) {
	QListViewItem* i = new QListViewItem(d->mCellView);
	if (RTTI::isUnit((*it)->rtti())) {
		i->setText(0, QString::number(((Unit*)*it)->id()));
	} else {
		i->setText(0, i18n("No unit - rtti=%1").arg((*it)->rtti()));
	}
 }
 
}


