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

#include "boufointernalframe.h"
#include "boufointernalframe.moc"

#include "boufomanager.h"
#include <bodebug.h>

#include <math.h>

BoUfoInternalFrame::BoUfoInternalFrame(BoUfoManager* manager, const QString& title)
	: BoUfoWidget(new ufo::UInternalFrame())
{
 init();
 setTitle(title);

 manager->addFrame(this);

 setMouseEventsEnabled(true, true);
// setKeyEventsEnabled(true);
}

BoUfoInternalFrame::~BoUfoInternalFrame()
{
}

void BoUfoInternalFrame::init()
{
// TODO: provide a method setCentered() or so, that relocates the frame to the
// center of the screen. that's important for dialogs.
 setBounds(0, 0, 100, 100);

 mRootPane = frame()->getRootPane();
 mContentPane = frame()->getContentPane();
 mContentWidget = new BoUfoWidget(mContentPane);
 mContentWidget->setLayoutClass(BoUfoWidget::UVBoxLayout);
 if (rootPane() && contentWidget()) {
//	rootPane()->setOpaque(opaque);
//	contentWidget()->setOpaque(opaque);
 }
}

void BoUfoInternalFrame::setBounds(int x, int y, int w, int h)
{
 frame()->setBounds(x, y, w, h);
}

void BoUfoInternalFrame::setTitle(const QString& t)
{
 frame()->setTitle(t.latin1());
}

QString BoUfoInternalFrame::title() const
{
 return QString(frame()->getTitle().c_str());
}

void BoUfoInternalFrame::setResizable(bool r)
{
 frame()->setResizable(r);
}

bool BoUfoInternalFrame::resizable() const
{
 return frame()->isResizable();
}

void BoUfoInternalFrame::adjustSize()
{
 frame()->pack();
}


