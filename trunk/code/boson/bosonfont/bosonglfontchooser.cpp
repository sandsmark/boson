/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

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

#define QT_CLEAN_NAMESPACE

#include "bosonglfontchooser.h"
#include "bosonglfontchooser.moc"

#include "bodebug.h"
#include "bosonglfont.h"
#include "../bosonglwidget.h"
#include "../bo3dtools.h"

#include <knuminput.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <qfont.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qptrdict.h>
#include <qvbox.h>
#include <qlayout.h>

#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/Xlib.h>
#include <math.h>

class BosonGLFontPreview : public BosonGLWidget
{
//  Q_OBJECT
public:
    BosonGLFontPreview(QWidget* parent);
    ~BosonGLFontPreview();

    virtual void paintGL();
    BoFontInfo setFont(const BoFontInfo& font);

protected:
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);

private:
    BosonGLFont* mFont;
};

BosonGLFontPreview::BosonGLFontPreview(QWidget* parent)
    : BosonGLWidget(parent, true)
{
 mFont = 0;
}

BosonGLFontPreview::~BosonGLFontPreview()
{
 makeCurrent();
 delete mFont;
}

BoFontInfo BosonGLFontPreview::setFont(const BoFontInfo& font)
{
 boDebug() << k_funcinfo << font.guiName() << endl;
 QString errorString, errorName;
 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error before setFont(): string=" << errorString << " name=" << errorName << endl;
 }
 makeCurrent();
 delete mFont;
 mFont = new BosonGLFont();
 if (!mFont->loadFont(font)) {
    boError() << k_funcinfo << "Unable to load font" << endl;
    return BoFontInfo();
 }

 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error at the end of setFont() before slotUpdateGL(): string=" << errorString << " name=" << errorName << endl;
 }
 slotUpdateGL();
 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error at the end of setFont(): string=" << errorString << " name=" << errorName << endl;
 }
 return mFont->fontInfo();
}

void BosonGLFontPreview::initializeGL()
{
 if (isInitialized()) {
    return;
 }
 QString errorString, errorName;
 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error before initializeGL(): string=" << errorString << " name=" << errorName << endl;
 }
 glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
 glDisable(GL_DITHER);
 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error at the end of initializeGL(): string=" << errorString << " name=" << errorName << endl;
 }
}

void BosonGLFontPreview::resizeGL(int w, int h)
{
 if (!isInitialized()) {
    initGL();
 }
 QString errorString, errorName;
 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error before resizeGL(): string=" << errorString << " name=" << errorName << endl;
 }
 makeCurrent();
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 glViewport(0, 0, w, h);
 gluOrtho2D(0.0, (GLfloat)width(), 0.0, (GLfloat)height());
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error at the end of resizeGL(): string=" << errorString << " name=" << errorName << endl;
 }
}

void BosonGLFontPreview::paintGL()
{
 if (!mFont) {
    return;
 }
 if (!isInitialized()) {
    initGL();
 }
 QString errorString, errorName;
 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error before paintGL(): string=" << errorString << " name=" << errorName << endl;
 }
 glLoadIdentity();
 glColor3ub(255, 255, 255);
 glClear(GL_COLOR_BUFFER_BIT);
 QString text = "foobar";
 int x = width() / 2 - mFont->width(text) / 2;
 int maxWidth = width() - x;
 int y = height() / 2 + mFont->height(text, maxWidth) / 2;
 mFont->begin();
 mFont->renderText(x, y, text, maxWidth);
 if (Bo3dTools::checkError(0, &errorString, &errorName)) {
    boError() << k_funcinfo << "OpenGL error at the end of paintGL(): string=" << errorString << " name=" << errorName << endl;
 }
}

class BosonGLFontChooserPrivate
{
public:
    BosonGLFontChooserPrivate()
    {
        mGLPreview = 0;
    }
    QPtrDict<BoFontInfo> mItem2Font;
    BosonGLFontPreview* mGLPreview;
};

BosonGLFontChooser::BosonGLFontChooser(QWidget* parent, const char* name)
    : BosonGLFontChooserBase(parent, name)
{
 d = new BosonGLFontChooserPrivate;
 d->mItem2Font.setAutoDelete(true);
 mUseTexturedFontCheckbox->setChecked(true);
 mUseTexturedFontCheckbox->hide(); // not (yet) used

 QVBoxLayout* previewLayout = new QVBoxLayout(mFontPreview);
 d->mGLPreview = new BosonGLFontPreview(mFontPreview);
 previewLayout->addWidget(d->mGLPreview);
 d->mGLPreview->setMinimumHeight(100);
 d->mGLPreview->setMinimumWidth(300);

 loadFonts();
}

BosonGLFontChooser::~BosonGLFontChooser()
{
 d->mItem2Font.clear();
 delete d;
}


void BosonGLFontChooser::loadFonts()
{
 d->mItem2Font.clear();
 loadGLXFonts();
 loadTXFFonts();
}

void BosonGLFontChooser::loadTXFFonts()
{
 boDebug() << k_funcinfo << endl;
 QStringList list = KGlobal::dirs()->findAllResources("data", "boson/fonts/*.txf");
 if (list.isEmpty()) {
    boDebug() << k_funcinfo << "no .txf fonts available. can't use textured fonts." << endl;
    return;
 }
 for (unsigned int i = 0; i < list.count(); i++) {
    QListBoxText* t = new QListBoxText(mFontNameList, list[i]);
    BoFontInfo* f = new BoFontInfo();
    f->setName(list[i]);
    f->setTextured(true);
    d->mItem2Font.insert(t, f);
 }
}

void BosonGLFontChooser::loadGLXFonts()
{
 boDebug() << k_funcinfo << endl;
 // dummy implementation. we should list all available fonts that we have a
 // QFont::handle() for.
 QFont font("fixed");
 font.setStyleHint(QFont::AnyStyle, QFont::PreferBitmap);
 font.setFixedPitch(true); // seems to be necessary on some systems. Do NOT remove that unless you know that it works without on such systems. We use textured fonts for non-fixed width
 if (font.handle() == 0) {
    boWarning() << k_funcinfo << "handle is 0 - trying to find a usable font..." << endl;
    int handle = font.handle();
    int count = 0;
    char** names = XListFonts(QPaintDevice::x11AppDisplay(), "*.txf", 0xffff, &count);
    for (int i = 0; i < count && handle == 0; i++) {
        font.setRawName(names[i]);
        handle = (int)font.handle();
    }
    XFreeFontNames(names);
 }
 if (font.handle() == 0) {
    boError() << k_funcinfo << "no bitmap font found" << endl;
 } else {
    QListBoxText* t = new QListBoxText(mFontNameList, font.family());
    BoFontInfo* f = new BoFontInfo();
    f->setName(font.rawName());
    f->setTextured(false);
    d->mItem2Font.insert(t, f);
 }
}

void BosonGLFontChooser::slotUseTexturedFonts(bool e)
{
}

void BosonGLFontChooser::slotFontChanged()
{
 BoFontInfo f = font();
 d->mGLPreview->setFont(f);
 mFontSizeNumInput->setEnabled(f.textured());
 mFontSizeList->setEnabled(f.textured());
 mFontStyleList->setEnabled(f.textured());
}

void BosonGLFontChooser::slotFontStyleChanged()
{
 d->mGLPreview->setFont(font());
}

void BosonGLFontChooser::slotFontSizeChanged(QListBoxItem* item)
{
 if (!item) {
    return;
 }
 QString t = item->text();
 bool ok = false;
 int v = t.toInt(&ok);
 if (!ok) {
    boError() << k_funcinfo << "invalid number" << endl;
    return;
 }
 mFontSizeNumInput->setValue(v);
 d->mGLPreview->setFont(font());
}

void BosonGLFontChooser::slotFontSizeChanged(int size)
{
 d->mGLPreview->setFont(font());
}

// AB: unused
void BosonGLFontChooser::slotFontPreview()
{
}

int BosonGLFontChooser::getFont(BoFontInfo& font, QWidget* parent)
{
 KDialogBase dlg(parent, "glfontchooserdlg", true, i18n("Select Font"),
        KDialogBase::Ok | KDialogBase::Cancel);
 QVBox* page = dlg.makeVBoxMainWidget();
 BosonGLFontChooser* chooser = new BosonGLFontChooser(page, "glfontchooser");
 chooser->setFont(font);
 int result = dlg.exec();
 if (result == QDialog::Accepted) {
    font = chooser->font();
 }
 return result;
}

void BosonGLFontChooser::setFont(const BoFontInfo& requested)
{
 BoFontInfo font;
 QPtrDictIterator<BoFontInfo> it(d->mItem2Font);
 while (it.current()) {
    if (it.current()->name() == requested.name()) {
        mFontNameList->setSelected((QListBoxText*)it.currentKey(), true);
        font.setName(requested.name());
        break;
    }
    ++it;
 }
 mFontSizeNumInput->setValue(requested.pointSize());
 font.setPointSize(requested.pointSize());
 if (requested.italic()) {
    // TODO
    // ...
    // font.setItalic(true);
 }
 if (requested.bold()) {
    // TODO
    // ...
    // font.setBold(true);
 }
 if (requested.underline()) {
    // TODO
    // ...
    // font.setUnderline(true);
 }
 if (requested.strikeOut()) {
    // TODO
    // ...
    // font.setStrikeOut(true);
 }
 BoFontInfo actualFont = d->mGLPreview->setFont(font);
 if (!actualFont.isEqual(font)) {
    boDebug() << k_funcinfo << "actual font differs - updating settings..." << endl;
    static bool isRecursive = false;
    if (isRecursive) {
        boWarning() << k_funcinfo << "recursive call - updating settings failed?" << endl;
    } else {
        isRecursive = true;
        setFont(actualFont);
        isRecursive = false;
    }
 }
}

BoFontInfo BosonGLFontChooser::font() const
{
 BoFontInfo f;

 QListBoxText* item = (QListBoxText*)mFontNameList->selectedItem();
 if (item) {
    BoFontInfo* font = d->mItem2Font[item];
    if (!font) {
        boError() << k_funcinfo << "don't know that font (internal error)" << endl;
        return f;
    }
    f = *font;
 } else {
    boWarning() << k_funcinfo << "no font selected ?! -> we will use default font" << endl;
 }

 // TODO
 f.setBold(false);
 f.setItalic(false);

 if (mFontSizeNumInput->isEnabled()) {
    // AB: might be disabled for GLX fonts
    f.setPointSize(mFontSizeNumInput->value());
 }
 return f;
}

