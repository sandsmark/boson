/*
    This file is part of the Boson game
    Copyright (C) 2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "bowelcomewidget.h"
#include "bowelcomewidget.moc"


BoWelcomeWidget::BoWelcomeWidget(QWidget* parent)
	: QWidget(parent)
{
 connect(mOptions, SIGNAL(clicked()), this, SIGNAL(signalPreferences()));
 connect(mLoadGame, SIGNAL(clicked()), this, SIGNAL(signalLoadGame()));
 connect(mStartSPGame, SIGNAL(clicked()), this, SIGNAL(signalNewSPGame()));
 connect(mStartMPGame, SIGNAL(clicked()), this, SIGNAL(signalNewMPGame()));
 connect(mStartEditor, SIGNAL(clicked()), this, SIGNAL(signalStartEditor()));
 connect(mQuit, SIGNAL(clicked()), this, SIGNAL(signalQuit()));
}

BoWelcomeWidget::~BoWelcomeWidget()
{
}

