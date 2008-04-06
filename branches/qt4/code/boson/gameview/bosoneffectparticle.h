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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef BOSONEFFECTPARTICLE_H
#define BOSONEFFECTPARTICLE_H


#include "bosoneffect.h"

#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3PtrCollection>

#include "bo3dtools.h"


class BosonEffectPropertiesParticle;
class QDomElement;
class BosonEffectPropertiesParticleGeneric;
class BoTextureArray;
class BoTexture;
class BosonEffectParticle;
class BosonEffectPropertiesParticleTrail;
class BosonEffectPropertiesParticleEnvironmental;


/**
 * @short Base class for particles
 *
 * Particle base class only provides properties necessary for rendering.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonParticle
{
  public:
    /**
     * Construct new particle.
     * This only calls @ref reset()
     **/
    BosonParticle()
    {
      reset();
    }

    virtual ~BosonParticle()  {}

    /**
     * Reset all variables of the particle.
     * If you add variables in the subclass, then also reimplement this method
     *  and reset new variables there.
     **/
    virtual void reset()
    {
      color.reset();
      pos.reset();
      size = 0.0f;
      life = -1.0f;
      tex = 0;
      system = 0;
      distance = 0.0f;
    }


    /**
     * Current color of particle.
     */
    BoVector4Float color;
    /**
     * Current position of particle in OpenGL (world) coordinates (absolute
     *  coordinates).
     **/
    BoVector3Float pos;
    /**
     * Current size (diameter) of particle. When it's 1.0, particle is as big as
     *  one cell.
     **/
    float size;
    /**
     * How many remaining seconds particles has left to live, before it dies
     * If it is -1.0, particle is already dead (inactive) and will be ignored
     * (not rendered/updated).
     **/
    float life;
    /**
     * Current texture. This is cached here for performance reasons.
     **/
    BoTexture* tex;
    /**
     * Parent effect of this particle.
     **/
    BosonEffectParticle* system;
    /**
     * Square of distance of particle from the camera. This is cached here to
     * improve performance.
     **/
    float distance;
};


/**
 * @short Base class for particle effects
 *
 * This class mostly holds variables needed for rendering particle effects.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectParticle : public BosonEffect
{
  public:
    BosonEffectParticle(const BosonEffectPropertiesParticle* prop);


    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


    const BosonEffectPropertiesParticle* properties() const  { return mProperties; }


    /**
     * @return Radius of the bounding sphere of this particle effect.
     **/
    float boundingSphereRadius() const  { return mBoundingSphereRadius; }
    /**
     * @return Int array, containing two variables which specify OpenGL
     *  blending function which should be used to render this effect.
     **/
    const int* blendFunc() const  { return mBlendFunc; }
    /**
     * @return How much particles of this system must be moved towards the
     *  camera.
     **/
    float particleDist() const  { return mParticleDist; }
    void setParticleDist(float d)  { mParticleDist = d; }

    /**
     * @return Vector by which particles are moved towards the camera.
     * This must be precalculated and set using @ref setParticleDistVector every
     *  time position of camera or particle system changes.
     **/
    const BoVector3Float& particleDistVector() const  { return mParticleDistVector; }
    /**
     * Sets particle distance vector (specifies how much particles are "pulled")
     *  towards the camera.
     * @see particleDistVector
     **/
    void setParticleDistVector(const BoVector3Float& v)  { mParticleDistVector = v; }

    /**
     * @return Particle at position i in the particles list.
     * Must be reimplemented in child classes.
     * Usually you'll probably want to hold all particles in a single array and
     *  then this method would return something like  mParticles[i]
     **/
    virtual BosonParticle* particle(unsigned int i) = 0;
    /**
     * @return Number of particles in the particle array.
     * Note that this does _not_ show number of active particles, but rather
     *  maximum number of active particles there can be once.
     **/
    unsigned int particleCount() const  { return mParticleCount; }
    /**
     * @return Whether to align the particles to the camera.
     * Unaligned particles will be aligned to xy-plane.
     **/
    bool alignParticles() const  { return mAlign; }

    /**
     * @return Whether to test if particle system is fogged before rendering.
     * Fogged particle systems (the ones that player can't see) won't be
     *  rendered unless this method returns false.
     **/
    virtual bool testFogged() const  { return true; }

    virtual void setPosition(const BoVector3Fixed& pos);
    const BoVector3Float& positionFloat()  { return mPositionFloat; }

  protected:
    float mBoundingSphereRadius;
    int mBlendFunc[2];
    float mParticleDist;
    BoVector3Float mParticleDistVector;
    unsigned int mParticleCount;
    const BosonEffectPropertiesParticle* mProperties;
    bool mAlign;  // Whether to align particles to camera
    BoVector3Float mPositionFloat;
};



/**
 * @short Particle class for generic particle effect.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonGenericParticle : public BosonParticle
{
  public:
    virtual void reset()
    {
      BosonParticle::reset();
      velo.reset();
      maxage = 0.0f;
    }

    /**
     * Current velocity i.e. how much particle moves per second.
     **/
    BoVector3Float velo;
    /**
     * Maximum lifetime of this particle.
     * @see life
     **/
    float maxage;
};


/**
 * @short Generic particle effect
 *
 * Generic particle effect can be used if none of the other
 *  @ref BosonEffectParticle subclasses is suitable for you. This is the most
 *  customizable particle effect, but it may also require more work to get it
 *  looking right.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectParticleGeneric : public BosonEffectParticle
{
  public:
    /**
     * Creates new generic particle effect.
     * @param prop Pointer to properties object of this effect.
     * @param maxnum Maximum number of particles this effect can have.
     * @param textures Texture array for this effect.
     **/
    BosonEffectParticleGeneric(const BosonEffectPropertiesParticleGeneric* prop, int maxnum,
        const BoTextureArray* textures);
    virtual ~BosonEffectParticleGeneric();


    virtual Type type() const  { return BosonEffect::ParticleGeneric; }


    /**
     * Updates all particles within this system and creates new ones if needed
     * and if age is more than 0.
     **/
    virtual void update(float elapsed);

    virtual void start();


    virtual BosonParticle* particle(unsigned int i)  { return &mParticles[i]; }

    /**
     * Modifies rotation matrix of this particle system.
     *
     * @see matrix
     **/
    virtual void setRotation(const BoVector3Fixed& rotation);

    virtual void setPosition(const BoVector3Fixed& pos);


    /**
     * @return If rotation matrix of this system has been modified e.g. if
     *   @ref setRotation has been called.
     **/
    inline bool isRotated() const  { return mRotated; }

    /**
     * @return Rotation matrix of this system.
     * Rotation matrix is used to calculate initial position and velocity of
     *  particle when it's initialized. This feature is used by e.g. missile
     *  trails.
     **/
    inline const BoMatrix& matrix() const  { return mMatrix; }


    /**
     * Sets age of this system (in seconds).
     * Age is decreased every time @ref update is called and if it's 0, no new
     * particles are created anymore
     * -1 means infinite time
     * @param a New age of this system
     **/
    void setAge(float a)  { mAge = a; }
    float age() const  { return mAge; }

    /**
     * Sets @ref age of this effect to 0.
     * For effects with infinite lifetime, you will need to use this to end
     *  creation of new particles.
     **/
    virtual void makeObsolete()  { setAge(0); BosonEffect::makeObsolete(); }

    /**
     * Return mass of the particles of this particle system.
     * It defines how much the wind affects particles.
     **/
    inline float mass() const  { return mMass; }
    /**
     * Set the mass of the particles of this system
     * @see mass
     **/
    void setMass(float m)  { mMass = m; }

    /**
     * Set rate of this effect. This defines number of particles emitted per
     *  second.
     **/
    void setRate(float rate)  { mRate = rate; }

    void setAlignParticles(bool align)  { mAlign = align; }

    /**
     * Set the maximum particle size of this effect.
     * This will be used to calculate radius of the bounding sphere of this
     *  effect.
     **/
    void setMaxParticleSize(float size)  { mMaxParticleSize = size; }

    /**
     * @return Whether this system is active
     * Active means that either this system has at least 1 living (active)
     *  particle or it's age is not 0 (can create more particles) or it's
     *  delayed (not yet started)
     * Only active systems have to be updated.
     **/
    virtual bool isActive() const  { return ((mNum > 0) || (mAge != 0.0f) || (!hasStarted())); }

    /**
     * Sets OpenGL blending function of this system. This function is used in
     * @ref draw mehod. You can use this to specify your own blending functions
     * if needed. Defaults are GL_SRC_ALPHA and GL_ONE_MINUS_SRC_ALPHA.
     * @param sf Source blend function
     * @param df Destination blend function
     **/
    void setBlendFunc(int sf, int df) { mBlendFunc[0] = sf; mBlendFunc[1] = df; }

    void setMoveParticlesWithSystem(bool move)  { mMoveParticlesWithSystem = move; }

    /**
     * Creates specified number of new particles.
     * @param count How many new particles to create
     **/
    void createParticles(int count);


    virtual int maxDelayedUpdates() const  { return mMaxDelayedUpdates; }
    void setMaxDelayedUpdates(int n)  { mMaxDelayedUpdates = n; }

    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    /**
     * Called when new particle is made active and needs to be initialized
     **/
    virtual void initParticle(BosonGenericParticle* particle);
    /**
     * Called when particle has to be updated. Called from @ref update
     **/
    virtual void updateParticle(BosonGenericParticle* particle, float elapsed);


  private:
    BosonGenericParticle* mParticles;  // Array of particles
    int mNum;  // Current number of particles (aka number of active particles)
    float mRate;  // Number of particles created per second
    float mCreateCache;  // Number of particles to create during next update
    bool mMoveParticlesWithSystem;  // Particles are moved with system if true
    const BoTextureArray* mTextures;  // Textures of particles
    bool mRotated;
    float mAge;
    float mMass;
    float mMaxParticleSize;
    BoMatrix mMatrix;
    int mMaxDelayedUpdates;
};



/**
 * @short Trail particle effect.
 *
 * This can be used to draw realistic smoke (or other) trails of missiles. It
 *  is special in that it lets you define fixed spacing between every two
 *  particles, and whenever the system is moved to a new position, enough
 *  particles will be emitted, so that there will be exactly one particle every
 *  specified units on a line from the old position to new one.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectParticleTrail : public BosonEffectParticle
{
  public:
    /**
     * Constructs new trail particle effect.
     * @param prop Properties for this effect.
     * @param maxnum Maximum number of particles in this effect.
     * @param textures Texture array for this effect
     * @param pos Initial position for this effect.
     **/
    BosonEffectParticleTrail(const BosonEffectPropertiesParticleTrail* prop, int maxnum,
        const BoTextureArray* textures, const BoVector3Fixed& pos);
    virtual ~BosonEffectParticleTrail();


    virtual Type type() const  { return BosonEffect::ParticleTrail; }


    /**
     * Updates all particles within this system and creates new ones if needed
     * and if age is more than 0.
     **/
    virtual void update(float elapsed);
    virtual bool supportsDelayedUpdates() const  { return false; }


    virtual BosonParticle* particle(unsigned int i)  { return &mParticles[i]; }

    /**
     * Modifies rotation matrix of this particle system.
     *
     * @see matrix
     **/
    virtual void setRotation(const BoVector3Fixed& rotation);

    /**
     * Set new position for this effect.
     * Note that new particles between old position and the new one will be
     *  created once the @ref update method is next called, so if you move the
     *  effect several times between two @ref update calls, only one "line of
     *  particles" is drawn.
     **/
    virtual void setPosition(const BoVector3Fixed& pos);


    /**
     * @return If rotation matrix of this system has been modified e.g. if
     *   @ref setRotation has been called.
     **/
    inline bool isRotated() const  { return mRotated; }

    /**
     * @return Rotation matrix of this system.
     * Rotation matrix is used to calculate initial position and velocity of
     * particle when it's initialized. This feature is used by missile traces.
     **/
    inline const BoMatrix& matrix() const  { return mMatrix; }


    /**
     * Makes the effect obsolete.
     * No more particles will be created after this.
     **/
    virtual void makeObsolete()  { mObsolete = true; BosonEffect::makeObsolete(); }
    virtual bool isActive() const  { return ((mNum > 0) || (!mObsolete)); }


    /**
     * Return mass of the particles of this particle system.
     * It defines how much the wind affects particles.
     **/
    inline float mass() const  { return mMass; }
    /**
     * Set the mass of the particles of this system
     * @see mass
     **/
    void setMass(float m)  { mMass = m; }

    // In world coords!
    float spacing() const  { return mSpacing; }
    void setSpacing(float s)  { mSpacing = s; }

    /**
     * @return Offset of the particles of this effect.
     * Offset defines how much particles should be moved from the position where
     *  we want to place them. You can use it to e.g. place a smoke trail behind
     *  the missile.
     **/
    const BoVector3Float& offset() const  { return mOffset; }
    void setOffset(const BoVector3Float& o)  { mOffset = o; }

    void setMaxParticleSize(float size)  { mMaxParticleSize = size; }


    /**
     * Sets OpenGL blending function of this system. This function is used in
     * @ref draw mehod. You can use this to specify your own blending functions
     * if needed. Defaults are GL_SRC_ALPHA and GL_ONE_MINUS_SRC_ALPHA.
     * @param sf Source blend function
     * @param df Destination blend function
     **/
    void setBlendFunc(int sf, int df) { mBlendFunc[0] = sf; mBlendFunc[1] = df; }


    virtual int maxDelayedUpdates() const  { return mMaxDelayedUpdates; }
    void setMaxDelayedUpdates(int n)  { mMaxDelayedUpdates = n; }

    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    /**
     * Called when new particle is made active and needs to be initialized
     **/
    virtual void initParticle(BosonGenericParticle* particle, const BoVector3Float& pos);
    /**
     * Called when particle has to be updated. Called from @ref update
     **/
    virtual void updateParticle(BosonGenericParticle* particle, float elapsed);


  protected:
    BosonGenericParticle* mParticles;  // Array of particles
    int mNum;  // Current number of particles (aka number of active particles)
    float mCreateCache;  // Number of particles to create during next update
    const BoTextureArray* mTextures;  // Textures of particles
    bool mRotated;
    float mMass;
    float mMaxParticleSize;
    BoMatrix mMatrix;
    bool mObsolete;  // If true, new particles won't be created anymore
    float mSpacing;
    int mMaxDelayedUpdates;

    BoVector3Float mOffset;

    BoVector3Float mLastPos;
};



/**
 * @short Environmental particle effect.
 *
 * Environmental effects, as the name implies, fill the whole environment (map)
 *  in Boson. Such effects include e.g. rain and snow.
 *
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonEffectParticleEnvironmental : public BosonEffectParticle
{
  public:
    /**
     * Constructs new trail particle effect.
     * @param prop Properties for this effect.
     * @param maxnum Maximum number of particles in this effect.
     * @param textures Texture array for this effect
     * @param pos Initial position for this effect.
     **/
    BosonEffectParticleEnvironmental(const BosonEffectPropertiesParticleEnvironmental* prop,
        float density, float range, const BoTextureArray* textures,
        const BoVector3Fixed& pos);
    virtual ~BosonEffectParticleEnvironmental();


    virtual Type type() const  { return BosonEffect::ParticleEnvironmental; }


    /**
     * Updates all particles within this system and creates new ones if needed
     * and if age is more than 0.
     **/
    virtual void update(float elapsed);
    virtual bool supportsDelayedUpdates() const  { return false; }


    virtual BosonParticle* particle(unsigned int i)  { return &mParticles[i]; }

    /**
     * Set new position for this effect.
     * Note that new particles between old position and the new one will be
     *  created once the @ref update method is next called, so if you move the
     *  effect several times between two @ref update calls, only one "line of
     *  particles" is drawn.
     **/
    virtual void setPosition(const BoVector3Fixed& pos);


    /**
     * Makes the effect obsolete.
     * No more particles will be created after this.
     **/
    virtual void makeObsolete()  { mObsolete = true; BosonEffect::makeObsolete(); }
    virtual bool isActive() const  { return ((mNum > 0) || (!mObsolete)); }


    /**
     * Return mass of the particles of this particle system.
     * It defines how much the wind affects particles.
     **/
    inline float mass() const  { return mMass; }
    /**
     * Set the mass of the particles of this system
     * @see mass
     **/
    void setMass(float m)  { mMass = m; }

    float range() const  { return mRange; }
    void setRange(float r)  { mRange = r; }

    /**
     * @return Average velocity of particles.
     * Note that this doesn't take wind into account
     **/
    const BoVector3Float& particleVelo() const  { return mParticleVelo; }
    void setParticleVelo(const BoVector3Float& velo)  { mParticleVelo = velo; }


    /**
     * Sets OpenGL blending function of this system. This function is used in
     * @ref draw mehod. You can use this to specify your own blending functions
     * if needed. Defaults are GL_SRC_ALPHA and GL_ONE_MINUS_SRC_ALPHA.
     * @param sf Source blend function
     * @param df Destination blend function
     **/
    void setBlendFunc(int sf, int df) { mBlendFunc[0] = sf; mBlendFunc[1] = df; }

    // Always render
    virtual bool testFogged() const  { return false; }


    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);


  protected:
    /**
     * Called when new particle is made active and needs to be initialized
     **/
    virtual void initParticle(BosonGenericParticle* particle, const BoVector3Float& pos);
    /**
     * Called when particle has to be updated. Called from @ref update
     **/
    virtual void updateParticle(BosonGenericParticle* particle, float elapsed);

    void particleBoxMoved(const BoVector3Float& oldpos, const BoVector3Float& newpos);


  protected:
    BosonGenericParticle* mParticles;  // Array of particles
    int mNum;  // Current number of particles (aka number of active particles)
    const BoTextureArray* mTextures;  // Textures of particles
    float mMass;
    bool mObsolete;  // If true, new particles won't be created anymore
    float mRange;
    float mDensity;
    BoVector3Float mParticleVelo;
};



/**
 * @short List of particles
 * This is simply QPtrList of particles. Only reimplemented method is
 * compareItems, which enables you to sort the list by distance from camera.
 * Note that you must set @ref BosonParticle::distance values first, they are
 * not automatically calculated.
 *
 * @see BosonParticle
 * @see BosonEffectParticle
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoParticleList : public Q3PtrList<BosonParticle>
{
  public:
    /**
     * Compares two particles by distance using @ref BosonParticle::distance
     */
    virtual int compareItems(Q3PtrCollection::Item item1, Q3PtrCollection::Item item2);
};


#endif //BOSONEFFECTPARTICLE_H

