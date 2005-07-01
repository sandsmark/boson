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
#ifndef BPFDESCRIPTIONDIALOG_H
#define BPFDESCRIPTIONDIALOG_H

#include <kdialogbase.h>

class BPFDescription;

class BPFDescriptionDialogPrivate;

/**
 * @author Andreas Beckermann <b_mann@gmx.de
 * @short Configuration widget for the description.xml file of maps.
 **/
class BPFDescriptionDialog : public KDialogBase
{
	Q_OBJECT
public:
	BPFDescriptionDialog(QWidget* parent, bool modal);
	~BPFDescriptionDialog();

	/**
	 * Set the description file. @ref slotApply will save the values there.
	 **/
	void setDescription(BPFDescription* description);

protected slots:
	/**
	 * Close the dialog and call @ref slotApply
	 **/
	virtual void slotOk();

	/**
	 * Store the values to the description file, see @ref setDescription
	 **/
	virtual void slotApply();

private:
	void init();

private:
	BPFDescriptionDialogPrivate* d;
};

#endif
