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

#ifndef BOLIGHT_H
#define BOLIGHT_H

#include "bo3dtools.h"


template<class T> class QValueVector;

/**
 *
 **/
class BoLightManager
{
  public:
    static int nextFreeId();
    static void setIdUsed(int id, bool used);

  private:
    static void init();

    static bool mInited;
    static QValueVector<bool> mIds;
};

/**
 * Class for OpenGL lights
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoLight
{
  public:
    BoLight();
    ~BoLight();

    bool isEnabled()  { return mEnabled; }
    void setEnabled(bool e);

    const BoVector4& ambient()  { return mAmbient; }
    const BoVector4& diffuse()  { return mDiffuse; }
    const BoVector4& specular()  { return mSpecular; }

    void setAmbient(BoVector4 a);
    void setDiffuse(BoVector4 d);
    void setSpecular(BoVector4 s);

    const BoVector4& position()  { return mPos; }
    void setPosition(BoVector4 pos);

    BoVector3 position3()  { return BoVector3(mPos.x(), mPos.y(), mPos.z()); }
    void setPosition3(BoVector3 pos)  { setPosition(BoVector4(pos.x(), pos.y(), pos.z(), mPos.w())); };

    int id()  { return mId; }

    void refreshPosition();

  private:
    BoVector4 mAmbient;
    BoVector4 mDiffuse;
    BoVector4 mSpecular;
    BoVector4 mPos;
    bool mEnabled;
    int mId;
};

#endif // BOLIGHT_H
