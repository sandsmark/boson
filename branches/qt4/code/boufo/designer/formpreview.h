/*
    This file is part of the Boson game
    Copyright (C) 2004-2006 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef FORMPREVIEW_H
#define FORMPREVIEW_H

#include <qgl.h> // AB: _Q_GLWidget
#include <qdom.h>
#include <qmap.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QWheelEvent>

/**
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class FormPreview : public QGLWidget
{
	Q_OBJECT
public:
	FormPreview(const QGLFormat& format, QWidget*);
	~FormPreview();

	void setPlacementMode(bool m);

	void updateGUI(const QDomElement& root);

	BoUfoWidget* getWidgetAt(int x, int y);
	BoUfoWidget* getContainerWidgetAt(int x, int y);

	void setSelectedWidget(const QDomElement& widget);

	BoUfoManager* ufoManager() const
	{
		return mUfoManager;
	}

signals:
	void signalPlaceWidget(const QDomElement& parent);
	void signalSelectWidget(const QDomElement& widget);

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int, int);

	virtual bool eventFilter(QObject* o, QEvent* e);
	virtual void mouseMoveEvent(QMouseEvent*);
	virtual void mousePressEvent(QMouseEvent*);
	virtual void mouseReleaseEvent(QMouseEvent*);
	virtual void wheelEvent(QWheelEvent*);
	virtual void keyPressEvent(QKeyEvent* e);
	virtual void keyReleaseEvent(QKeyEvent* e);

	void updateGUI(const QDomElement& root, BoUfoWidget* parent);
	void selectWidgetUnderCursor();
	void selectWidget(BoUfoWidget* widget);

private:
	void addWidget(BoUfoWidget*, const QDomElement&);

private:
	BoUfoManager* mUfoManager;
	BoUfoWidget* mContentWidget;
	bool mPlacementMode;
	QMap<void*, QDomElement> mUfoWidget2Element;
	QMap<void*, BoUfoWidget*> mUfoWidget2Widget;

	QString mNameOfSelectedWidget;
};

#endif

