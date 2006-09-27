/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufolayeredpane.h"
#include "boufolayeredpane.moc"

#include <bodebug.h>

BoUfoLayeredPane::BoUfoLayeredPane()
	: BoUfoWidget(new ufo::ULayeredPane())
{
 setLayoutClass(BoUfoWidget::UFullLayout);
}

BoUfoLayeredPane::BoUfoLayeredPane(ufo::ULayeredPane* p, bool provideLayout)
	: BoUfoWidget(p)
{
 if (provideLayout) {
	setLayoutClass(BoUfoWidget::UFullLayout);
 }
}

BoUfoLayeredPane::~BoUfoLayeredPane()
{
}

void BoUfoLayeredPane::addLayer(BoUfoWidget* w, int layer, int pos)
{
 BO_CHECK_NULL_RET(w);
 BO_CHECK_NULL_RET(ufoWidget());
 addWidget(w);
 setLayer(w, layer, pos);
}

void BoUfoLayeredPane::setLayer(BoUfoWidget* w, int layer, int pos)
{
 BO_CHECK_NULL_RET(w);
 BO_CHECK_NULL_RET(ufoWidget());
 if (!dynamic_cast<ufo::ULayeredPane*>(ufoWidget())) {
	boError() << k_funcinfo << "oops - not a ULayeredPane" << endl;
	return;
 }
 dynamic_cast<ufo::ULayeredPane*>(ufoWidget())->setLayer(w->ufoWidget(), layer, pos);
}



