/*
    This file is part of the Boson game
    Copyright (C) 2001-2005 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <kdialogbase.h>

class OptionsWidget;
class BoFontInfo;

class OptionsDialogPrivate;
/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class OptionsDialog : public KDialogBase
{
	Q_OBJECT
public:
	OptionsDialog(QWidget* parent = 0);
	~OptionsDialog();

signals:
	void signalCursorChanged(int index, const QString& cursorDir);
	void signalFontChanged(const BoFontInfo&);

	/**
	 * Emitted when apply or ok is clicked. At this point all options
	 * widgets already saved themselves and you are meant to activate the
	 * settings now, if necessary. I.e. if a timer interval has changed you
	 * are meant to restart the timer when this is emitted.
	 *
	 * Values that depend on @ref BosonConfig anyway (e.g. a timer that uses
	 * @ref QTimer::singleShot with the current value from @ref BosonConfig)
	 * do not need to be touched here.
	 **/
	void signalApply();

public slots:
	void slotLoad();

protected slots:
	virtual void slotOk();
	virtual void slotApply();
	virtual void slotDefault();


protected:
	void initGeneralPage();
	void initCursorPage();
	void initScrollingPage();
	void initSoundsPage();
	void initOpenGLPage();
	void initWaterPage();
	void initChatPage();
	void initToolTipPage();

	void addOptions(OptionsWidget* o);

private:
	OptionsDialogPrivate* d;

};

#endif
