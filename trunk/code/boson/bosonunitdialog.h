/*
    This file is part of the Boson game
    Copyright (C) 1999-2000,2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __BOSONUNITDIALOG_H__
#define __BOSONUNITDIALOG_H__

#include <kdialogbase.h>

class BosonUnitDialogPrivate;
class BosonUnitDialog : public KDialogBase
{
	Q_OBJECT
public:
	BosonUnitDialog(QWidget* parent = 0);
	~BosonUnitDialog();

protected slots:
	void slotChangeUnitDir();
	void slotCreateUnit();
	void slotTypeChanged(int);

protected:
	void loadConfig(const QString& file);

private:
	void initDirectories();
	void initProperties();

private:
	BosonUnitDialogPrivate* d;
};

#endif
