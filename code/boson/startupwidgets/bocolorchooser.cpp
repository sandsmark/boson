/*
    This file is part of the Boson game
    Copyright (C) 2005-2008 Andreas Beckermann (b_mann@gmx.de)

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

#include "bocolorchooser.h"
#include "bocolorchooser.moc"

#include "../../bomemory/bodummymemory.h"
#include <bodebug.h>
#include <bogl.h>

#include <q3ptrlist.h>
#include <q3valuevector.h>
#include <qcolor.h>
#include <QEvent>
#include <QMouseEvent>
#include <Q3ValueList>
#include <QHBoxLayout>
#include <QLabel>

class BoColorDrawable : public BoUfoDrawable
{
public:
	BoColorDrawable()
	{
		mTaken = false;
		mHighlight = false;
	}

	void setColor(const QColor c)
	{
		mColor = c;
	}
	void setTaken(bool t)
	{
		mTaken = t;
	}
	void setHighlight(bool h)
	{
		mHighlight = h;
	}

	virtual int drawableWidth() const
	{
		return 20;
	}
	virtual int drawableHeight() const
	{
		return 20;
	}

	virtual void render(int x, int y, int w, int h)
	{
		glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_HINT_BIT);

		glDisable(GL_TEXTURE_2D);

		glColor3ub(mColor.red(), mColor.green(), mColor.blue());
		glBegin(GL_QUADS);
			glVertex2i(x, y);
			glVertex2i(x + w, y);
			glVertex2i(x + w, y + h);
			glVertex2i(x, y + h);
		glEnd();

		glColor3ub(0, 0, 0);
		glLineWidth(2.0f);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		const int margin = 3;
		if (mTaken) {
			glBegin(GL_LINES);
				glVertex2i(x + margin, y + margin);
				glVertex2i(x + w - margin, y + h - margin);

				glVertex2i(x + w - margin, y + margin);
				glVertex2i(x + margin, y + h - margin);
			glEnd();
		}
		if(mHighlight) {
			glBegin(GL_LINE_LOOP);
				glVertex2i(x + margin, y + margin);
				glVertex2i(x + w - margin, y + margin);
				glVertex2i(x + w - margin, y + h - margin);
				glVertex2i(x + margin, y + h - margin);
			glEnd();
		}

		glColor3ub(255, 255, 255);
		glPopAttrib();
	}

private:
	QColor mColor;
	bool mTaken;
	bool mHighlight;
};

class BoColorChooserPrivate
{
public:
	BoColorChooserPrivate()
	{
	}

	Q3PtrList<BoUfoWidget> mButtons;
	Q3PtrList<BoColorDrawable> mButtonDrawable;
	Q3ValueVector<QColor> mColors;
	Q3ValueVector<bool> mTaken;
};


BoColorChooser::BoColorChooser(QWidget* parent)
	: QWidget(parent)
{
 d = new BoColorChooserPrivate;

 QHBoxLayout* layout = new QHBoxLayout(this);

 const int count = 10;
 d->mTaken.resize(count);
 for (int i = 0; i < count; i++) {
	QLabel* button = new QLabel(this);
	layout->addWidget(button);
	button->setOpaque(true);
	d->mButtons.append(button);
	d->mTaken[i] = false;

	BoColorDrawable* drawable = new BoColorDrawable();
	d->mButtonDrawable.append(drawable);
	button->setBackground(drawable);

	button->setPreferredWidth(20);
	button->setPreferredHeight(20);
	button->setMinimumWidth(20);
	button->setMinimumHeight(20);

	button->setMouseEventsEnabled(true, false);
	connect(button, SIGNAL(signalMouseReleased(QMouseEvent*)),
			this, SLOT(slotMouseButtonReleased(QMouseEvent*)));
 }

 Q3ValueVector<QColor> colors(count);
 colors[0] = QColor(0, 0, 255);
 colors[1] = QColor(0, 255, 0);
 colors[2] = QColor(255, 0, 0);
 colors[3] = QColor(255, 255, 0);
 colors[4] = QColor(255, 0, 255);
 colors[5] = QColor(0, 255, 255);
 colors[6] = QColor(127, 255, 0);
 colors[7] = QColor(255, 0, 127);
 colors[8] = QColor(0, 127, 255);
 colors[9] = QColor(0, 127, 127);
 setColors(colors);

 showButtons(count);
}

BoColorChooser::~BoColorChooser()
{
 for (unsigned int i = 0; i < d->mButtons.count(); i++) {
	BoUfoWidget* w = d->mButtons.at(i);
	BoColorDrawable* drawable = d->mButtonDrawable.at(i);
	w->setBackground((BoUfoDrawable*)0);

	delete drawable;
 }
 d->mButtons.clear();
 d->mButtonDrawable.clear();
 delete d;
}

void BoColorChooser::showButtons(int count)
{
 if (count > (int)d->mButtons.count()) {
	boWarning() << k_funcinfo << "currently only <= " << d->mButtons.count() << " buttons are supported" << endl;
	count = d->mButtons.count();
 }
 if (count < 0) {
	count = 0;
 }
 for (int i = 0; i < count; i++) {
	d->mButtons.at(i)->show();
 }
 for (int i = count; i < (int)d->mButtons.count(); i++) {
	d->mButtons.at(i)->hide();
 }
}

unsigned int BoColorChooser::buttonCount() const
{
 return d->mButtons.count();
}

void BoColorChooser::setTaken(int i, bool taken)
{
 if (i < 0) {
	boError() << k_funcinfo << i << " < 0" << endl;
	return;
 }
 if (i >= (int)d->mTaken.count()) {
	boError() << k_funcinfo << i << " is out of range" << endl;
	return;
 }
 d->mTaken[i] = taken;

 applyColors();
}

void BoColorChooser::setTaken(const QColor& c, bool taken)
{
 int i = indexOfColor(c);
 if (i < 0) {
	boError() << k_funcinfo << "color not found" << endl;
	return;
 }
 setTaken(i, taken);
}

bool BoColorChooser::isTaken(int i) const
{
 if (i < 0) {
	boError() << k_funcinfo << i << " < 0" << endl;
	return false;
 }
 if (i >= (int)d->mTaken.count()) {
	boError() << k_funcinfo << i << " is out of range" << endl;
	return false;
 }
 return d->mTaken[i];
}

void BoColorChooser::setAllTaken(bool taken)
{
 for (unsigned int i = 0; i < d->mButtons.count(); i++) {
	setTaken(i, taken);
 }
}

QColor BoColorChooser::colorOfIndex(int i) const
{
 if (i < 0) {
	boError() << k_funcinfo << i << " < 0" << endl;
	return QColor();
 }
 if (i >= (int)d->mTaken.count()) {
	boError() << k_funcinfo << i << " is out of range" << endl;
	return QColor();
 }
 return d->mColors[i];
}

int BoColorChooser::indexOfColor(const QColor& c) const
{
 for (int i = 0; i < (int)d->mColors.count(); i++) {
	if (d->mColors[i] == c) {
		return i;
	}
 }
 return -1;
}

void BoColorChooser::setColors(const Q3ValueVector<QColor>& colors)
{
 if (colors.count() < d->mButtons.count()) {
	boError() << k_funcinfo << "must provide at least " << d->mButtons.count() << " colors" << endl;
	return;
 }
 d->mColors = colors;

 applyColors();
}

void BoColorChooser::setColors(const Q3ValueList<QColor>& colors)
{
 Q3ValueVector<QColor> v(colors.count());
 for (unsigned int i = 0; i < colors.count(); i++) {
	v[i] = colors[i];
 }
 setColors(v);
}

void BoColorChooser::applyColors()
{
 for (unsigned int i = 0; i < d->mButtons.count(); i++) {
	BoUfoWidget* b = d->mButtons.at(i);
	QColor c = d->mColors[i];
	BoColorDrawable* drawable = d->mButtonDrawable.at(i);

	b->setOpaque(true);
	drawable->setColor(c);
//	b->setBackgroundColor(c);

	drawable->setTaken(isTaken(i));
 }
}

void BoColorChooser::highlightColor(int i)
{
 if (i < 0) {
	boError() << k_funcinfo << i << " < 0" << endl;
	return;
 }
 if (i >= (int)d->mTaken.count()) {
	boError() << k_funcinfo << i << " is out of range" << endl;
	return;
 }

 for (int j = 0; j < (int)buttonCount(); j++) {
	if (j == i) {
		continue;
	}
	unhighlightColor(j);
 }

 BoUfoWidget* b = d->mButtons.at(i);
 b->setBorderType(BoUfoWidget::LoweredBevelBorder);
 BoColorDrawable* drawable = d->mButtonDrawable.at(i);
 drawable->setHighlight(true);
}

void BoColorChooser::highlightColor(const QColor& c)
{
 int i = indexOfColor(c);
 if (i < 0) {
	boError() << k_funcinfo << "color not found" << endl;
	return;
 }
 highlightColor(i);
}

void BoColorChooser::unhighlightColor(int i)
{
 if (i < 0) {
	boError() << k_funcinfo << i << " < 0" << endl;
	return;
 }
 if (i >= (int)d->mTaken.count()) {
	boError() << k_funcinfo << i << " is out of range" << endl;
	return;
 }

 BoUfoWidget* b = d->mButtons.at(i);
 b->setBorderType(BoUfoWidget::NoBorder);
 BoColorDrawable* drawable = d->mButtonDrawable.at(i);
 drawable->setHighlight(false);
}

void BoColorChooser::unhighlightColor(const QColor& c)
{
 int i = indexOfColor(c);
 if (i < 0) {
	boError() << k_funcinfo << "color not found" << endl;
	return;
 }
 unhighlightColor(i);
}

void BoColorChooser::slotButtonClicked(int i)
{
 if (i < 0 || (unsigned int)i >= d->mButtons.count()) {
	boError() << k_funcinfo << "invalid index " << i << endl;
	return;
 }
 if (isTaken(i)) {
	boDebug() << k_funcinfo << i << " already taken" << endl;
	return;
 }
 emit signalColorSelected(i);
 emit signalColorSelected(colorOfIndex(i));
}

void BoColorChooser::slotMouseButtonReleased(QMouseEvent* e)
{
 if (e->type() != QEvent::MouseButtonRelease) {
	return;
 }
 if (e->button() != Qt::LeftButton) {
	return;
 }
 for (int i = 0; i < (int)d->mButtons.count(); i++) {
	if ((QObject*)d->mButtons.at(i) == sender()) {
		slotButtonClicked(i);
		return;
	}
 }
 boError() << k_funcinfo << "sender not found" << endl;
}


