/*
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

// note the copyright above: this is LGPL!

#include "bosonplayfieldview.h"
#include "bosonplayfieldview.moc"

#include <klocale.h>

BosonPlayFieldItem::BosonPlayFieldItem(const QString& playFieldIdentifier, BosonPlayFieldView* parent) : QListViewItem(parent)
{
 init(playFieldIdentifier);
}

BosonPlayFieldItem::BosonPlayFieldItem(const QString& playFieldIdentifier, BosonPlayFieldItem* parent) : QListViewItem(parent)
{
 init(playFieldIdentifier);
}

BosonPlayFieldItem::~BosonPlayFieldItem()
{
}

void BosonPlayFieldItem::init(const QString& playFieldIdentifier)
{
 mPlayFieldIdentifier = playFieldIdentifier;
}

void BosonPlayFieldItem::setPlayFieldName(const QString& text)
{
 setText(0, text);
}

BosonPlayFieldNewMapItem::BosonPlayFieldNewMapItem(BosonPlayFieldView* parent) : BosonPlayFieldItem(QString::null, parent)
{
 init();
}

BosonPlayFieldNewMapItem::BosonPlayFieldNewMapItem(BosonPlayFieldItem* parent) : BosonPlayFieldItem(QString::null, parent)
{
 init();
}

BosonPlayFieldNewMapItem::~BosonPlayFieldNewMapItem()
{
}

void BosonPlayFieldNewMapItem::init()
{
 setPlayFieldName(i18n("New Map"));
}



class BosonPlayFieldViewPrivate
{
public:
	BosonPlayFieldViewPrivate()
	{
	}
};

BosonPlayFieldView::BosonPlayFieldView(QWidget* parent, const char* name) : KListView(parent, name)
{
 d = new BosonPlayFieldViewPrivate;
 addColumn(i18n("Map"));

 // uic generates this code for our current listview:
// header()->setClickEnabled(false, header()->count() - 1);
// header()->setResizeEnabled(false, header()->count() - 1);
// setFullWidth(true);

}

BosonPlayFieldView::~BosonPlayFieldView()
{
 delete d;
}


