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

#ifndef BOSONPARTICLEMANAGER_H
#define BOSONPARTICLEMANAGER_H

#include <krandomsequence.h>
#include <qmap.h>
#include <GL/gl.h>

#include "bo3dtools.h"

class BosonTextureArray;
class BosonParticleSystem;
class BosonParticle;
class QString;
class KSimpleConfig;

/*class BosonParticleManager
{
  public:
    static void loadTextures(QString texdir);

    enum Type { None = 0, Explosion = 1, Smoke = 2, Shot = 3, Fire = 4, SmallSmoke = 5, ShockWave = 6, LastType };

    static BosonParticleSystem* newSystem(BoVector3 pos, Type type);

    inline static BosonParticleSystem* newExplosion(BoVector3 pos) { return newSystem(pos, Explosion); };
    inline static BosonParticleSystem* newSmoke(BoVector3 pos) { return newSystem(pos, Smoke); };
    inline static BosonParticleSystem* newShot(BoVector3 pos) { return newSystem(pos, Shot); };
    inline static BosonParticleSystem* newFire(BoVector3 pos) { return newSystem(pos, Fire); };
    inline static BosonParticleSystem* newSmallSmoke(BoVector3 pos) { return newSystem(pos, SmallSmoke); };
    inline static BosonParticleSystem* newShockWave(BoVector3 pos) { return newSystem(pos, ShockWave); };

    static void initExplosionParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void initSmokeParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void initShotParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void initFireParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void initSmallSmokeParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void initShockWaveParticle(BosonParticleSystem* system, BosonParticle* particle);

    static void updateFadeOutParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void updateFireParticle(BosonParticleSystem* system, BosonParticle* particle);
    static void updateExplosionParticle(BosonParticleSystem* system, BosonParticle* particle);

    inline static float getFloat(float min, float max)  { return ((float)(mRandom->getDouble())) * (max - min) + min; };

  protected:
    static BosonTextureArray* mTextures;
   static KRandomSequence* mRandom;
};*/


/**
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonParticleSystemProperties
{
  public:
    BosonParticleSystemProperties(KSimpleConfig* cfg);
    ~BosonParticleSystemProperties();

    BosonParticleSystem* newSystem(float x, float y, float z);

    inline static float getFloat(float min, float max)  { return ((float)(mRandom->getDouble())) * (max - min) + min; };

    inline static BoVector3 wind()  { return BoVector3(0.25, 0.15, 0); };

    void initParticle(BosonParticleSystem* system, BosonParticle* particle);

    void updateParticle(BosonParticleSystem* system, BosonParticle* particle);
    
    static void init(QString texdir);
    
    unsigned long int id()  { return mId; };

  protected:
    static void addTexture(QString name);
    static GLuint texture(QString name);

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

#endif // BOSONPARTICLEMANAGER_H
