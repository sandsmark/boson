/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001-2004 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include "bosonwidgetbase.h"
#include "bosonwidgetbase.moc"

#include "bodisplaymanager.h"
#include "bodebug.h"

#include <qlayout.h>

BosonWidgetBase::BosonWidgetBase(QWidget* parent)
    : QWidget( parent, "BosonWidgetBase" )
{
 mDisplayManager = 0;
}

BosonWidgetBase::~BosonWidgetBase()
{
 if (displayManager()) {
	// we do NOT delete the display manager here
	setDisplayManager(0);
 }
}

void BosonWidgetBase::setDisplayManager(BoDisplayManager* displayManager)
{
 if (mDisplayManager) {
	mDisplayManager->hide();
	mDisplayManager->reparent(0, QPoint(0, 0)); // we do NOT own the display manager!
 }
 mDisplayManager = displayManager;
 if (mDisplayManager) {
	if (mDisplayManager->parent()) {
		boError() << k_funcinfo << "the displaymanager already has a parent - reparenting..." << endl;
	}
	mDisplayManager->reparent(this, QPoint(0, 0));
	mDisplayManager->show();
 }
}

void BosonWidgetBase::initGameMode()
{
 QVBoxLayout* topLayout = new QVBoxLayout(this);
 topLayout->addWidget(displayManager());
}

