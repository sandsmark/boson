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

#include "bo3dtools.h"

class BosonParticleSystemProperties;


/**
 * @short This class represents a single particle.
 * Particle is part of particle system. Each particle has individual color,
 * position, velocity, size and age.
 * In BosonParticle, all these are public variables which means that you can
 * easily access them.
 * Note that velocity is velocity per second and life and maxage are also in seconds
 * @see BosonParticleSystem
 **/
class BosonParticle
{
  public:
    /**
     * Constructs particle. All properties are resetted (set to zero)
     **/
    BosonParticle();
    /**
     * Constructs particle and sets properties to given arguments
     **/
    BosonParticle(BoVector4 color, BoVector3 pos, BoVector3 velo, float size, float life = 0);
    ~BosonParticle();

    /**
     * Updates particle. This decreases it's lifetime (when lifetime is 0,
     * particle dies) and updates positon accordingly to velocity.
     * @param elapsed How much time in seconds has elapsed since last update.
     **/
    void update(float elapsed);
    void reset();

    BoVector4 color;  // Current color of particle
    BoVector3 pos;  // Current pos
    BoVector3 velo;  // Velocity per second
    float size;  // Current size
    float life;  // How many remaining seconds particles has left to live, before it dies
    float maxage;
};

/**
 * @short This class takes care of entire particle system (collection of similar particles)
 * When should you use one particle system and when more? One particle system
 * should only hold similar particles. This means that if you e.g. have a unit
 * burning and smoking, you should use two particle systems, because smoke and
 * flames are very different things. It doesn't matter that both are used for
 * the same unit.
 * 
 * BosonParticleSystem should be very flexible. You can specify your own init 
 * and update functions for particles. They are called every time when particle
 * is updated or initialized (resetted). If you want even more flexibility, then
 * you can write you own subclass; most important methods are virtual, so you
 * can change them if you really want to.
 **/
class BosonParticleSystem
{
  public:
    /**
     * Constructs new BosonParticleSystem. Use this constructor if you don't
     * want to use update- and init-functions feature.
     * @param maxnum Maximum number of particles this system may have
     * @param initialnum This number of initial particles are created
     * @param size Size of particle
     * @param createrate How many additional particles are created each second
     * @param align Whether to align particles to camera
     * @param maxradius Radius of bounding sphere of this system (not used yet)
     * @param texture Id of particle texture
     * @param color Color of particle
     * @param particleage How long (in seconds) each particle lives
     * @param age Age of this system (for how long it creates new particles)
     * @param pos Position of this system
     * @param velo Velocity of each particle
     * @param initFunc This function, if specified, is called every time particle is inited
     * @param updateFunc This function, if specified, is called every time particle is updated
     **/
    BosonParticleSystem(int maxnum, int initialnum, float size,
        float createrate, bool align, float maxradius, int texture,
        BoVector4 color, float particleage, float age, BoVector3 pos, BoVector3 velo,
        BosonParticleSystemProperties* prop = 0);
    /**
     * Constructs new BosonParticleSystem. This constructor is often enough if
     * you use functions for initing and updating particles.
     * If you use this constructor, use accessor methods to set different
     * variables and then @ref createParticles to create initial particles
     * @param maxnum Maximum number of particles this system may have
     * @param createrate How many additional particles are created each second
     * @param align Whether to align particles to camera
     * @param maxradius Radius of bounding sphere of this system (not used yet)
     * @param texture Id of particle texture
     * @param initFunc This function, if specified, is called every time particle is inited
     * @param updateFunc This function, if specified, is called every time particle is updated
     **/
    BosonParticleSystem(int maxnum, float createrate,
        bool align, float maxradius, int texture, BosonParticleSystemProperties* prop);
    /**
     * Destructs BosonParticleSystem. This deleted all particles
     **/
    virtual ~BosonParticleSystem();

    /**
     * Updates all particles within this system and creates new ones if needed
     * and if age is more than 0.
     **/
    virtual void update(float elapsed);
    /**
     * Draws all particles.
     * Note that blending must be enabled before you call this and if you use
     * custom blending functions (see @ref setBlendFunc), you may want to reset
     * blending functions after calling this.
     **/
    virtual void draw();
    
    /**
     * Moves all active particles by v
     **/
    void moveParticles(BoVector3 v);

    /**
     * Sets current position of this system. Position of particles are relative
     * to position of system they belong to, so you can easily move all
     * particles with this method
     * @param p New position of this system
     **/
    inline void setPosition(BoVector3 p) { mPos = p; };
    /**
     * @return Current position of this system
     **/
    inline BoVector3 position() { return mPos; };

    /**
     * Sets current velocity of particles in this system.
     * Note that if you use custom update or init functions you can specify
     * velocity for each particle directly and this may not have any effect
     * then.
     * @param v New velocity of particles in this system
     **/
    inline void setVelocity(BoVector3 v) { mVelo = v; };
    /**
     * @return Current velocity of particles in this system
     **/
    inline BoVector3 velocity() { return mVelo; };

    /**
     * Sets color of particles in this system.
     * Note that new color is only applied to particles which will be inited
     * after calling this method, it doesn't change color of active particles.
     * @param v New color of particles in this system
     **/
    inline void setColor(BoVector4 c) { mColor = c; };
    /**
     * @return Color of particles in this system
     **/
    inline BoVector4 color() { return mColor; };

    /**
     * @return Average size of particles in this system
     **/
    inline float size() { return mSize; };
    /** 
     * Sets average size of particles in this system. This is used for some
     * drawing calculations.
     * @param s New average size of particles
     **/
    inline void setSize(float s) { mSize = s; };

    /**
     * @return Current age of this system
     **/
    inline float age() { return mAge; };
    /**
     * Sets age of this system (in seconds).
     * Age is decreased every time @ref update is called and if it's 0, no new
     * particles are created anymore
     * @param a New age of this system
     **/
    inline void setAge(float a) { mAge = a; };

    /**
     * @return Current creation rate of particles (per second)
     **/
    inline float createRate() { return mCreateRate; };
    /**
     * Sets current creation rate of particles (per second).
     * New particles are periodically created every @ref update call.
     * @param r New creation rate of particles
     **/
    inline void setCreateRate(float r) { mCreateRate = r; };

    /**
     * @return Radius of bounding sphere of this system. This can be used for culling
     **/
    inline float boundingSphereRadius()  { return mRadius; };
    
    /**
     * @return Whether this system is active
     * Active means that this system has at least 1 living particle
     **/
    inline bool isActive()  { return (mNum > 0); };

    /**
     * Sets OpenGL blending function of this system. This function is used in
     * @ref draw mehod. You can use this to specify your own blending functions
     * if needed. Defaults are GL_SRC_ALPHA and GL_ONE_MINUS_SRC_ALPHA.
     * @param sf Source blend function
     * @param df Destination blend function
     **/
    inline void setBlendFunc(int sf, int df) { mBlendFunc[0] = sf; mBlendFunc[1] = df; };

    /**
     * @return Number of living (active) particles in this system
     **/
    inline int particleCount() { return mNum; };

    /**
     * Creates specified number of new particles.
     * @param count How many new particles to create
     **/
    void createParticles(int count);

  protected:
    virtual void init(int initialnum);

    virtual void initParticle(BosonParticle* particle);
    virtual void updateParticle(BosonParticle* particle);

  protected:
    BosonParticle* mParticles;  // Array of particles
    int mMaxNum;  // Maximum number of particles
    int mNum;  // Current number of particle (aka number of active particles)
    float mSize;  // Size of particles
    float mCreateRate;  // Number of particles created per second
    float mCreateCache;  // Number of particles to create during next update
    bool mAlign;  // Whether to align particles to camera
    float mRadius;  // Radius of bounding sphere
    int mTexture;  // Texture of particles
    BoVector3 mPos;
    BoVector3 mVelo;
    float mAge;
    int mBlendFunc[2];

    BoVector4 mColor;  // Color of particle
    float mParticleAge;  // How many seconds particles live

    BosonParticleSystemProperties* mProp;
};

#endif // BOSONPARTICLESYSTEM_H
