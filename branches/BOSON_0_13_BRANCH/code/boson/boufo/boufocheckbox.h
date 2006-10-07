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

// note the copyright above: this is LGPL!
#ifndef BOUFOCHECKBOX_H
#define BOUFOCHECKBOX_H

#include "boufowidget.h"

class BoUfoCheckBox : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(bool checked READ checked WRITE setChecked);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoCheckBox();
	BoUfoCheckBox(const QString& text, bool checked = false);

	ufo::UCheckBox* checkBox() const
	{
		return mCheckBox;
	}

	void setText(const QString& text);
	QString text() const;
	void setChecked(bool);
	bool checked() const;

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

signals:
	void signalActivated();
	void signalHighlighted();
	void signalToggled(bool);

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);

private:
	ufo::UCheckBox* mCheckBox;
};

#endif
