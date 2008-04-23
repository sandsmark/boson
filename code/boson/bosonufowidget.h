/*
    This file is part of the Boson game
    Copyright (C) 2004-2008 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOSONUFOWIDGET_H
#define BOSONUFOWIDGET_H

#include <bogl.h>
#include <QWidget>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QWheelEvent>

class BoUfoManager;

/**
 * @short A @ref QWidget with support for @ref BoUfoManager
 *
 * This widget is just a @ref QWidget that provides one @ref BoUfoManager
 * object. You must call @ref initUfo in order to
 * use it.
 *
 * For convenience all event method have been implemented to call the proper
 * even method in the @ref BoUfoManager.
 *
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BosonUfoWidget : public QWidget
{
	Q_OBJECT
public:
	BosonUfoWidget(QWidget* parent = 0);
	~BosonUfoWidget();

	void initUfo();

	BoUfoManager* ufoManager() const { return mUfoManager; }

	void resizeGL(int, int);
	void makeContextCurrent();

	/**
	 * If enabled, the events to this widget (see @ref mouseMoveEvent, @ref
	 * mousePressEvent, ...) are sent to the @ref ufoManager
	 * automatically.
	 *
	 * If @p send is FALSE, this feature is disabled.
	 *
	 * By default events are sent , i.e. @p send is TRUE.
	 **/
	void setSendEventsToUfo(bool send)
	{
		mSendEvents = send;
	}

protected:
	virtual bool eventFilter(QObject* o, QEvent* e);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void wheelEvent(QWheelEvent*);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyReleaseEvent(QKeyEvent* e);

private:
	BoUfoManager* mUfoManager;
	bool mSendEvents;
};


#endif

