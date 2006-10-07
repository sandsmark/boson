/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

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

#include <ufo/ufo.hpp>
#include <ufo/ux/ux.hpp>
#include "../boufo.h"
#include <bogl.h>

#include "formpreview.h"
#include "formpreview.moc"

#include <bodebug.h>

#include <qtimer.h>
#include <qcursor.h>

#include <math.h>
#include <stdlib.h>

// TODO: provide this information in the BoUfoFactory!
#warning FIXME: code duplication
static bool isContainerWidget(const QString& className)
{
 if (className.isEmpty()) {
	return false;
 }
 if (className == "BoUfoWidget") {
	return true;
 }
 if (className == "BoUfoHBox") {
	return true;
 }
 if (className == "BoUfoVBox") {
	return true;
 }
 if (className == "BoUfoWidgetStack") {
	return true;
 }
 if (className == "BoUfoLayeredPane") {
	return true;
 }
 if (className == "BoUfoButtonGroupWidget") {
	return true;
 }
 if (className == "BoUfoGroupBox") {
	return true;
 }
 return false;
}



FormPreview::FormPreview(const QGLFormat& format, QWidget* parent)
	: QGLWidget(format, parent)
{
// qApp->setGlobalMouseTracking(true);
// qApp->installEventFilter(this);
 setMouseTracking(true);
// setFocusPolicy(StrongFocus);

 setMinimumSize(200, 200);

 mUfoManager = 0;
 mPlacementMode = false;

 QTimer* updateTimer = new QTimer(this);
 connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
 updateTimer->start(40);
 setUpdatesEnabled(false);
}

FormPreview::~FormPreview()
{
 boDebug() << k_funcinfo << endl;
 delete mUfoManager;
 boDebug() << k_funcinfo << "done" << endl;
// qApp->setGlobalMouseTracking(false);
}

BoUfoWidget* FormPreview::getWidgetAt(int x, int y)
{
 ufo::UWidget* widget = 0;
 widget = mUfoManager->contentWidget()->ufoWidget()->getWidgetAt(x, y);
 if (!widget) {
	return 0;
 }
 BoUfoWidget* w = mUfoWidget2Widget[widget];

 while (!w && widget) {
	widget = widget->getParent();
	w = mUfoWidget2Widget[widget];
 }

 return w;
}

BoUfoWidget* FormPreview::getContainerWidgetAt(int x, int y)
{
 BoUfoWidget* w = getWidgetAt(x, y);
 if (!w) {
	return w;
 }
 QDomElement e = mUfoWidget2Element[w->ufoWidget()];
 QString className;
 if (!e.isNull()) {
	className = e.namedItem("ClassName").toElement().text();
 }
 while (w && !isContainerWidget(className)) {
	ufo::UWidget* widget = w->ufoWidget();
	w = 0;
	while (!w && widget) {
		widget = widget->getParent();
		w = mUfoWidget2Widget[widget];
	}
	if (!w) {
		return 0;
	}
	e = mUfoWidget2Element[w->ufoWidget()];
	if (e.isNull()) {
		boError() << k_funcinfo << "NULL element for BoUfoWidget() !" << endl;
		return 0;
	}
	className = e.namedItem("ClassName").toElement().text();
 }
 return w;
}

void FormPreview::updateGUI(const QDomElement& root)
{
 glInit();
 BO_CHECK_NULL_RET(mUfoManager);
 makeCurrent();
 mUfoManager->contentWidget()->ufoWidget()->removeAll();
 mContentWidget->loadPropertiesFromXML(root.namedItem("Properties").toElement());

 mUfoWidget2Element.clear();
 mUfoWidget2Widget.clear();
 addWidget(mContentWidget, root);
 updateGUI(root, mContentWidget);
}

void FormPreview::updateGUI(const QDomElement& root, BoUfoWidget* parent)
{
 boDebug() << k_funcinfo << endl;
 if (root.isNull() || !parent) {
	return;
 }
 bool useConvenienceBackground = false;
 QDomNode n;
 static int depth = 0;
 bool useRed = true;
 for (n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
	QDomElement e = n.toElement();
	if (e.isNull()) {
		continue;
	}
	if (e.tagName() != "Widget") {
		continue;
	}
	QString className = e.namedItem("ClassName").toElement().text();
	if (className.isEmpty()) {
		boWarning() << k_funcinfo << "empty ClassName" << endl;
	}
	BoUfoWidget* widget = BoUfoFactory::createWidget(className);
	if (!widget) {
		boError() << k_funcinfo << "could not create widget with ClassName " << className << endl;
		continue;
	}
	parent->addWidget(widget);
	addWidget(widget, e);

	if (useConvenienceBackground) {
		int blue = depth;
		if (blue > 255) {
			blue = 255;
		}
		if (useRed) {
			widget->setBackgroundColor(QColor(255, 0, blue));
		} else {
			widget->setBackgroundColor(QColor(0, 255, blue));
		}
		useRed = !useRed;
	}

	widget->loadPropertiesFromXML(e.namedItem("Properties").toElement());

	if (useConvenienceBackground) {
		widget->setOpaque(true);
	}

	if (widget->name() == mNameOfSelectedWidget) {
		widget->setBorderType(BoUfoWidget::LineBorder);
	}

	depth += 40;
	updateGUI(e, widget);
	depth -= 40;
 }
}

void FormPreview::addWidget(BoUfoWidget* widget, const QDomElement& e)
{
 BO_CHECK_NULL_RET(widget);
 BO_CHECK_NULL_RET(widget->ufoWidget());
 if (e.isNull()) {
	boError() << k_funcinfo << "NULL element" << endl;
	return;
 }
 mUfoWidget2Element.insert(widget->ufoWidget(), e);
 mUfoWidget2Widget.insert(widget->ufoWidget(), widget);
}

void FormPreview::setPlacementMode(bool m)
{
 mPlacementMode = m;
}

void FormPreview::initializeGL()
{
 static bool recursive = false;
 static bool initialized = false;
 if (recursive) {
	return;
 }
 if (initialized) {
	return;
 }
 recursive = true;
 makeCurrent();

 glDisable(GL_DITHER);

 boDebug() << k_funcinfo << endl;
 mUfoManager = new BoUfoManager(width(), height(), true);
 mContentWidget = mUfoManager->contentWidget();

 recursive = false;
 initialized = true;
}

void FormPreview::resizeGL(int w, int h)
{
 makeCurrent();
 if (mUfoManager) {
	// WARNING: FIXME: we use size == oldsize
	// doesn't harm atm, as BoUfo does not use oldsize
	QResizeEvent r(QSize(w, h), QSize(w, h));

	mUfoManager->sendEvent(&r);
 }
}

void FormPreview::paintGL()
{
 if (mUfoManager) {
	mUfoManager->dispatchEvents();
	mUfoManager->render(false);
 }
}

bool FormPreview::eventFilter(QObject* o, QEvent* e)
{
 return QObject::eventFilter(o, e);
}

void FormPreview::mouseMoveEvent(QMouseEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void FormPreview::mousePressEvent(QMouseEvent* e)
{
 Q_UNUSED(e);
#if 0
 // AB: we display the ufo widgets only, we don't use them. so don't deliver
 // this event.
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
#endif

 if (mPlacementMode) {
	// we need a _container_ widget. it doesn't make sense to add children
	// to a button or so.
	QPoint pos = mapFromGlobal(QCursor::pos());
	BoUfoWidget* w = getContainerWidgetAt(pos.x(), pos.y());
	QDomElement parent;
	if (w) {
		parent = mUfoWidget2Element[w->ufoWidget()];
	}
	emit signalPlaceWidget(parent);
 } else {
	selectWidgetUnderCursor();
 }
}

void FormPreview::mouseReleaseEvent(QMouseEvent* e)
{
 Q_UNUSED(e);
#if 0
 // AB: we display the ufo widgets only, we don't use them. so don't deliver
 // this event.
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
#endif
}

void FormPreview::wheelEvent(QWheelEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
}

void FormPreview::keyPressEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyPressEvent(e);
}

void FormPreview::keyReleaseEvent(QKeyEvent* e)
{
 if (mUfoManager) {
	mUfoManager->sendEvent(e);
 }
 QGLWidget::keyReleaseEvent(e);
}

void FormPreview::selectWidgetUnderCursor()
{
 QPoint pos = mapFromGlobal(QCursor::pos());
 BoUfoWidget* w = getWidgetAt(pos.x(), pos.y());
 selectWidget(w);
}

void FormPreview::selectWidget(BoUfoWidget* widget)
{
 QDomElement widgetUnderCursor;
 if (widget && widget->ufoWidget()) {
	widgetUnderCursor = mUfoWidget2Element[widget->ufoWidget()];
 }

 emit signalSelectWidget(widgetUnderCursor);
}

void FormPreview::setSelectedWidget(const QDomElement& widget)
{
 // called e.g. when a widget in the widgettree is selected
 mNameOfSelectedWidget = QString::null;
 if (widget.isNull()) {
	return;
 }
 QDomElement properties = widget.namedItem("Properties").toElement();
 if (properties.isNull()) {
	boError() << k_funcinfo << "no Properties" << endl;
	return;
 }
 mNameOfSelectedWidget = properties.namedItem("name").toElement().text();

 BoUfoWidget* contentWidget = mUfoManager->contentWidget();
 BO_CHECK_NULL_RET(contentWidget);
 QDomElement root = mUfoWidget2Element[contentWidget->ufoWidget()];
 if (root.isNull()) {
	boError() << k_funcinfo << "cannot find root widget" << endl;
	return;
 }
 updateGUI(root);
}



