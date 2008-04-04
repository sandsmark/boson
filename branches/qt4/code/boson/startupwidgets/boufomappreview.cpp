/*
    This file is part of the Boson game
    Copyright (C) 2006-2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "boufomappreview.h"
#include "boufomappreview.moc"

#include "../../bomemory/bodummymemory.h"
#include "../gameengine/player.h"
#include "../gameengine/bosonplayfield.h"
#include "../gameengine/bosonmap.h"
#include "../gameengine/bpfloader.h" // for BPFPreview // TODO: dedicated file!
#include "../boufo/boufoimage.h"
#include "../boufo/boufozoomscrollviewporthandler.h"
#include "bodebug.h"

#include <klocale.h>

#include <qimage.h>

class BoUfoMapPreviewDisplayPrivate
{
public:
	BoUfoMapPreviewDisplayPrivate()
	{
		mViewport = 0;
	}
	BoUfoZoomScrollViewportHandler* mViewport;

	BoUfoImage mPreview;
};

BoUfoMapPreviewDisplay::BoUfoMapPreviewDisplay()
	: BoUfoCustomWidget()
{
 d = new BoUfoMapPreviewDisplayPrivate;
 d->mViewport = new BoUfoZoomScrollViewportHandler(this);
 d->mViewport->setViewSize(width(), height());

 setMouseEventsEnabled(true, true);

 connect(this, SIGNAL(signalMouseMoved(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseDragged(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMousePressed(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseReleased(QMouseEvent*)),
		this, SLOT(slotMouseEvent(QMouseEvent*)));
 connect(this, SIGNAL(signalMouseWheel(QWheelEvent*)),
		this, SLOT(slotWheelEvent(QWheelEvent*)));

 connect(this, SIGNAL(signalWidgetResized()),
		this, SLOT(slotWidgetResized()));

 // FIXME: hardcoded
 setPreferredWidth(150);
 setPreferredHeight(150);
#if 0
 setMinimumWidth(150);
 setMinimumHeight(150);
 setSize(150, 150);
#endif
}

BoUfoMapPreviewDisplay::~BoUfoMapPreviewDisplay()
{
 delete d->mViewport;
 delete d;
}

void BoUfoMapPreviewDisplay::setPreview(const QImage& image)
{
 if (image.isNull()) {
	d->mPreview = BoUfoImage();
 } else {
	d->mPreview = BoUfoImage(image);
 }
 d->mViewport->setDataSize(d->mPreview.width(), d->mPreview.height());
}

void BoUfoMapPreviewDisplay::paintWidget()
{
 if (width() <= 0 || height() <= 0) {
	return;
 }
 glPushAttrib(GL_ALL_ATTRIB_BITS);
 glPushMatrix();

 glTranslatef(0.0f, (float)height(), 0.0f);
 glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
 glMultMatrixf(d->mViewport->transformationMatrix().data());

 glScalef(1.0f / ((float)d->mPreview.width()), 1.0f / ((float)d->mPreview.height()), 1.0f);
 glTranslatef(0.0f, (float)d->mPreview.height(), 0.0f);
 glRotatef(180.0f, 1.0f, 0.0f, 0.0f);

 d->mPreview.paint();

 glPopMatrix();
 glPopAttrib();
}

void BoUfoMapPreviewDisplay::slotWidgetResized()
{
 d->mViewport->setViewSize(width(), height());
}

void BoUfoMapPreviewDisplay::slotMouseEvent(QMouseEvent* e)
{
 QPoint pos = e->pos();

 // AB: when using click+move, the coordinates may go off this widget. we don't
 // want this.
 pos.setX(QMAX(0, pos.x()));
 pos.setY(QMAX(0, pos.y()));
 pos.setX(QMIN(pos.x(), width()));
 pos.setY(QMIN(pos.y(), height()));

 QPoint cell = d->mViewport->widgetPointToDataPoint(pos);
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
			// emit signalReCenterView(cell);
			d->mViewport->centerViewOnDataPoint(cell.x(), cell.y());
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

void BoUfoMapPreviewDisplay::slotWheelEvent(QWheelEvent* e)
{
 if (e->delta() > 0) {
	d->mViewport->zoomIn();
 } else {
	d->mViewport->zoomOut();
 }
 e->accept();
}


class BoUfoMapPreviewPrivate
{
public:
	BoUfoMapPreviewPrivate()
	{
		mPreviewDisplay = 0;
	}

	BoUfoMapPreviewDisplay* mPreviewDisplay;
};


BoUfoMapPreview::BoUfoMapPreview()
    : BoUfoWidget()
{
 d = new BoUfoMapPreviewPrivate;

 BoUfoWidget* hbox = new BoUfoWidget();
 hbox->setLayoutClass(BoUfoWidget::UHBoxLayout);
 addWidget(hbox);

 BoUfoWidget* vbox = new BoUfoWidget();
 vbox->setLayoutClass(BoUfoWidget::UVBoxLayout);
 hbox->addWidget(vbox);

 d->mPreviewDisplay = new BoUfoMapPreviewDisplay();
 vbox->addWidget(d->mPreviewDisplay);

 BoUfoWidget* stretch = 0;
 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 hbox->addWidget(stretch);

 stretch = new BoUfoWidget();
 stretch->setStretch(1);
 vbox->addWidget(stretch);

// d->mLabel->setVerticalAlignment(BoUfoWidget::AlignVCenter);
// d->mLabel->setHorizontalAlignment(BoUfoWidget::AlignHCenter);

}

BoUfoMapPreview::~BoUfoMapPreview()
{
 delete d;
}

void BoUfoMapPreview::setPlayField(const BPFPreview& preview)
{
 if (!preview.isLoaded()) {
	//d->mLabel->setText(i18n("No preview available"));
	d->mPreviewDisplay->setPreview(QImage());
	return;
 }
 if (preview.mapPreviewPNGData().size() <= 0) {
	setPlayField(0);
 } else {
	d->mPreviewDisplay->setPreview(QImage(preview.mapPreviewPNGData()));
 }
}


