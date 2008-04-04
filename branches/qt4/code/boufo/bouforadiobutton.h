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

// note the copyright above: this is LGPL!
#ifndef BOUFORADIOBUTTON_H
#define BOUFORADIOBUTTON_H

#include "boufowidget.h"

class BoUfoRadioButton : public BoUfoWidget
{
	Q_OBJECT
	Q_PROPERTY(QString text READ text WRITE setText);
	Q_PROPERTY(bool selected READ selected WRITE setSelected);
public:
	// AB: we must not use a QObject parent here. otherwise garbage
	// collection of libufo and Qt may confuse each other.
	BoUfoRadioButton();
	BoUfoRadioButton(const QString& text, bool selected = false);

	ufo::URadioButton* radioButton() const
	{
		return mRadioButton;
	}

	void setText(const QString& text);
	QString text() const;
	void setSelected(bool);
	bool selected() const;

	virtual void setMinimumSize(const ufo::UDimension& size);
	virtual void setPreferredSize(const ufo::UDimension& size);

	virtual void setOpaque(bool o);

signals:
	void signalActivated();
	void signalHighlighted();
	void signalToggled(bool);

private:
	void init();
	void uslotActivated(ufo::UActionEvent*);
	void uslotHighlighted(ufo::UActionEvent*);

private:
	ufo::URadioButton* mRadioButton;
};

/**
 * WARNING: buttons that are deleted are @em not automatically removed from the
 * button group! You should make sure that the button group is always deleted
 * first!
 *
 * Note: An object of this class is not deleted by libufo but by Qt (this is
 * contrary to most other BoUfo classes). It is therefore recommended to use a
 * Qt parent for objects of this class (e.g. a BoUfoWidget).
 **/
class BoUfoButtonGroup : public QObject
{
	Q_OBJECT
public:
	BoUfoButtonGroup(QObject* parent);
	~BoUfoButtonGroup();

	void addButton(BoUfoRadioButton* button);
	void removeButton(BoUfoRadioButton* button);

	BoUfoRadioButton* selectedButton() const;

signals:
	void signalButtonActivated(BoUfoRadioButton* button);

protected slots:
	void slotButtonActivated();

private:
	ufo::UButtonGroup* mButtonGroup;
	QMap<ufo::URadioButton*, BoUfoRadioButton*>* mButtons;
};

/**
 * A normal @ref BoUfoWidget that automatically adds any @ref BoUfoRadioButton
 * that are added to an internal @ref BoUfoButtonGroup.
 **/
class BoUfoButtonGroupWidget : public BoUfoWidget
{
	Q_OBJECT
public:
	BoUfoButtonGroupWidget();
	~BoUfoButtonGroupWidget();

	virtual void addWidget(BoUfoWidget*);
	virtual void removeWidget(BoUfoWidget*);

	BoUfoRadioButton* selectedButton() const;

signals:
	void signalButtonActivated(BoUfoRadioButton*);

private:
	BoUfoButtonGroup* mButtonGroup;
};




#endif
