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

#ifndef BOSONGLFONTCHOOSER_H
#define BOSONGLFONTCHOOSER_H

#include <GL/gl.h>

#include <qdialog.h> // for checing for QDialog::Accepted in getFont()

#include <bosonfont/bosonglfontchooserbase.h>

class BoFont;
class BoFontInfo;

class BosonGLFontChooserPrivate;

class BosonGLFontChooser : public BosonGLFontChooserBase
{
    Q_OBJECT
public:
    BosonGLFontChooser(QWidget* parent = 0, const char* name = 0);
    ~BosonGLFontChooser();

    static int getFont(BoFontInfo& font, QWidget* parent = 0);

    void setFont(const BoFontInfo& font);

    /**
     * @return An object describing the selected font
     **/
    BoFontInfo font() const;

protected:
    void loadFonts();
    void loadTXFFonts();


protected slots:
    virtual void slotUseTexturedFonts(bool);
    virtual void slotFontChanged();
    virtual void slotFontStyleChanged();
    virtual void slotFontSizeChanged(QListBoxItem*);
    virtual void slotFontSizeChanged(int);
    virtual void slotFontPreview();

private:
    BosonGLFontChooserPrivate* d;
};

#endif

