/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __OPTIONSDIALOG_H__
#define __OPTIONSDIALOG_H__

#include <kdialogbase.h>

class OptionsDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class OptionsDialog : public KDialogBase
{
	Q_OBJECT
public:
	OptionsDialog(QWidget* parent, bool modal = false);
	~OptionsDialog();

	/**
	 * Set the shown value for the game speed. Note that this value is the
	 * time between 2 @ref QCanvas::advance calls in ms while the dialog
	 * does not show anything in ms. The dialog values are just the
	 * opposite: higher values mean higher speed.
	 **/
	void setGameSpeed(int ms);

	void setArrowScrollSpeed(int);

protected slots:
	/**
	 * @param ms The new game speed in ms
	 **/
	void slotSpeedChanged(int ms);

signals:
	void signalArrowScrollChanged(int);
	void signalSpeedChanged(int);

private:
	OptionsDialogPrivate* d;

};

#endif
