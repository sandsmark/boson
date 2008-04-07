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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "botexmapimportdialog.h"
#include "botexmapimportdialog.moc"

#include "../bomemory/bodummymemory.h"
#include "bodebug.h"
#include "bosonmap.h"
#include "bosongroundtheme.h"
#include "bofiledialog.h"

#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <knuminput.h>

#include <qlabel.h>
#include <qlayout.h>
#include <q3vbox.h>
#include <q3hbox.h>
#include <qpushbutton.h>
#include <qimage.h>
#include <qcombobox.h>
#include <q3vgroupbox.h>
#include <q3intdict.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3HBoxLayout>

class BoTexMapImportDialogPrivate
{
public:
	BoTexMapImportDialogPrivate()
	{
		mMap = 0;

		mTexMap = 0;
		mTexMapLabel = 0;

		mRed = 0;
		mGreen = 0;
		mBlue = 0;
		mAlpha = 0;

		mTexturesGroupBox = 0;
	}

	BosonMap* mMap;

	BoTexMap* mTexMap;
	QLabel* mTexMapLabel;

	QComboBox* mRed;
	QComboBox* mGreen;
	QComboBox* mBlue;
	QComboBox* mAlpha;

	Q3VGroupBox* mTexturesGroupBox;
	Q3IntDict<KIntNumInput> mTextures;

};

BoTexMapImportDialog::BoTexMapImportDialog(QWidget* parent)
		: KDialog(parent)
{
 d = new BoTexMapImportDialogPrivate();
 setWindowTitle(KDialog::makeStandardCaption(i18n("Texmap import")));
 setButtons(KDialog::Ok | KDialog::Apply | KDialog::Cancel);
 setDefaultButton(KDialog::Cancel);

 Q3HBoxLayout* layout = new Q3HBoxLayout(mainWidget());

 Q3VBox* texMapImage = new Q3VBox(mainWidget(), "texmapimage");
 d->mTexMapLabel = new QLabel(texMapImage, "texmaplabel");
 QPushButton* selectTexMapImage = new QPushButton(i18n("Select &texmap..."), texMapImage, "selecttexmapimage");
 connect(selectTexMapImage, SIGNAL(clicked()), this, SLOT(slotSelectTexMapImage()));
 layout->addWidget(texMapImage);

 Q3VGroupBox* colors = new Q3VGroupBox(i18n("Colors"), mainWidget(), "colors");
 layout->addWidget(colors);

 Q3HBox* hbox = new Q3HBox(colors, "hbox_red");
 (void)new QLabel(i18n("Red: "), hbox);
 d->mRed = new QComboBox(hbox);

 hbox = new Q3HBox(colors, "hbox_green");
 (void)new QLabel(i18n("Green: "), hbox);
 d->mGreen = new QComboBox(hbox);

 hbox = new Q3HBox(colors, "hbox_blue");
 (void)new QLabel(i18n("Blue: "), hbox);
 d->mBlue = new QComboBox(hbox);

 hbox = new Q3HBox(colors, "hbox_alpha");
 (void)new QLabel(i18n("Alpha: "), hbox);
 d->mAlpha = new QComboBox(hbox);

 connect(d->mRed, SIGNAL(activated(int)), this, SLOT(slotComponentTargetChanged(int)));
 connect(d->mGreen, SIGNAL(activated(int)), this, SLOT(slotComponentTargetChanged(int)));
 connect(d->mBlue, SIGNAL(activated(int)), this, SLOT(slotComponentTargetChanged(int)));
 connect(d->mAlpha, SIGNAL(activated(int)), this, SLOT(slotComponentTargetChanged(int)));

 d->mRed->setEnabled(false);
 d->mGreen->setEnabled(false);
 d->mBlue->setEnabled(false);
 d->mAlpha->setEnabled(false);

 d->mTexturesGroupBox = new Q3VGroupBox(i18n("Additional manipulation"), mainWidget(), "texturesgroupbox");
 layout->addWidget(d->mTexturesGroupBox);
 (void)new QLabel(i18n("Reset textures on entire map to (-1 does not touch at all)"), d->mTexturesGroupBox);
 d->mTextures.setAutoDelete(true);
}

BoTexMapImportDialog::~BoTexMapImportDialog()
{
 delete d->mTexMap;
 delete d;
}

bool BoTexMapImportDialog::unchanged() const
{
 if (!d->mMap) {
	return true;
 }
 if (!d->mTexMap) {
	return true;
 }
 if (d->mRed->currentItem() > 0 ||
		d->mGreen->currentItem() > 0 ||
		d->mBlue->currentItem() > 0 ||
		d->mAlpha->currentItem() > 0) {
	return false;
 }
 BosonGroundTheme* theme = d->mMap->groundTheme();
 if (!theme) {
	return true;
 }
 bool modifyDirect = false;
 for (unsigned int i = 0; i < theme->groundTypeCount() && !modifyDirect; i++) {
	if (!d->mTextures[i]) {
		continue;
	}
	if (d->mTextures[i]->value() >= 0) {
		modifyDirect = true;
	}
 }
 if (!modifyDirect) {
	return true;
 }
 return true;
}

void BoTexMapImportDialog::slotSelectTexMapImage()
{
 BO_CHECK_NULL_RET(d->mMap);
 QString fileName = BoFileDialog::getOpenFileName(QString::null, "*.png", this);
 if (fileName.isNull()) {
	return;
 }
 QImage image(fileName);
 if (image.isNull()) {
	boError() << k_funcinfo << "null image: fileName" << endl;
	KMessageBox::sorry(this, i18n("Unable to load %1 as image").arg(fileName));
	return;
 }
 if ((unsigned int)image.width() != d->mMap->width() + 1 ||
		(unsigned int)image.height() != d->mMap->height() + 1) {
	boError() << k_funcinfo << "invalid image size" << endl;
	KMessageBox::sorry(this, i18n("This image can't be used as texmap for this map. The map is a %1x%2 map, meaning you need a %3x%4 image.\nThe selected image (%5) is a %6x%7 image.").
			arg(d->mMap->width()).arg(d->mMap->height()).
			arg(d->mMap->width() + 1).arg(d->mMap->height() + 1).
			arg(fileName).
			arg(image.width()).arg(image.height()));
	return;
 }
 if (image.depth() != 32) {
	boError() << k_funcinfo << "bpp: " << image.depth() << endl;
	KMessageBox::sorry(this, i18n("Only 32bpp (depth) images are supported at the moment. %1 has %2 bits per pixel").arg(fileName).arg(image.depth()));
	return;
 }

 delete d->mTexMap;
 if (image.hasAlphaBuffer()) {
	d->mTexMap = new BoTexMap(4, image.width(), image.height());
 } else {
	d->mTexMap = new BoTexMap(3, image.width(), image.height());
 }
 if (!d->mTexMap->importTexMap(&image)) {
	boError() << k_funcinfo << "could not import texmap" << endl;
	KMessageBox::sorry(this, i18n("An error occured while importing the image"));
	delete d->mTexMap;
	d->mTexMap = 0;
	return;
 }

 QPixmap pixmap(image);
 d->mTexMapLabel->setPixmap(pixmap);

 d->mRed->setEnabled(true);
 d->mGreen->setEnabled(true);
 d->mBlue->setEnabled(true);
 d->mAlpha->setEnabled(image.hasAlphaBuffer());

 d->mRed->setCurrentItem(0);
 d->mGreen->setCurrentItem(0);
 d->mBlue->setCurrentItem(0);
 d->mAlpha->setCurrentItem(0);
}

void BoTexMapImportDialog::slotOk()
{
 boDebug() << k_funcinfo << endl;
 slotApply();
 accept();
 boDebug() << k_funcinfo << "done" << endl;
}

void BoTexMapImportDialog::slotApply()
{
 boDebug() << k_funcinfo << endl;
 if (unchanged()) {
	boDebug() << k_funcinfo << "nothing to do" << endl;
	return;
 }
 BO_CHECK_NULL_RET(d->mMap);
 BO_CHECK_NULL_RET(d->mTexMap);
 BosonGroundTheme* theme = d->mMap->groundTheme();
 BO_CHECK_NULL_RET(theme);
 int ret = KMessageBox::questionYesNo(this, i18n("You are about to use a different texmap. This will also change where units can go on the map and where they cannot go.\nYou may encounter trouble if units are at position where they cannot go anymore after the texmap changed.\n\nDo you really want to do this?"));
 if (ret == KMessageBox::Yes) {
	// apply direct manipulations first. texmap will override them, if same
	// textures will be used again
	for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
		if (!d->mTextures[i]) {
			continue;
		}
		if (d->mTextures[i]->value() < 0 || d->mTextures[i]->value() > 255) {
			continue;
		}
		d->mMap->resetTexMap(i, d->mTextures[i]->value());
	}

	if (d->mRed->currentItem() > 0) {
		boDebug() << k_funcinfo << "importing red" << endl;
		unsigned int dstTexture = (unsigned int)(d->mRed->currentItem() - 1);
		d->mMap->copyTexMapTexture(dstTexture, d->mTexMap, 0);
		boDebug() << k_funcinfo << "imported red" << endl;
	}
	if (d->mGreen->currentItem() > 0) {
		boDebug() << k_funcinfo << "importing green" << endl;
		unsigned int dstTexture = (unsigned int)(d->mGreen->currentItem() - 1);
		d->mMap->copyTexMapTexture(dstTexture, d->mTexMap, 1);
		boDebug() << k_funcinfo << "imported green" << endl;
	}
	if (d->mBlue->currentItem() > 0) {
		boDebug() << k_funcinfo << "importing blue" << endl;
		unsigned int dstTexture = (unsigned int)(d->mBlue->currentItem() - 1);
		d->mMap->copyTexMapTexture(dstTexture, d->mTexMap, 2);
		boDebug() << k_funcinfo << "imported blue" << endl;
	}
	if (d->mTexMap->textureCount() >= 4 && d->mAlpha->currentItem() > 0 && d->mAlpha->isEnabled()) {
		boDebug() << k_funcinfo << "importing alpha" << endl;
		unsigned int dstTexture = (unsigned int)(d->mAlpha->currentItem() - 1);
		d->mMap->copyTexMapTexture(dstTexture, d->mTexMap, 3);
		boDebug() << k_funcinfo << "imported alpha" << endl;
	}
 }
 boDebug() << k_funcinfo << "done" << endl;
}

void BoTexMapImportDialog::setMap(BosonMap* map)
{
 d->mMap = map;
 if (!d->mMap) {
	return;
 }
 if (!d->mMap->groundTheme()) {
	BO_NULL_ERROR(d->mMap->groundTheme());
	setMap(0);
	return;
 }
 QStringList textureList;
 textureList.append(i18n("Ignore this component"));
 BosonGroundTheme* theme = d->mMap->groundTheme();
 for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
	textureList.append(theme->groundType(i)->name);
 }
 d->mRed->clear();
 d->mGreen->clear();
 d->mBlue->clear();
 d->mAlpha->clear();

 d->mRed->insertStringList(textureList);
 d->mGreen->insertStringList(textureList);
 d->mBlue->insertStringList(textureList);
 d->mAlpha->insertStringList(textureList);

 d->mRed->setCurrentItem(0);
 d->mGreen->setCurrentItem(0);
 d->mBlue->setCurrentItem(0);
 d->mAlpha->setCurrentItem(0);

 d->mRed->setEnabled(false);
 d->mGreen->setEnabled(false);
 d->mBlue->setEnabled(false);
 d->mAlpha->setEnabled(false);

 d->mTextures.clear();
 for (unsigned int i = 0; i < theme->groundTypeCount(); i++) {
	KIntNumInput* input = new KIntNumInput(d->mTexturesGroupBox);
	input->setRange(-1, 255);
	input->setLabel(theme->groundType(i)->name);
	input->setValue(0);
	d->mTextures.insert(i, input);
 }
}

void BoTexMapImportDialog::slotComponentTargetChanged(int index)
{
 BO_CHECK_NULL_RET(sender());
 if (index < 0) {
	return;
 }
 if (index == 0) {
	// ignoring a component is _always_ valid
	return;
 }
 bool isTwice = false;
 QComboBox* s = (QComboBox*)sender();
 if (s != d->mRed && d->mRed->currentItem() == index) {
	isTwice = true;
 }
 if (s != d->mGreen && d->mGreen->currentItem() == index) {
	isTwice = true;
 }
 if (s != d->mBlue && d->mBlue->currentItem() == index) {
	isTwice = true;
 }
 if (s != d->mAlpha && d->mAlpha->currentItem() == index) {
	isTwice = true;
 }
 if (isTwice) {
	KMessageBox::sorry(this, i18n("Only one component can be assigned to one texture"));
	s->setCurrentItem(0);
 }
}

