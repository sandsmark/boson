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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <bogl.h>

// AB: first include the ufo headers, otherwise we conflict with Qt
#include <ufo/ufo.hpp>

// AB: make sure that we are compatible to system that have QT_NO_STL defined
#ifndef QT_NO_STL
#define QT_NO_STL
#endif

#include "bouforadiobutton.h"
#include "bouforadiobutton.moc"

#include "boufolabel.h"
#include <bodebug.h>

BoUfoButtonGroup::BoUfoButtonGroup(QObject* parent)
	: QObject(parent)
{
 mButtonGroup = new ufo::UButtonGroup();
 mButtonGroup->reference();

 mButtons = new QMap<ufo::URadioButton*, BoUfoRadioButton*>();
}

BoUfoButtonGroup::~BoUfoButtonGroup()
{
 mButtonGroup->unreference();
 delete mButtons;
}

void BoUfoButtonGroup::addButton(BoUfoRadioButton* button)
{
 BO_CHECK_NULL_RET(button);
 connect(button, SIGNAL(signalActivated()),
		this, SLOT(slotButtonActivated()));
 ufo::UButton* b = (ufo::UButton*)button->radioButton();
 mButtonGroup->addButton(b);

 mButtons->insert(button->radioButton(), button);
}

void BoUfoButtonGroup::removeButton(BoUfoRadioButton* button)
{
 BO_CHECK_NULL_RET(button);
 ufo::UButton* b = (ufo::UButton*)button->radioButton();
 mButtonGroup->removeButton(b);

 mButtons->remove(button->radioButton());
}

void BoUfoButtonGroup::slotButtonActivated()
{
 emit signalButtonActivated(selectedButton());
}

BoUfoRadioButton* BoUfoButtonGroup::selectedButton() const
{
 ufo::URadioButton* b = dynamic_cast<ufo::URadioButton*>(mButtonGroup->getSelectedButton());
 BoUfoRadioButton* button = 0;
 if (b) {
	button = (*mButtons)[b];
 }
 return button;
}

BoUfoButtonGroupWidget::BoUfoButtonGroupWidget()
	: BoUfoWidget()
{
 mButtonGroup = new BoUfoButtonGroup(this);
 connect(mButtonGroup, SIGNAL(signalButtonActivated(BoUfoRadioButton*)),
		this, SIGNAL(signalButtonActivated(BoUfoRadioButton*)));
}

BoUfoButtonGroupWidget::~BoUfoButtonGroupWidget()
{
}

void BoUfoButtonGroupWidget::addWidget(BoUfoWidget* widget)
{
 BoUfoWidget::addWidget(widget);
 if (widget && widget->inherits("BoUfoRadioButton")) {
	mButtonGroup->addButton((BoUfoRadioButton*)widget);
 }
}

void BoUfoButtonGroupWidget::removeWidget(BoUfoWidget* widget)
{
 BoUfoWidget::removeWidget(widget);
 if (widget && widget->inherits("BoUfoRadioButton")) {
	mButtonGroup->removeButton((BoUfoRadioButton*)widget);
 }
}

BoUfoRadioButton* BoUfoButtonGroupWidget::selectedButton() const
{
 return mButtonGroup->selectedButton();
}

BoUfoRadioButton::BoUfoRadioButton()
	: BoUfoWidget()
{
 init();
}

BoUfoRadioButton::BoUfoRadioButton(const QString& text, bool selected)
	: BoUfoWidget()
{
 init();
 setText(text);
 setSelected(selected);
}

void BoUfoRadioButton::init()
{
 setLayoutClass(UHBoxLayout);
 mRadioButton = new ufo::URadioButton();
 ufoWidget()->add(mRadioButton);
 // AB: at least the background of the label must be transparent
 mRadioButton->setOpaque(false);

 setForegroundColor(BoUfoLabel::defaultForegroundColor());

 CONNECT_UFO_TO_QT(BoUfoRadioButton, mRadioButton, Activated);
 CONNECT_UFO_TO_QT(BoUfoRadioButton, mRadioButton, Highlighted);
}

void BoUfoRadioButton::setOpaque(bool o)
{
 BoUfoWidget::setOpaque(o);
 mRadioButton->setOpaque(o);
}

void BoUfoRadioButton::uslotActivated(ufo::UActionEvent*)
{
 emit signalActivated();
 emit signalToggled(selected());
}

void BoUfoRadioButton::uslotHighlighted(ufo::UActionEvent*)
{
 emit signalHighlighted();
}

void BoUfoRadioButton::setMinimumSize(const ufo::UDimension& s)
{
 BoUfoWidget::setMinimumSize(s);
 mRadioButton->setMinimumSize(s);
}

void BoUfoRadioButton::setPreferredSize(const ufo::UDimension& s)
{
 BoUfoWidget::setPreferredSize(s);
 mRadioButton->setPreferredSize(s);
}

void BoUfoRadioButton::setText(const QString& text)
{
 if (text.isNull()) {
	mRadioButton->setText("");
 } else {
	mRadioButton->setText(text.latin1());
 }
}

QString BoUfoRadioButton::text() const
{
 QString text = mRadioButton->getText().c_str();
 return text;
}

void BoUfoRadioButton::setSelected(bool s)
{
 mRadioButton->setSelected(s);
}

bool BoUfoRadioButton::selected() const
{
 return mRadioButton->isSelected();
}


