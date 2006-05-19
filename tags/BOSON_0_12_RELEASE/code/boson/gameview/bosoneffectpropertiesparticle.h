/*
    This file is part of the Boson game
    Copyright (C) 2004-2005 Rivo Laks (rivolaks@hot.ee)

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

#ifndef BOSONEFFECTPROPERTIESPARTICLE_H
#define BOSONEFFECTPROPERTIESPARTICLE_H


#include "bosoneffectproperties.h"

#include <qdict.h>

#include "bosoneffect.h"
#include "bo3dtools.h"


class KSimpleConfig;
class QString;
class BoTextureArray;
class BosonEffectParticle;
class BosonParticle;


/**
 * @short Base class for particle effect properties
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesParticle : public BosonEffectProperties
{
  public:
    BosonEffectPropertiesParticle();


    //virtual BosonEffect::Type type()  { return mType; }

    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited);


    static void initStatic(const QString& texdir);


    inline static const BoVector3Float& wind()  { return mWind; }
    inline static void setWind(const BoVector3Float& w)  { mWind = w; }


  protected:
    static const BoTextureArray* getTextures(const QString& name);

    static QDict<BoTextureArray> mTextureArrays;
    static QString mTexturePath;
    static BoVector3Float mWind;
};



/**
 * @short Properties for generic particle effect.
 *
 * Generic particle effect is the simplest particle effect. It is not
 *  specialized for any special kind of particle system (e.g. smoke), so it can
 *  be used to create totally new and different-looking effects.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesParticleGeneric : public BosonEffectPropertiesParticle
{
  public:
    BosonEffectPropertiesParticleGeneric();


    virtual BosonEffect::Type type() const  { return BosonEffect::ParticleGeneric; }

    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;


    int initialParticles() const  { return mInitNum; }


    /**
     * Initializes given particle
     **/
    virtual void initParticle(BosonEffectParticle* effect, BosonParticle* particle) const;

    /**
     * Updates given particle
     **/
    virtual void updateParticle(BosonEffectParticle* effect, BosonParticle* particle, float elapsed) const;


  protected:
    void reset();


  protected:
    BoVector3Float mMinVelo, mMaxVelo;
    BoVector3Float mMinPos, mMaxPos;
    bool mNormalizePos, mNormalizeVelo;
    float mMinPosScale, mMaxPosScale, mMinVeloScale, mMaxVeloScale;
    BoVector4Float mStartColor, mEndColor;
    float mMinLife, mMaxLife;
    int mMaxNum, mInitNum;
    int mGLBlendFunc, mGLSrcBlendFunc;
    float mRate, mStartSize, mEndSize, mAge, mMass, mParticleDist;
    bool mAlign;
    bool mMoveParticlesWithSystem;
    QString mTextureName;
    QString mGLBlendFuncStr, mGLSrcBlendFuncStr;
    const BoTextureArray* mTextures;
};



/**
 * @short Properties class for BosonEffectParticleTrail
 *
 * See @ref BosonEffectParticleTrail for info about trail particle effect.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesParticleTrail : public BosonEffectPropertiesParticle
{
  public:
    BosonEffectPropertiesParticleTrail();


    virtual BosonEffect::Type type() const  { return BosonEffect::ParticleTrail; }

    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;


    /**
     * Initializes given particle
     **/
    virtual void initParticle(BosonEffectParticle* effect, BosonParticle* particle) const;

    /**
     * Updates given particle
     **/
    virtual void updateParticle(BosonEffectParticle* effect, BosonParticle* particle, float elapsed) const;


  protected:
    void reset();


  protected:
    float mSpacing;
    BoVector3Float mMinVelo, mMaxVelo;
    BoVector3Float mMinOffset, mMaxOffset;
    float mStartSize;
    float mEndSize;
    BoVector4Float mStartColor;
    BoVector4Float mEndColor;
    float mMass, mParticleDist;
    float mMaxSpeed;
    float mMinLife, mMaxLife;
    QString mTextureName;
    QString mGLBlendFuncStr, mGLSrcBlendFuncStr;
    int mGLBlendFunc, mGLSrcBlendFunc;
    const BoTextureArray* mTextures;
};



/**
 * @short Properties class for BosonEffectParticleEnvironmental
 *
 * See @ref BosonEffectParticleEnvironmental for info about environmental
 *  particle effects.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectPropertiesParticleEnvironmental : public BosonEffectPropertiesParticle
{
  public:
    BosonEffectPropertiesParticleEnvironmental();


    virtual BosonEffect::Type type() const  { return BosonEffect::ParticleEnvironmental; }

    virtual bool load(KSimpleConfig* cfg, const QString& group, bool inherited = false);


    virtual BosonEffect* newEffect(const BoVector3Fixed& pos, const BoVector3Fixed& rot = BoVector3Fixed()) const;


    /**
     * Initializes given particle
     **/
    virtual void initParticle(BosonEffectParticle* effect, BosonParticle* particle) const;

    /**
     * Updates given particle
     **/
    virtual void updateParticle(BosonEffectParticle* effect, BosonParticle* particle, float elapsed) const;


  protected:
    void reset();


  protected:
    BoVector3Float mMinVelo, mMaxVelo;
    float mSize;
    BoVector4Float mColor;
    float mMass, mParticleDist;
    QString mTextureName;
    QString mGLBlendFuncStr, mGLSrcBlendFuncStr;
    int mGLBlendFunc, mGLSrcBlendFunc;
    float mDensity;
    float mRange;
    const BoTextureArray* mTextures;
};


#endif //BOSONEFFECTPROPERTIESPARTICLE_H

