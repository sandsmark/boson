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

#ifndef BOSONPARTICLESYSTEMPROPERTIES_H
#define BOSONPARTICLESYSTEMPROPERTIES_H

#include <krandomsequence.h>

#include <qstring.h>

#include <GL/gl.h>

#include "bo3dtools.h"

class BosonTextureArray;
class BosonParticleSystem;
class BosonParticle;
class QString;
class KSimpleConfig;
class KConfig;
class SpeciesTheme;
template<class T, class T2> class QMap;
template<class T> class QPtrList;
template<class T> class QValueList;

/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonParticleSystemProperties
{
  public:
    BosonParticleSystemProperties(KSimpleConfig* cfg);
    ~BosonParticleSystemProperties();

    BosonParticleSystem* newSystem(BoVector3 pos) const;

    inline static float getFloat(float min, float max)  { return ((float)(mRandom->getDouble())) * (max - min) + min; };

    inline static BoVector3 wind()  { return BoVector3(0.25, 0.15, 0); };

    void initParticle(BosonParticleSystem* system, BosonParticle* particle) const;

    void updateParticle(BosonParticleSystem* system, BosonParticle* particle) const;

    static void init(const QString& texdir);

    unsigned long int id() const { return mId; };

    static QPtrList<BosonParticleSystemProperties> loadParticleSystemProperties(KSimpleConfig* cfg, QString key, SpeciesTheme* theme);
    static QPtrList<BosonParticleSystemProperties> loadParticleSystemProperties(QValueList<unsigned long int> ids, SpeciesTheme* theme);

  protected:
    static void addTexture(const QString& name);
    static GLuint texture(const QString& name);

  protected:
    static QMap<QString, GLuint> mTextures;
    static KRandomSequence* mRandom;
    static QString mTexturePath;

  private:
    /*float mMinXVelo, mMinYVelo, mMinZVelo;
    float mMaxXVelo, mMaxYVelo, mMaxZVelo;*/
    BoVector3 mMinVelo, mMaxVelo;
    BoVector3 mMinPos, mMaxPos;
    bool mNormalize;
    float mMinScale, mMaxScale;
    BoVector4 mStartColor, mEndColor;
    float mMinLife, mMaxLife;
    int mMaxNum, mInitNum;
    int mGLBlendFunc;
    float mRate, mSize, mAge;
    bool mAlign;
    QString mTextureName;
    unsigned long int mId;
};

#endif

/*
 * vim: et sw=2
 */
