/*
    This file is part of the Boson game
    Copyright (C) 2001 The Boson Team (boson-devel@lists.sourceforge.net)

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
#ifndef __OPTIONSDIALOG_H__
#define __OPTIONSDIALOG_H__

#include <kdialogbase.h>

#include "global.h"

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

	void setCommandFramePosition(CommandFramePosition position);
	void setChatFramePosition(ChatFramePosition position);
	void setCursor(CursorMode mode);
	void setGroupMove(GroupMoveMode mode);

signals:
	void signalArrowScrollChanged(int);
	void signalSpeedChanged(int);

	/**
	 * @param index see @ref CommandFramePosition
	 **/
	void signalCommandFramePositionChanged(int index); 

	/**
	 * @param index see @ref ChatFramePosition
	 **/
	void signalChatFramePositionChanged(int index); 

	void signalCursorChanged(int index, const QString& cursorDir);
	void signalGroupMoveChanged(int index);

	void signalCmdBackgroundChanged(const QString& file);

protected slots:
	/**
	 * @param ms The new game speed in ms
	 **/
	void slotSpeedChanged(int ms);

	void slotCursorChanged(int index);
	void slotCursorThemeChanged(int index);

	void slotCmdBackgroundChanged(int index);

protected:
	void initGeneralPage();
	void initCursorPage();
	void initPathfindingPage();

private:
	class OptionsDialogPrivate;
	OptionsDialogPrivate* d;

};

#endif
