/*
    This file is part of the Boson game
    Copyright (C) 2005 Andreas Beckermann (b_mann@gmx.de)

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

#include <ufo/ufo.hpp>

#include "boufofontselectionwidget.h"
#include "boufofontselectionwidget.moc"

#include <bodebug.h>

#include <klocale.h>

#include <qmap.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3ValueList>

class BoUfoFontSelectionWidgetPrivate
{
public:
	BoUfoFontSelectionWidgetPrivate()
	{
		mUfoManager = 0;
	}
	BoUfoManager* mUfoManager;
	BoUfoFontInfo mFontInfo;
	QMap<int, int> mIndex2Style;
};

BoUfoFontSelectionWidget::BoUfoFontSelectionWidget(BoUfoManager* manager)
	: BoUfoFontSelectionWidgetBase()
{
 d = new BoUfoFontSelectionWidgetPrivate;
 d->mUfoManager = manager;

 QStringList families;
 Q3ValueList<BoUfoFontInfo> allFonts = d->mUfoManager->listFonts();
 for (unsigned int i = 0; i < allFonts.count(); i++) {
	QString font = QString("%1: %2").arg(allFonts[i].fontPlugin()).arg(allFonts[i].family());
	if (!families.contains(font)) {
		families.append(font);
	}
 }
 mFonts->setItems(families);
 if (allFonts.count() > 0) {
	// TODO: rather select the font currently in use
	mFonts->setSelectedItem(0);
 }

 QStringList sizes;
 sizes.append("6");
 sizes.append("7");
 sizes.append("8");
 sizes.append("9");
 sizes.append("10");
 sizes.append("11");
 sizes.append("12");
 sizes.append("13");
 sizes.append("14");
 sizes.append("15");
 sizes.append("16");
 sizes.append("17");
 sizes.append("18");
 sizes.append("19");
 sizes.append("20");
 mSizes->setItems(sizes);
 mSizesCombo->setItems(sizes);
 mSizes->setItemSelected(sizes.findIndex("12"), true);
 mSizesCombo->setCurrentItem(sizes.findIndex("12"));

 int supportedStyles = 0;
 supportedStyles |= BoUfoFontInfo::StyleItalic;
 supportedStyles |= BoUfoFontInfo::StyleBold;
 supportedStyles |= BoUfoFontInfo::StyleUnderline;
 supportedStyles |= BoUfoFontInfo::StyleStrikeOut;
 setSupportedStyles(supportedStyles);
}

BoUfoFontSelectionWidget::~BoUfoFontSelectionWidget()
{
 delete d;
}

const BoUfoFontInfo& BoUfoFontSelectionWidget::fontInfo() const
{
 return d->mFontInfo;
}

void BoUfoFontSelectionWidget::slotSizeChanged()
{
 QString size = mSizes->selectedText();
 mSizesCombo->setCurrentItem(mSizesCombo->items().findIndex(size));
 updateFont();
}

void BoUfoFontSelectionWidget::slotSizeChangedCombo()
{
 QString size = mSizesCombo->currentText();
 if (mSizes->items().findIndex(size) < 0) {
	boError() << k_funcinfo << "sizes listbox does not know about size " << size << " reverting to previous size" << endl;
	size = QString::number((int)fontInfo().pointSize());
	mSizesCombo->setCurrentItem(mSizesCombo->items().findIndex(size));
	updateFont(); // no-op
	return;
 }
 mSizes->blockSignals(true);
 mSizes->setItemSelected(mSizes->items().findIndex(size), true);
 mSizes->blockSignals(false);
 updateFont();
}

void BoUfoFontSelectionWidget::slotStyleChanged()
{
 updateFont();
}

void BoUfoFontSelectionWidget::slotFontChanged()
{
 updateFont();
}

void BoUfoFontSelectionWidget::updateFont()
{
 bool familyChanged = false;
 QString family;
 QString plugin;
 QRegExp familyExp("(.*): (.*)");
 if (familyExp.search(mFonts->selectedText()) < 0) {
	boError() << k_funcinfo << "oops internal error" << endl;
	return;
 }
 plugin = familyExp.cap(1);
 family = familyExp.cap(2);
 if (fontInfo().family() != family || fontInfo().fontPlugin() != plugin) {
	familyChanged = true;
 }
 d->mFontInfo.setFontPlugin(plugin);
 d->mFontInfo.setFamily(family);
 d->mFontInfo.setStyle(selectedStyles());
 d->mFontInfo.setPointSize((float)mSizes->selectedText().toUInt());

 if (familyChanged) {
	// TODO
	// int supportedStyles = 0;
	// ...
	// setSupportedStyles(supportedStyles);
 }

 mPreview->setFont(fontInfo());
}

int BoUfoFontSelectionWidget::selectedStyles() const
{
 int styles = 0;
 Q3ValueList<unsigned int> list = mStyles->selectedItems();
 Q3ValueList<unsigned int>::iterator it;
 for (it = list.begin(); it != list.end(); ++it) {
	if (d->mIndex2Style.contains(*it)) {
		styles |= d->mIndex2Style[*it];
	}
 }
 return styles;
}

void BoUfoFontSelectionWidget::setSelectedStyles(int styles)
{
 mStyles->unselectAll();
 QMap<int, int>::iterator it = d->mIndex2Style.begin();
 for (; it != d->mIndex2Style.end(); ++it) {
	if (styles & it.data()) {
		mStyles->setItemSelected(it.key(), true);
	}
 }
}

void BoUfoFontSelectionWidget::setSupportedStyles(int supportedStyles)
{
 int selected = selectedStyles();

 QStringList styles;
 mStyles->clear();
 if (supportedStyles & BoUfoFontInfo::StyleItalic) {
	d->mIndex2Style.insert(styles.count(), BoUfoFontInfo::StyleItalic);
	styles.append(i18n("Italic"));
 }
 if (supportedStyles & BoUfoFontInfo::StyleBold) {
	d->mIndex2Style.insert(styles.count(), BoUfoFontInfo::StyleBold);
	styles.append(i18n("Bold"));
 }
 if (supportedStyles & BoUfoFontInfo::StyleUnderline) {
	d->mIndex2Style.insert(styles.count(), BoUfoFontInfo::StyleUnderline);
	styles.append(i18n("Underline"));
 }
 if (supportedStyles & BoUfoFontInfo::StyleStrikeOut) {
	d->mIndex2Style.insert(styles.count(), BoUfoFontInfo::StyleStrikeOut);
	styles.append(i18n("StrikeOut"));
 }

 mStyles->setItems(styles);
 setSelectedStyles(selected);
}

void BoUfoFontSelectionWidget::slotApply()
{
 emit signalFontSelected(fontInfo());
 emit signalApply();
}

void BoUfoFontSelectionWidget::slotOk()
{
 emit signalFontSelected(fontInfo());
 emit signalOk();

 // TODO: close dialog
}


