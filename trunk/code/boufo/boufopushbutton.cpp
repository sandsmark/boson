/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Andreas Beckermann (b_mann@gmx.de)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "boufopushbutton.h"
#include "boufopushbutton.moc"

#include "boufoimage.h"
#include "boufodrawable.h"
#include <bodebug.h>
#include "ufoext/ubodrawableicon.h"

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qimage.h>

#include <math.h>

BoUfoPushButton::BoUfoPushButton()
	: BoUfoWidget(new ufo::UButton())
{
 init();
}

BoUfoPushButton::BoUfoPushButton(const QString& text)
	: BoUfoWidget(new ufo::UButton())
{
 init();
 setText(text);
}

void BoUfoPushButton::init()
{
 mButton = (ufo::UButton*)ufoWidget();

 CONNECT_UFO_TO_QT(BoUfoPushButton, mButton, Activated);
 CONNECT_UFO_TO_QT(BoUfoPushButton, mButton, Highlighted);

 // allow clicking the button + enable highlight effect
 setMouseEventsEnabled(true, true);

 // AB: key events may be required for several tasks on buttons (using tab to
 //     change to a different button or using return to activate a selected
 //     button) but with the current ufo/boson event design they cause a lot of
 //     trouble.
 //     for example in the editor select a unit, then click some button (e.g.
 //     one of the units in the commandframe) and then try to delete it using
 //     "del". it won't work, because the button receives the key event and
 //     therefore the KAction object won't.
 //     (KActions get the key event only, if the focused widget does not take
 //     key events)
 setKeyEventsEnabled(false);

 // TODO:
 // it would be nice if we could use setOpaque(false), but still have a border
 // around the button.
 // additionally we should provide some kind of default color (like for labels)
 setOpaque(true);



 connect(this, SIGNAL(signalMouseDragged(QMouseEvent*)),
		this, SLOT(slotMouseDragged(QMouseEvent*)));
}

void BoUfoPushButton::setVerticalAlignment(VerticalAlignment a)
{
 BoUfoWidget::setVerticalAlignment(a);
 mButton->setVerticalAlignment(ufoWidget()->getVerticalAlignment());
}

void BoUfoPushButton::setHorizontalAlignment(HorizontalAlignment a)
{
 BoUfoWidget::setHorizontalAlignment(a);
 mButton->setHorizontalAlignment(ufoWidget()->getHorizontalAlignment());
}

void BoUfoPushButton::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mButton->setOpaque(o);
}

void BoUfoPushButton::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalClicked();
}

void BoUfoPushButton::uslotHighlighted(ufo::UActionEvent*)
{
 emit signalHighlighted();
}

void BoUfoPushButton::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mButton->setMinimumSize(s);
}

void BoUfoPushButton::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mButton->setPreferredSize(s);
}

void BoUfoPushButton::setText(const QString& text)
{
 if (text.isNull()) {
	mButton->setText("");
 } else {
	mButton->setText(text.latin1());
 }
}

QString BoUfoPushButton::text() const
{
 QString text = mButton->getText().c_str();
 return text;
}

void BoUfoPushButton::setIcon(const BoUfoImage& img)
{
 if (img.isNull()) {
	mButton->setIcon(0);
 } else {
	mButton->setIcon(new ufo::UBoDrawableIcon(img.image()));
 }
}

void BoUfoPushButton::setIcon(const BoUfoDrawable& drawable)
{
 if (!drawable.drawable()) {
	mButton->setIcon(0);
 } else {
	mButton->setIcon(new ufo::UBoDrawableIcon(drawable.drawable()));
 }
}

void BoUfoPushButton::setIconFile(const QString& file_)
{
 QString file = file_;
 if (!file_.isEmpty()) {
	QImage img;
	if (KGlobal::_instance) { // NULL in boufodesigner
		file = locate("data", "boson/" + file_);
		if (file.isEmpty()) {
			boDebug() << k_funcinfo << "file " << file_ << " not found" << endl;
			file = file_;
		}
	}
	if (!img.load(file)) {
		boError() << k_funcinfo << file << " could not be loaded" << endl;
		return;
	}
	setIcon(img);
 } else {
	setIcon(BoUfoImage());
 }
 mIconFile = file;
}

QString BoUfoPushButton::iconFile() const
{
 return mIconFile;
}

void BoUfoPushButton::setToggleButton(bool t)
{
 mButton->setToggable(t);
}

bool BoUfoPushButton::isToggleButton() const
{
 return mButton->isToggable();
}

void BoUfoPushButton::setOn(bool on)
{
 mButton->setSelected(on);
}

bool BoUfoPushButton::isOn() const
{
 return mButton->isSelected();
}

void BoUfoPushButton::slotMouseDragged(QMouseEvent* e)
{
 e->accept();
}

