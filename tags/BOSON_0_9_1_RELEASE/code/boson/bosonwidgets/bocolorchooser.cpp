/*
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

// note the copyright above: this is LGPL!

#include "bocolorchooser.h"
#include "bocolorchooser.moc"

#include <qvaluelist.h>
#include <qptrlist.h>
#include <qlabel.h>
#include <qmap.h>
#include <qbitmap.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qpen.h>

#include <kdebug.h>

class BoColorChooserPrivate
{
public:
	BoColorChooserPrivate()
	{
	}
	QPtrList<QLabel> mLabels;
	QValueList<QColor> mColors;
	QMap<QLabel*, bool> mTaken;
};

BoColorChooser::BoColorChooser(const QValueList<QColor>& colors, QWidget* parent, const char* name)
	: BoColorChooserBase(parent, name)
{
 init();
 setColors(colors);
}

BoColorChooser::BoColorChooser(QWidget* parent, const char* name)
	: BoColorChooserBase(parent, name)
{
 init();
}

BoColorChooser::~BoColorChooser()
{
 d->mLabels.clear();
 d->mColors.clear();
 d->mTaken.clear();
 delete d;
}

void BoColorChooser::init()
{
 d = new BoColorChooserPrivate;
 d->mLabels.append(mColor1);
 d->mLabels.append(mColor2);
 d->mLabels.append(mColor3);
 d->mLabels.append(mColor4);
 d->mLabels.append(mColor5);
 d->mLabels.append(mColor6);
 d->mLabels.append(mColor7);
 d->mLabels.append(mColor8);
 d->mLabels.append(mColor9);
 d->mLabels.append(mColor10);

 QPtrListIterator<QLabel> it(d->mLabels);
 while (it.current()) {
	it.current()->installEventFilter(this);
	it.current()->resize(it.current()->minimumSize());
	d->mTaken.insert(it.current(), false);
	++it;
 }
 QValueList<QColor> colors;
 colors.append(QColor(0, 0, 255));
 colors.append(QColor(0, 255, 0));
 colors.append(QColor(255, 0, 0));
 colors.append(QColor(255, 255, 0));
 colors.append(QColor(255, 0, 255));
 colors.append(QColor(0, 255, 255));
 colors.append(QColor(127, 255, 0));
 colors.append(QColor(255, 0, 127));
 colors.append(QColor(0, 127, 255));
 colors.append(QColor(0, 127, 127));
 while (colors.count() < d->mLabels.count()) {
	kdWarning() << k_funcinfo << "lacking colors. adding duplicates!" << endl;
	colors.append(QColor(0, 0, 255));
 }
 setColors(colors);
}

void BoColorChooser::setColors(const QValueList<QColor>& colors)
{
 if (colors.count() < d->mLabels.count()) {
	kdError() << k_funcinfo << "must provide at least " << d->mLabels.count() << " colors!" << endl;
	return;
 }
 d->mColors.clear();

 // we ensure that d->mColors.count() == d->mLabels.count(), even if
 // colors.count() > d->mLabels.count().
 QValueList<QColor>::ConstIterator colorIt = colors.begin();
 QPtrListIterator<QLabel> labelIt(d->mLabels);
 while (labelIt.current()) {
	d->mColors.append(*colorIt);
	++colorIt;
	++labelIt;
 }

 applyColors();
}

void BoColorChooser::applyColors()
{
 QValueList<QColor>::ConstIterator colorIt = d->mColors.begin();
 QPtrListIterator<QLabel> labelIt(d->mLabels);
 while (labelIt.current() && colorIt != d->mColors.end()) {
	QLabel* label = labelIt.current();
	label->setPaletteBackgroundColor(*colorIt);
	if (d->mTaken[label]) {
		markTaken(label);
	} else {
		label->setPixmap(QPixmap());
	}
	++colorIt;
	++labelIt;
 }

 // in case colors.count() < d->mLabels.count()
 while (labelIt.current()) {
	markTaken(labelIt.current());
	++labelIt;
 }
}

void BoColorChooser::markTaken(QLabel* label)
{
 if (!label) {
	kdError() << k_funcinfo << "NULL label" << endl;
	return;
 }
 QBitmap mask(label->width(), label->height(), true);
 QPixmap pix(label->width(), label->height());
 QPainter p(&mask);
 p.setPen(QPen(Qt::color1, 2));
 p.drawLine(0, 0, mask.width()-1, mask.height()-1);
 p.drawLine(mask.width()-1, 0, 0, mask.height()-1);
 pix.fill(Qt::black);
 pix.setMask(mask);
 label->setPixmap(pix);
}

void BoColorChooser::setAllTaken(bool taken)
{
 QValueList<QColor>::Iterator it;
 for (it = d->mColors.begin(); it != d->mColors.end(); ++it) {
	setTaken(*it, taken);
 }
}

void BoColorChooser::setTaken(const QColor& color, bool taken)
{
 QLabel* label = 0;
 QPtrListIterator<QLabel> labelIt(d->mLabels);
 QValueList<QColor>::Iterator colorIt = d->mColors.begin();
 while (labelIt.current() && colorIt != d->mColors.end() && !label) {
	if (*colorIt == color) {
		label = labelIt.current();
	}
	++labelIt;
	++colorIt;
 }
 if (!label) {
	kdDebug() << k_funcinfo << "color not found" << endl;
	return;
 }
 d->mTaken.replace(label, taken);

 applyColors();
}

bool BoColorChooser::eventFilter(QObject* o, QEvent* e)
{
 if (e->type() != QEvent::MouseButtonRelease) {
	return BoColorChooserBase::eventFilter(o, e);
 }
 if (o == this || !o->isA("QLabel")) {
	return BoColorChooserBase::eventFilter(o, e);
 }
 QLabel* label = (QLabel*)o;
 if (!d->mLabels.contains(label)) {
	return BoColorChooserBase::eventFilter(o, e);
 }

 // label is in d->mLabels and e is a MouseButtonRelease event at this point.
 if (d->mTaken[label]) {
	kdDebug() << k_funcinfo << "color is taken" << endl;
	// do nothing - color is already gone.
	return true;
 }
 int index = d->mLabels.find(label);
 if (index < 0 || (unsigned int)index >= d->mColors.count()) {
	kdDebug() << k_funcinfo << "no color found for index " << index << endl;
	return true;
 }
 QColor c = d->mColors[index];

 highlightColor(c);
 emit signalColorSelected(c);
 emit signalColorSelected(index);

 return true;
}

void BoColorChooser::highlightColor(const QColor& color)
{
 QPtrListIterator<QLabel> it(d->mLabels);
 while (it.current()) {
	it.current()->setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	++it;
 }

 if (!d->mColors.contains(color)) {
	kdError() << k_funcinfo << "dont know color " << color.rgb() << endl;
	return;
 }
 int index = d->mColors.findIndex(color);
 if (index < 0 || (unsigned int)index >= d->mLabels.count()) {
	return;
 }
 QLabel* label = d->mLabels.at(index);
 if (!label) {
	kdError() << k_funcinfo << "NULL label" << endl;
	return;
 }
 label->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
}

