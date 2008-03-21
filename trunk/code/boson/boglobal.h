/*
    This file is part of the Boson game
    Copyright (C) 2003 Andreas Beckermann (b_mann@gmx.de)

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
#ifndef BOGLOBAL_H
#define BOGLOBAL_H

class BoGlobalObjectBase;
class BoInfo;
class BosonConfig;
class BosonProfiling;
class BosonData;
class BosonAudioInterface;
class BoItemListHandler;

class BoGlobalPrivate;

/**
 * This class groups most of the global static object used in boson. There is
 * only a single object of this class per boson process. It will take care of
 * creating and deleting global objects e.g. for @ref BosonConfig and @ref
 * BosonProfiling and so on.
 *
 * Create the object of this class using @ref initStatic. It will be deleted
 * automatically on program destruction.
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGlobal
{
public:
	// AB: there is no public constructor. leave it like this! we are
	// enforeced to use initStatic()!
	~BoGlobal();

	/**
	 * Initialize the BoGlobal object. This must be called before @ref
	 * boGlobal.
	 *
	 * See also @ref BoGlobalObject
	 **/
	static void initStatic();

	/**
	 * Construct all global objects that have been registered using @ref
	 * registerObject.
	 **/
	void initGlobalObjects();

	void destroyGlobalObjects();

	static BoGlobal* boGlobal() { return mBoGlobal; }

	void setBoInfo(BoInfo* b) { mBoInfo = b; }
	BoInfo* boInfo() const { return mBoInfo; }

	void setBosonConfig(BosonConfig* b) { mBosonConfig = b; }
	BosonConfig* bosonConfig() const { return mBosonConfig; }

	void setBosonProfiling(BosonProfiling* b) { mBosonProfiling = b; }
	BosonProfiling* bosonProfiling() const { return mBosonProfiling; }

	void setBosonData(BosonData* b) { mBosonData = b; }
	BosonData* bosonData() const { return mBosonData; }

	void setBosonAudio(BosonAudioInterface* b) { mBosonAudio = b; }
	BosonAudioInterface* bosonAudio() const { return mBosonAudio; }

	void setBoItemListHandler(BoItemListHandler* b) { mBoItemListHandler = b; }
	BoItemListHandler* boItemListHandler() const { return mBoItemListHandler; }

	/**
	 * Use @p pointer for one of the global objects (such as @ref
	 * bosonProfiling, @ref bosonConfig, ...)
	 * @param id A value from @ref BoGlobalObjectBase::ObjectType.
	 **/
	void setPointer(void* pointer, int id);

	/**
	 * @return A pointer to the object inserted using @ref setPointer with
	 * @p id. If there are multiple pointers inserted with the same id then
	 * it is undefined which object gets returned.
	 **/
	void* pointer(int id) const;

	/**
	 * Register an object for initializing by @ref initGlobalObjects. The
	 * object is appended by default, if @p initFirst is TRUE then it is
	 * prepended.
	 **/
	void registerObject(BoGlobalObjectBase* o, bool initFirst = false);

	/**
	 * Remove @p o from the object list. This is called by the @ref
	 * BoGlobalObjectBase destructor.
	 **/
	void unregisterObject(BoGlobalObjectBase* o);

protected:
	BoGlobal();

private:
	static BoGlobal* mBoGlobal;

private:
	BoGlobalPrivate* d;
	BoInfo* mBoInfo;
	BosonConfig* mBosonConfig;
	BosonProfiling* mBosonProfiling;
	BosonData* mBosonData;
	BosonAudioInterface* mBosonAudio;
	BoItemListHandler* mBoItemListHandler;
};

/**
 * You should use @ref BoGlobalObject instead of the base class directly.
 *
 * You might want to use BoGlobalObject directly if you have an object that
 * needs special initialization, i.e. some special constructor arguments. But in
 * such a case you should really re-think whether you <em>really</em> need to
 * make that object global!
 *
 * @short Base class for @ref BoGlobalObject
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
class BoGlobalObjectBase
{
public:
	enum ObjectType {
		// a BosonConfig object
		BoGlobalConfig = 1,

		// a BoInfo object
		BoGlobalInfo = 2,

		// a BosonProfiling object
		BoGlobalProfiling = 3,

		// a BosonData object
		BoGlobalData = 4,

		// a BoItemListHandler object
		BoGlobalItemListHandler = 6,

		// a BosonAudioInterface object
		BoGlobalAudio = 7

	};

public:
	/**
	 * Construct a BoGlobalObjectBase object. This takes care about
	 * initializing the @ref BoGlobal object, in case that this hasn't
	 * happened yet.
	 *
	 * Do not create the object for your global class here! Use @ref
	 * loadObject instead!
	 *
	 * @param initFirst TRUE to initialize this object before all others.
	 * Note that this object must not depend on any other object in any way!
	 * An example for an "initFirst" object is @ref BosonConfig, because
	 * some other global objects may depend on it.
	 **/
	BoGlobalObjectBase(bool initFirst = false);

	/**
	 * Destruct the BoGlobalObjectBase object. You must <em>not</em> destroy
	 * the object this class contains in the destructor!
	 *
	 * Your object's destructor may depend on some variables being available
	 * (such ass qapp or kapp) but that isn't the case here, as an object of
	 * this class is created on the stack (i.e. without new) and deleted (in
	 * <em>undefined</em> order) when the program quits!
	 *
	 * See @ref deleteObject
	 **/
	virtual ~BoGlobalObjectBase();

	/**
	 * You must reimplement this to create the global object that you want
	 * to use (i.e. a @ref BosonConfig object, or a @ref BosonData object,
	 * or ...)
	 *
	 * Do <em>not</em> do that in the constructor! Also you must not call
	 * this method in your constructor!
	 **/
	virtual bool loadObject() = 0;

	/**
	 * Your implementation should delete the object that was created in @ref
	 * loadObject here. Do <em>not</em> do that in the destructor! Also you
	 * must not call this method in your destructor!
	 **/
	virtual bool deleteObject() = 0;

	/**
	 * @return The pointer of the object this class contains (not to this
	 * class, i.e. not the this pointer itself!)
	 **/
	virtual void* pointer() const = 0;
};

/**
 * This is the recommended (ahem: by me ;-)) way to make an object of a class
 * global. For your own convenience you should also add a set/get method to @ref
 * BoGlobal so that you can access the global object once it was created. Also
 * add an enum entry to @ref BoGlobalObjectBase::ObjectType and add it to the
 * switch in @ref BoGlobal::setPointer.
 * This is only some cut'n'paste work (calling the set function you just added).
 * If you don't want to do this work you can leave it out and just use @ref
 * BoGlobal::pointer(), but I guess you don't want that :-)
 *
 * The intention of this class is to make it easy to add a global object of a
 * class (such as @ref BosonConfig, @ref BosonData, ...) without ever creating
 * the object in a place like main.cpp/top.cpp where they get created very
 * early. You would have to create this global object also in borendermain.cpp,
 * boinfomain.cpp and so on (that sucks).
 *
 * An additional (very important) requirement is that this file
 * (boglobal.[h|cpp]) mustn't depend on the class's implementation, as we would
 * have to link to it then (it isn't acceptable for boinfo to link to
 * bosonplayfield.o).
 *
 * This class meets these requirements. All you have to do (beside code changes
 * to @ref BoGlobal mentioned above) is the following code:
 * <pre>
 * static BoGlobalObject<BosonConfig> object(BoGlobalObject::BoGlobalConfig);
 * </pre>
 * That's all. Everything else will happen automatically. Once @ref
 * BoGlobal::initGlobalObjects was called (you need to do this once in your
 * program) all the objects that have been created this way will get loaded (see
 * @ref loadObject). Note that this is a global variable, created on the stack.
 * That means that they will be created on application startup (even before the
 * @ref KApplication object), as long as your application is linked to the file
 * the line above is in.
 *
 * There is one special requirement to the classes that can be used here: they
 * must provide a default constructor (i.e. a constructor that takes no
 * arguments). If you need special arguments you need to reimplement @ref
 * loadObject, but you should first re-think your design, as you are most
 * probably doing something very bad!
 * @author Andreas Beckermann <b_mann@gmx.de>
 **/
template<class T>
class BoGlobalObject : public BoGlobalObjectBase
{
public:
	/**
	 * @param id Either a value from @ref BoGlobalObjectBase::ObjectType or
	 * 0. If it is non-0, then @ref BoGlobal::setPointer will be called for
	 * this object in @ref loadObject. Note that this class must actually be
	 * of the type that you provide here or we will crash!
	 *
	 * @param See @ref BoGlobalObjectBase::BoGlobalObjectBase
	 **/
	BoGlobalObject(int id = 0, bool initFirst = false) : BoGlobalObjectBase(initFirst)
	{
		mObject = 0;
		mId = id;
	}
	virtual ~BoGlobalObject()
	{
	}

	/**
	 * Create the global object. It will be deleted by @ref deleteObject.
	 *
	 * Do not call this from within your constructor!
	 **/
	virtual bool loadObject()
	{
		mObject = new T();
		if (mId != 0) {
			BoGlobal::boGlobal()->setPointer(mObject, mId);
		}
		return true;
	}
	virtual bool deleteObject()
	{
		if (mId != 0 && BoGlobal::boGlobal()) {
			BoGlobal::boGlobal()->setPointer(0, mId);
		}
		delete mObject;
		mObject = 0;
		return true;
	}

	virtual void* pointer() const
	{
		return (void*)mObject;
	}

private:
	T* mObject;
	int mId;
};


#endif

