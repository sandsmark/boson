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

#ifndef BOSONPARTICLESYSTEM_H
#define BOSONPARTICLESYSTEM_H

#include <qptrlist.h>

#include "bo3dtools.h"

#include "bosonparticlesystemproperties.h"

class QDomElement;


/**
 * @short List of particles
 * This is simply QPtrList of particles. Only reimplemented method is
 * compareItems, which enables you to sort the list by distance from camera.
 * Note that you must set @ref BosonParticle::distance values first, they are
 * not automatically calculated.
 *
 * @see BosonParticle
 * @see BosonParticleSystem
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BoParticleList : public QPtrList<BosonParticle>
{
  public:
    /**
     * Compares two particles by distance using @ref BosonParticle::distance
     */
    virtual int compareItems(QPtrCollection::Item item1, QPtrCollection::Item item2);
};

/**
 * @short This class represents a single particle.
 * Particle is part of particle system. Each particle has individual color,
 * position, velocity, size, age, maximum age, texture, it's parent system and
 * current distance from camera.
 * In BosonParticle, all these are public variables which means that you can
 * easily access them.
 * Note that velocity is velocity per second and life and maxage are also in
 * seconds. Position, velocity and size are in OpenGL coordinates
 *
 * @see BosonParticleSystem
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonParticle
{
  public:
    /**
     * Constructs particle. Only life is set to -1, other variables remain
     * uninitialized (call @ref reset to reset them).
     **/
    BosonParticle();
    ~BosonParticle();

    /**
     * Updates particle. This decreases it's lifetime (when lifetime is 0,
     * particle dies) and updates positon accordingly to velocity.
     * @param elapsed How much time in seconds has elapsed since last update.
     **/
    void update(float elapsed);
    /**
     * Resets all variables. Usually there is no need to call this.
     **/
    void reset();

    /**
     * Current color of particle
     */
    BoVector4 color;
    /**
     * Current position of particle in OpenGL coordinates
     **/
    BoVector3 pos;
    /**
     * Current velocity i.e. how much particle moves per second
     **/
    BoVector3 velo;
    /**
     * Current size of particle. When it's 1.0, particle is as big as one cell
     **/
    float size;
    /**
     * How many remaining seconds particles has left to live, before it dies
     * If it is -1.0, particle is already dead (inactive).
     * @see maxage
     **/
    float life;
    /**
     * Maximum lifetime of this particle
     * @see life
     **/
    float maxage;
    /**
     * Current texture. This is cached here for performance reasons
     **/
    GLuint tex;
    /**
     * Parent system of this particle
     **/
    BosonParticleSystem* system;
    /**
     * Square of distance of particle from the camera. This is cached here to
     * improve performance
     **/
    float distance;
};


/**
 * @short This class takes care of entire particle system (collection of similar particles)
 *
 * When should you use one particle system and when more? One particle system
 * should only hold similar particles. This means that if you e.g. have a unit
 * burning and smoking, you should use two particle systems, because smoke and
 * flames are very different things. It doesn't matter that both are used for
 * the same unit.
 * 
 * BosonParticleSystem uses @ref BosonParticleSystemProperties to initialize
 * and update particles. This should provide enough flexibility for normal
 * usage. If you want even more flexibility, then you can write you own
 * subclass; most important methods are virtual, so you can change them if you
 * really want to.
 *
 * Note that this class uses OpenGL coordinates.
 *
 * @see BosonParticle
 * @author Rivo Laks <rivolaks@hot.ee>
 **/
class BosonParticleSystem
{
  public:
    /**
     * Constructs new BosonParticleSystem.
     * @param maxnum Maximum number of particles this system may have. Note that
     *   if you specify too big number, BosonParticleSystem will create too big
     *   array of particles which is bad for both performance and memory usage.
     * @param createrate How many additional particles are created each second.
     * @param align Whether to align particles to camera. If false, particles
     *   are aligned to XY plane.
     * @param textures @ref BosonTextureArray containing all textures
     *   that will be used by this particle system.
     * @param prop Properties of this system.
     **/
    BosonParticleSystem(int maxnum, float createrate,
        bool align, const BosonTextureArray* textures,
        const BosonParticleSystemProperties* prop);
    /**
     * Destructs BosonParticleSystem. This deletes all particles
     **/
    virtual ~BosonParticleSystem();

    /**
     * Updates all particles within this system and creates new ones if needed
     * and if age is more than 0.
     **/
    virtual void update(float elapsed);

    /**
     * Sets current position of this system.
     *
     * @param p New position of this system in OpenGL coordinates
     **/
    void setPosition(BoVector3 p);
    /**
     * @return Current position of this system in OpenGL coordinates
     **/
    inline const BoVector3& position()  { return mPos; };

    /**
     * @return x-coordinate of this system in cell coordinates
     **/
     inline int x() const  { return (int)(mPos[0]); };
    /**
     * @return y-coordinate of this system in cell coordinates
     **/
     inline int y() const  { return (int)(-mPos[1]); };

    /**
     * Modifies rotation matrix of this particle system.
     *
     * @param angle How much to rotate
     * @param x, y, z  Around which angle to rotate
     *
     * @see matrix
     **/
    void setRotation(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    /**
     * @return If rotation matrix of this system has been modified e.g. if
     *   @ref setRotation has been called.
     **/
    inline bool isRotated()  { return mRotated; };

    /**
     * @return Rotation matrix of this system.
     * Rotation matrix is used to calculate initial position and velocity of
     * particle when it's initialized. This feature is used by missile traces.
     **/
    inline const BoMatrix& matrix()  { return mMatrix;};

    /**
     * @return Current age of this system
     **/
//    float age() { return mAge; };
    /**
     * Sets age of this system (in seconds).
     * Age is decreased every time @ref update is called and if it's 0, no new
     * particles are created anymore
     * -1 means infinite time
     * @param a New age of this system
     **/
    void setAge(float a) { mAge = a; };

    /**
     * Return mass of the particles of this particle system.
     * It defines how much the wind affects particles.
     **/
    inline float mass()  { return mMass; };

    /**
     * Set the mass of the particles of this system
     * @see mass
     **/
     void setMass(float m)  { mMass = m; };

    /**
     * @return Current creation rate of particles (per second)
     **/
//    float createRate() const { return mCreateRate; };
    /**
     * Sets current creation rate of particles (per second).
     * New particles are periodically created every @ref update call.
     * @param r New creation rate of particles
     **/
//    void setCreateRate(float r) { mCreateRate = r; };

    /**
     * @return Radius of bounding sphere of this system. This can be used for
     * culling.
     **/
    float boundingSphereRadius() const { return mRadius; };

    /**
     * @return Whether this system is active
     * Active means that either this system has at least 1 living (active)
     * particle or it's age is not 0 (can create more particles)
     **/
    bool isActive()  { return ((mNum > 0) || (mAge != 0)); };

    /**
     * Sets OpenGL blending function of this system. This function is used in
     * @ref draw mehod. You can use this to specify your own blending functions
     * if needed. Defaults are GL_SRC_ALPHA and GL_ONE_MINUS_SRC_ALPHA.
     * @param sf Source blend function
     * @param df Destination blend function
     **/
    void setBlendFunc(int sf, int df) { mBlendFunc[0] = sf; mBlendFunc[1] = df; };

    void setMoveParticlesWithSystem(bool move)  { mMoveParticlesWithSystem = move; };

    /**
     * @return Number of living (active) particles in this system
     **/
//    int particleCount() { return mNum; };

    /**
     * Creates specified number of new particles.
     * @param count How many new particles to create
     **/
    void createParticles(int count);

    virtual bool saveAsXML(QDomElement& root) const;
    virtual bool loadFromXML(const QDomElement& root);

  protected:
    /**
     * Initializes particle system (resets variables) and creates some new
     * particles
     *
     * @param initialnum Number of particles to create
     **/
    virtual void init(int initialnum);

    /**
     * Called when new particle is made active and needs to be initialized
     **/
    virtual void initParticle(BosonParticle* particle);
    /**
     * Called when particle has to be updated. Called from @ref update
     **/
    virtual void updateParticle(BosonParticle* particle);

    const BosonParticleSystemProperties* properties() const;

  protected:
    BosonParticle* mParticles;  // Array of particles
    int mMaxNum;  // Maximum number of particles
    int mNum;  // Current number of particle (aka number of active particles)
    float mCreateRate;  // Number of particles created per second
    float mCreateCache;  // Number of particles to create during next update
    bool mAlign;  // Whether to align particles to camera
    float mRadius;  // Radius of bounding sphere
    bool mMoveParticlesWithSystem;  // Particles are moved with system if true
    const BosonTextureArray* mTextures;  // Textures of particles
    BoVector3 mPos;
    bool mRotated;
    float mAge;
    float mMass;
    int mBlendFunc[2];

    const BosonParticleSystemProperties* mProp;

    BoMatrix mMatrix;  // Rotation matrix. Used to calculate position and
        //  velocity of particles when they're initialized. Stored here for
        //  perf. reasons
    BoVector3 mRotation;  // Euler angles for rotation

    friend class BosonBigDisplayBase;
};

#endif // BOSONPARTICLESYSTEM_H

