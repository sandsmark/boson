/*
    This file is part of the Boson game
    Copyright (C) 2003 The Boson Team (boson-devel@lists.sourceforge.net)

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

#include <qintdict.h>
#include <qlayout.h>
#include <qlabel.h>

#include "bo3dtools.h"
#include "bodebug.h"

class BoMatrixWidgetPrivate
{
public:
	BoMatrixWidgetPrivate()
	{
		mLayout = 0;
	}
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
}
BoMatrixWidget::~BoMatrixWidget()
{
 delete d;
}

void BoMatrixWidget::setMatrix(BoMatrix* m)
{
 if (!m) {
	boError() << k_funcinfo << "NULL matrix" << endl;
	return;
 }
 for (int i = 0; i < 4; i++) {
	const float* data = m->data();
	d->mLabel[i + 0]->setText(QString::number(data[i + 0]));
	d->mLabel[i + 4]->setText(QString::number(data[i + 4]));
	d->mLabel[i + 8]->setText(QString::number(data[i + 8]));
	d->mLabel[i + 12]->setText(QString::number(data[i + 12]));
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
 for (int i = 0; i < 16; i++) {
	d->mLabel[i]->setText("");
 }
}


