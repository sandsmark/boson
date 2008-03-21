/*
    This file is part of the Boson game
    Copyright (C) 2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <qdialog.h>

class OptionsDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class OptionsDialog : public QDialog
{
	Q_OBJECT
public:
	OptionsDialog(QWidget* parent = 0);
	~OptionsDialog();

signals:
	/**
	 * Emitted after a click on apply. At this point the relevant options
	 * have been saved to @ref QSettings already. It is recommended to read
	 * them from there.
	 **/
	void signalApplyOptions();

protected slots:
	void slotBrowseDataDir();

	void slotApply();
	void slotOk();

private:
	OptionsDialogPrivate* d;

};

#endif
