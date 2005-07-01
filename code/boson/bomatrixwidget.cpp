/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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

#include "bomatrixwidget.h"
#include "bomatrixwidget.moc"

#include "../bomemory/bodummymemory.h"
#include "bo3dtools.h"
#include "bodebug.h"

#include <qintdict.h>
#include <qlayout.h>
#include <qlabel.h>

#include <math.h>

class BoMatrixWidgetPrivate
{
public:
	BoMatrixWidgetPrivate()
	{
		mLayout = 0;
	}
	BoMatrix mMatrix;
	QGridLayout* mLayout;
	QIntDict<QLabel> mLabel;
};

BoMatrixWidget::BoMatrixWidget(QWidget* parent, const char* name) : QWidget(parent, name)
{
 d = new BoMatrixWidgetPrivate;
 d->mLayout = new QGridLayout(this, 0, 1);
 for (int i = 0; i < 16; i++) {
	QLabel* l = new QLabel(this);
	d->mLabel.insert(i, l);
	d->mLayout->addWidget(l, i % 4, i / 4);
 }
 mPrecision = 6;
}
BoMatrixWidget::~BoMatrixWidget()
{
 delete d;
}

void BoMatrixWidget::setMatrix(const BoMatrix* m)
{
 if (!m) {
	boError() << k_funcinfo << "NULL matrix" << endl;
	return;
 }
 setMatrix(*m);
}

void BoMatrixWidget::setMatrix(const BoMatrix& m)
{
 d->mMatrix = m;
 for (int i = 0; i < 4; i++) {
	const float* data = d->mMatrix.data();
	d->mLabel[i + 0]->setText(QString::number(data[i + 0], 'f', precision()));
	d->mLabel[i + 4]->setText(QString::number(data[i + 4], 'f', precision()));
	d->mLabel[i + 8]->setText(QString::number(data[i + 8], 'f', precision()));
	d->mLabel[i + 12]->setText(QString::number(data[i + 12], 'f', precision()));
 }
}

void BoMatrixWidget::setMatrix(Lib3dsMatrix m)
{
 BoMatrix matrix;
 for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 4; j++) {
		matrix.setElement(i, j, m[j][i]);
	}
 }
 setMatrix(&matrix);
}

void BoMatrixWidget::setIdentity()
{
 BoMatrix m;
 setMatrix(&m);
}

void BoMatrixWidget::clear()
{
 d->mMatrix.loadIdentity();
 setMatrix(d->mMatrix);
 for (int i = 0; i < 16; i++) {
	unmark(i);
 }
}

const BoMatrix& BoMatrixWidget::matrix() const
{
 return d->mMatrix;
}

void BoMatrixWidget::mark(unsigned int i)
{
 if (i >= 16) {
	return;
 }
 QPalette p(palette());
 p.setColor(QColorGroup::Text, Qt::red);
// p.setColor(QColorGroup::ForeGround, Qt::red);
 d->mLabel[i]->setPalette(p);
 d->mLabel[i]->setPaletteForegroundColor(Qt::red);
}

void BoMatrixWidget::unmark(unsigned int i)
{
 if (i >= 16) {
	return;
 }
 d->mLabel[i]->unsetPalette();
}

bool BoMatrixWidget::compareMatrices(const BoMatrix& m, float diff)
{
 bool ok = true;
 for (unsigned int i = 0; i < 16; i++) {
	if (fabs(d->mMatrix[i] - m[i]) <= diff) {
		unmark(i);
	} else {
		mark(i);
		ok = false;
	}
 }
 return ok;
}

bool BoMatrixWidget::compareMatrices(BoMatrixWidget* widget, float diff)
{
 if (!widget) {
	return false;
 }
 // remember that the right part of && is not evaluated, if the left part is
 // false! but we want to mark them in all cases!
 bool ok = compareMatrices(widget->matrix(), diff);
 bool ok2 = widget->compareMatrices(matrix(), diff);
 return (ok && ok2);
}


class BoUfoMatrixWidgetPrivate
{
public:
	BoUfoMatrixWidgetPrivate()
	{
	}
	BoMatrix mMatrix;
	QIntDict<BoUfoLabel> mLabel;
};


BoUfoMatrixWidget::BoUfoMatrixWidget() : BoUfoWidget()
{
 d = new BoUfoMatrixWidgetPrivate;
 BoUfoVBox* vbox = new BoUfoVBox(); // FIXME: we need a grid layout!
 addWidget(vbox);
 for (int i = 0; i < 16; i++) {
	BoUfoLabel* l = new BoUfoLabel();
	d->mLabel.insert(i, l);
//	d->mLayout->addWidget(l, i % 4, i / 4); // TODO: grid layout
	vbox->addWidget(l); // FIXME: grid layout
 }
 mPrecision = 6;
}

BoUfoMatrixWidget::~BoUfoMatrixWidget()
{
 delete d;
}

void BoUfoMatrixWidget::setMatrix(const BoMatrix* m)
{
 if (!m) {
	boError() << k_funcinfo << "NULL matrix" << endl;
	return;
 }
 setMatrix(*m);
}

void BoUfoMatrixWidget::setMatrix(const BoMatrix& m)
{
 d->mMatrix = m;
 for (int i = 0; i < 4; i++) {
	const float* data = d->mMatrix.data();
	d->mLabel[i + 0]->setText(QString::number(data[i + 0], 'f', precision()));
	d->mLabel[i + 4]->setText(QString::number(data[i + 4], 'f', precision()));
	d->mLabel[i + 8]->setText(QString::number(data[i + 8], 'f', precision()));
	d->mLabel[i + 12]->setText(QString::number(data[i + 12], 'f', precision()));
 }
}

void BoUfoMatrixWidget::setMatrix(Lib3dsMatrix m)
{
 BoMatrix matrix;
 for (int i = 0; i < 4; i++) {
	for (int j = 0; j < 4; j++) {
		matrix.setElement(i, j, m[j][i]);
	}
 }
 setMatrix(&matrix);
}

void BoUfoMatrixWidget::setIdentity()
{
 BoMatrix m;
 setMatrix(&m);
}

void BoUfoMatrixWidget::clear()
{
 d->mMatrix.loadIdentity();
 setMatrix(d->mMatrix);
 for (int i = 0; i < 16; i++) {
	unmark(i);
 }
}

const BoMatrix& BoUfoMatrixWidget::matrix() const
{
 return d->mMatrix;
}

void BoUfoMatrixWidget::mark(unsigned int i)
{
 if (i >= 16) {
	return;
 }
#warning fixme
#if 0
 QPalette p(palette());
 p.setColor(QColorGroup::Text, Qt::red);
// p.setColor(QColorGroup::ForeGround, Qt::red);
 d->mLabel[i]->setPalette(p);
 d->mLabel[i]->setPaletteForegroundColor(Qt::red);
#endif
}

void BoUfoMatrixWidget::unmark(unsigned int i)
{
 if (i >= 16) {
	return;
 }
#warning fixme
#if 0
 d->mLabel[i]->unsetPalette();
#endif
}

bool BoUfoMatrixWidget::compareMatrices(const BoMatrix& m, float diff)
{
 bool ok = true;
 for (unsigned int i = 0; i < 16; i++) {
	if (fabs(d->mMatrix[i] - m[i]) <= diff) {
		unmark(i);
	} else {
		mark(i);
		ok = false;
	}
 }
 return ok;
}

bool BoUfoMatrixWidget::compareMatrices(BoUfoMatrixWidget* widget, float diff)
{
 if (!widget) {
	return false;
 }
 // remember that the right part of && is not evaluated, if the left part is
 // false! but we want to mark them in all cases!
 bool ok = compareMatrices(widget->matrix(), diff);
 bool ok2 = widget->compareMatrices(matrix(), diff);
 return (ok && ok2);
}


