/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOUFOGROUPBOX_H
#define BOUFOGROUPBOX_H

#include "boufowidget.h"

namespace ufo {
	class UGroupBox;
};

class BoUfoGroupBox : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString title READ title WRITE setTitle);
public:
	BoUfoGroupBox();
	BoUfoGroupBox(const QString& title);

	ufo::UGroupBox* groupBox() const
	{
		return mGroupBox;
	}

	void setTitle(const QString& text);
	QString title() const;

private:
	ufo::UGroupBox* mGroupBox;
};

#endif
