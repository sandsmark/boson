/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef BOTEXMAPIMPORTDIALOG_H
#define BOTEXMAPIMPORTDIALOG_H

#include <kdialogbase.h>

class BosonMap;

class BoTexMapImportDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoTexMapImportDialog : public KDialogBase
{
	Q_OBJECT
public:
	BoTexMapImportDialog(QWidget* parent, bool modal = false);
	~BoTexMapImportDialog();

	void setMap(BosonMap* map);

	/**
	 * @return TRUE if @ref slotApply would have no effect at all, e.g. no
	 * texmap chosen or all colors should be ignored.
	 **/
	bool unchanged() const;

public slots:
	void slotSelectTexMapImage();

protected slots:
	virtual void slotOk();
	virtual void slotApply();

protected:
private:
	BoTexMapImportDialogPrivate* d;

};

#endif
