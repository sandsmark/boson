/*
    This file is part of the Boson game
    Copyright (C) 2001-2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "bosonminimapdisplay.h"
#include "bosonminimapdisplay.moc"

#include "../../bomemory/bodummymemory.h"
#include "../gameengine/bosonmap.h"
#include "../bo3dtools.h"
#include "../gameengine/bosoncanvas.h"
#include "../bosonprofiling.h"
#include "../botexture.h"
#include "bodebug.h"
#include "bosonglminimapview.h"
#include <bogl.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QMouseEvent>

class BosonMiniMapDisplayPrivate
{
public:
	BosonMiniMapDisplayPrivate()
	{
		mLocalPlayerIO = 0;

		mLogoTexture = 0;
		mMiniMapView = 0;
	}
	PlayerIO* mLocalPlayerIO;

	bool mShowMiniMap;

	BoTexture* mLogoTexture;
	BosonGLMiniMapView* mMiniMapView;

};

BosonMiniMapDisplay::BosonMiniMapDisplay(QWidget* parent)
	: QWidget(parent)
{
 setName("ufoglminimapdisplay");

 d = new BosonMiniMapDisplayPrivate;
 d->mShowMiniMap = false;

// setOpaque(false);
// setMouseEventsEnabled(true, true);

 setShowMiniMap(false);

 resize(sizeHint());
 setMinimumSize(sizeHint());


 connect(this, SIGNAL(signalMouseMoved(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseDragged(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMousePressed(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseReleased(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
// connect(this, SIGNAL(signalMouseClicked(ufo::UMouseEvent*)),
//		this, SLOT(slotMouseEvent(ufo::UMouseEvent*)));
 connect(this, SIGNAL(signalMouseWheel(QWheelEvent*)),
		this, SLOT(slotWheelEvent(QWheelEvent*)));

 connect(this, SIGNAL(signalWidgetResized()),
		this, SLOT(slotWidgetResized()));

}

BosonMiniMapDisplay::~BosonMiniMapDisplay()
{
 delete d->mMiniMapView;
 delete d;
}

BosonGLMiniMapView* BosonMiniMapDisplay::miniMapView() const
{
 return d->mMiniMapView;
}

QSize BosonMiniMapDisplay::sizeHint() const
{
 return QSize(150, 150);
}

void BosonMiniMapDisplay::setLogoTexture(BoTexture* logo)
{
 d->mLogoTexture = logo;
}

void BosonMiniMapDisplay::quitGame()
{
 setLocalPlayerIO(0);
 delete d->mMiniMapView;
 d->mMiniMapView = 0;
 setShowMiniMap(false);
}

void BosonMiniMapDisplay::setLocalPlayerIO(PlayerIO* io)
{
 d->mLocalPlayerIO = io;
 if (d->mMiniMapView) {
	d->mMiniMapView->setLocalPlayerIO(io);
 }
 if (!io) {
	setShowMiniMap(false);
 }
}

bool BosonMiniMapDisplay::showMiniMap() const
{
 return d->mShowMiniMap;
}

void BosonMiniMapDisplay::setShowMiniMap(bool s)
{
 d->mShowMiniMap = s;
}

void BosonMiniMapDisplay::createMap(BosonCanvas* c, const BoGLMatrices* gameGLMatrices)
{
 BO_CHECK_NULL_RET(c);

 delete d->mMiniMapView;
 d->mMiniMapView = new BosonGLMiniMapView(gameGLMatrices, this);
 d->mMiniMapView->setViewSize(width(), height());
 d->mMiniMapView->setCanvas(c);
 d->mMiniMapView->setLocalPlayerIO(localPlayerIO());
 d->mMiniMapView->createMap(c->mapWidth(), c->mapHeight());
}


PlayerIO* BosonMiniMapDisplay::localPlayerIO() const
{
 return d->mLocalPlayerIO;
}

void BosonMiniMapDisplay::slotMouseEvent(QMouseEvent* e)
{
 if (!d->mMiniMapView) {
	return;
 }
 QPoint pos = e->pos();

 // AB: when using click+move, the coordinates may go off this widget. we don't
 // want this.
 pos.setX(qMax(0, pos.x()));
 pos.setY(qMax(0, pos.y()));
 pos.setX(qMin(pos.x(), width()));
 pos.setY(qMin(pos.y(), height()));

 QPoint cell = d->mMiniMapView->widgetToCell(pos);
 if (cell.x() < 0 || cell.y() < 0) {
	return;
 }

 // we accept all mouse events except mousemove events. this means that only
 // mouse move events are propagated to the parent (necessary for updating
 // cursor position)
 switch (e->type()) {
	case QMouseEvent::MouseMove:
		if (!(e->state() & Qt::LeftButton)) {
			// MouseMove is ignored when LMB is not pressed only
			e->ignore();
		} else {
			e->accept();
		}
		break;
	default:
		e->accept();
		break;
 }

 if (!showMiniMap()) {
	return;
 }

 int button = e->button();
 switch (e->type()) {
	case QMouseEvent::MouseMove:
		button = Qt::NoButton;
		if (e->state() & Qt::LeftButton) {
			button = Qt::LeftButton;
		} else {
			break;
		}
		// fall through intended, for LMB+Move
	case QMouseEvent::MouseButtonPress:
	{
		if (button == Qt::LeftButton) {
			emit signalReCenterView(cell);
		} else if (button == Qt::RightButton) {
			emit signalMoveSelection(cell.x(), cell.y());
		}
		break;
	}
	case QMouseEvent::MouseButtonRelease:
		break;
//	case QMouseEvent::MouseClicked:
//		break;
//	case QMouseEvent::MouseDragged:
//		break;
	default:
		break;
 }
}

void BosonMiniMapDisplay::slotWheelEvent(QWheelEvent* e)
{
 if (!d->mMiniMapView || !showMiniMap()) {
	return;
 }

 if (e->delta() > 0) {
	miniMapView()->zoomIn();
 } else {
	miniMapView()->zoomOut();
 }
 e->accept();
}

void BosonMiniMapDisplay::paintEvent(QPaintEvent*)
{
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the beginning of this method" << endl;
 }
 PROFILE_METHOD
 boTextureManager->invalidateCache();
 glPushMatrix();
 glTranslatef(0.0f, (float)height(), 0.0f);
 glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
 render();
 glPopMatrix();
 boTextureManager->invalidateCache();
 if (Bo3dTools::checkError()) {
	boError() << k_funcinfo << "GL error at the end of this method" << endl;
 }
}

void BosonMiniMapDisplay::render()
{
 if (!d->mMiniMapView) {
	return;
 }

// d->mMiniMapView->setZoom(zoom());

 d->mShowMiniMap = true;
 if (d->mShowMiniMap) {
	renderMiniMap();
 } else {
	renderLogo();
 }
}

void BosonMiniMapDisplay::renderMiniMap()
{
 d->mMiniMapView->render();
}

void BosonMiniMapDisplay::renderLogo()
{
 if (!d->mLogoTexture) {
	return;
 }
 glPushAttrib(GL_ENABLE_BIT);
 glEnable(GL_TEXTURE_2D);
 glColor3ub(255, 255, 255);
 d->mLogoTexture->bind();

 glPushMatrix();
 glScalef((float)(width()), (float)(height()), 1.0f);

 glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);
 glEnd();

 glPopMatrix();
 glPopAttrib();
 boTextureManager->invalidateCache();
}

void BosonMiniMapDisplay::slotWidgetResized()
{
 if (miniMapView()) {
	miniMapView()->setViewSize(width(), height());
 }
}

unsigned int BosonMiniMapDisplay::mapWidth() const
{
 if (miniMapView()) {
	return miniMapView()->mapWidth();
 }
 return 0;
}

unsigned int BosonMiniMapDisplay::mapHeight() const
{
 if (miniMapView()) {
	return miniMapView()->mapHeight();
 }
 return 0;
}

