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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// note the copyright above: this is LGPL!
#ifndef BOUFOLINEEDIT_H
#define BOUFOLINEEDIT_H

#include "boufowidget.h"

class BoUfoLineEdit : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(bool editable READ isEditable WRITE setEditable);
public:
	BoUfoLineEdit();

	ufo::ULineEdit* lineEdit() const
	{
		return mLineEdit;
	}

	void setText(const QString& text);
	QString text() const;

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

	void setEditable(bool);
	bool isEditable() const;

signals:
	void signalActivated();
	void signalActivated(const QString& text);

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);

signals:
	void signalTextChanged(const QString&);

private:
	ufo::ULineEdit* mLineEdit;
};

#endif
