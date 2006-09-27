/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : include/ufo/uobject.hpp
    begin             : Tue May 8 2001
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef UOBJECT_HPP
#define UOBJECT_HPP

#include "ufo_global.hpp"

#include "ucollectable.hpp"

#include "signals/usignal.hpp"

#include <string>
#include <iostream>

#include <list>

// UFO RTTI is mainly used for debugging purposes

#ifdef UFO_RTTI

namespace ufo {
class UObject;
typedef UObject *(*UObjectConstructorFn)(void);
} // namespace ufo

//
// UFO RTTI declaration
//

#define UFO_DECLARE_CLASS(name)           \
 public:                                          \
  static ufo::UClassInfo sm_class##name;          \
  virtual ufo::UClassInfo * getClassInfo() const  \
   { return &name::sm_class##name; }

//
// UFO RTTI implementation
//

// dynamic classes

// implements rtti for classes with default constructor
/*
#define UFO_IMPLEMENT_CLASS(name, basename)   \
 ufo::UObject * UConstructorFor##name()                       \
    { return new name; }                                      \
 ufo::UClassInfo name::sm_class##name(                        \
          #name,                                              \
          #basename,                                          \
          (unsigned int) sizeof(name),                        \
          (ufo::UObjectConstructorFn) UConstructorFor##name);
*/

#define UFO_IMPLEMENT_CLASS(name, basename)       \
 ufo::UClassInfo name::sm_class##name(                     \
           #name,                                          \
           #basename,                                      \
           (unsigned int) sizeof(name),                    \
           (ufo::UObjectConstructorFn) NULL);

//
// class info structure
//

// FIXME hash map would be better
#include <map>
namespace ufo {

class UFO_EXPORT UClassInfo {
public:
	UClassInfo(
			const std::string & className,
			const std::string & baseName,
			unsigned int size,
			UObjectConstructorFn ctor)
		: m_className(className)
		, m_baseClassName(baseName)
		, m_objectSize(size)
		, m_objectConstructor(ctor)
		, m_baseInfo(NULL)
		, m_next(sm_first)
	{
		sm_first = this;
	}

	~UClassInfo();

	UObject * createObject() const {
		return m_objectConstructor ? (*m_objectConstructor)() : NULL;
	}

	const std::string & getClassName() const { return m_className; }
	const std::string & getBaseClassName() const { return m_baseClassName; }
	const UClassInfo * getBaseClassInfo() const { return m_baseInfo; }
	unsigned int getSize() const { return m_objectSize; }

	UObjectConstructorFn getConstructor() const { return m_objectConstructor; }
	static const UClassInfo * getFirst() { return sm_first; }
	const UClassInfo * getNext() const { return m_next; }

	static UClassInfo * findClass(const std::string & className);

	/** Inits the class info structs and registers them at the internal cache
	  */
	static void initClassInfo();

private:
	std::string m_className;
	std::string m_baseClassName;
	unsigned int m_objectSize;
	UObjectConstructorFn m_objectConstructor;

	// Pointers to base UClassInfos: set in InitializeClasses
	const UClassInfo * m_baseInfo;

	// a simple single linked list for all class infos
	static UClassInfo * sm_first;
	UClassInfo * m_next;

	static std::map<std::string, UClassInfo*> sm_classTable;

	UFO_DECLARE_NO_COPY_CLASS(UClassInfo)
};

} // namespace ufo

#else // UFO_RTTI

#define UFO_DECLARE_CLASS(name)
#define UFO_IMPLEMENT_CLASS(name, basename)

#endif // !UFO_RTTI


#define UFO_DECLARE_DYNAMIC_CLASS(name) UFO_DECLARE_CLASS(name)
#define UFO_DECLARE_ABSTRACT_CLASS(name) UFO_DECLARE_CLASS(name)

#define UFO_IMPLEMENT_DYNAMIC_CLASS(name, basename) UFO_IMPLEMENT_CLASS(name, basename)
#define UFO_IMPLEMENT_DEFAULT_DYNAMIC_CLASS(name, basename) UFO_IMPLEMENT_CLASS(name, basename)
#define UFO_IMPLEMENT_ABSTRACT_CLASS(name, basename) UFO_IMPLEMENT_CLASS(name, basename)
#define UFO_IMPLEMENT_CTOR_DYNAMIC_CLASS(name, basename) UFO_IMPLEMENT_CLASS(name, basename)

namespace ufo {

struct UObjectSlotNode;

/** @short This is the base class for all dynamic UFO objects.
  * @ingroup core
  *
  * @author Johannes Schmidt
  */

class UFO_EXPORT UObject : public virtual UCollectable {
	UFO_DECLARE_CLASS(UObject)
	friend struct UObjectSlotNode;
public:
	UObject();
	UObject(const UObject &);
	virtual ~UObject();

	/** returns a std::size_t which is used
	  * when UObject pointers are the keys for UHashMap.
	  * If equals(UObject*) returns true, these two objects should return
	  * the same hash code. If these objects are different, the hash code may
	  * be equal.
	  * @return a hash code.
	  */
	virtual unsigned int hashCode() const;

	/** checks if this object is the same object than obj
	  */
	virtual bool equals(const UObject * obj) const;

	// FIXME
	// are there virtual operators? guess not.
	bool operator==(const UObject & obj) const;

	/** tries to make a new object with the same internal data. */
	virtual UObject * clone() const;

	//
	// output
	//

	/** std::string representation */
	virtual std::string toString() const;

	/** operator to print object data to output streams */
	friend std::ostream & operator<<(std::ostream & os, const UObject & o);
	/** operator to print object data to output streams */
	friend std::ostream & operator<<(std::ostream & os, const UObject * o);

	//
	// debugging
	//

	const std::string & getName() const;
	/** For debuggin purposes */
	void setName(const std::string & newNameA);

	/** This method is only for debugging purposes and
	  * may be removed without warning.
	  * @return The number of allocated UObject objects.
	  */
	unsigned int objCount();

	///
	/// memory management
	///

	/** Increases the reference count of c by 1 and adds c to the internal
	  * memory tracking list. Does nothing if c is NULL.
	  * You may call trackPointer several times with the same pointer as
	  * argument.
	  * @see releasePointer
	  * @param c The pointer this widget should track. May be NULL.
	  * @return The added object or NULL on failure.
	  */
	UCollectable * trackPointer(UCollectable * c);
	/** Increases the reference count of c by 1 and adds c to the internal
	  * memory tracking list. Does nothing if c is NULL.
	  * You may call trackPointer several times with the same pointer as
	  * argument.
	  * @see releasePointer
	  * @param c The pointer this widget should track. May be NULL.
	  * @return The added object or NULL on failure.
	  */
	const UCollectable * trackPointer(const UCollectable * c);

	/** Removes the first occurence of c from the memory tracking list and
	  * decreases its reference count by 1.
	  * @see trackPointer
	  * @param c The pointer this widget should remove from its memory
	  *  tracking list. May be NULL.
	  * @return true on success.
	  */
	bool releasePointer(UCollectable * c);
	/** Removes the first occurence of c from the memory tracking list and
	  * decreases its reference count by 1.
	  * @see trackPointer
	  * @param c The pointer this widget should remove from its memory
	  *  tracking list. May be NULL.
	  * @return true on success.
	  */
	bool releasePointer(const UCollectable * c);

	/** If oldObj does not point to newObj, calls releasePointer with oldObj
	  * abd trackPointer with newObj.
	  * Both may be NULL.
	  */
	void swapPointers(const UCollectable * oldObj, const UCollectable * newObj);

	/** Removes all pointers from the memory tracking list and decreases
	  * their reference count by one.
	  */
	void releaseAllPointers();

protected:  // Protected methods
	/** Prints some useful internal data to the ostream os.
	  * This method is meant only for debugging purposes.
	  */
	virtual std::ostream & paramString(std::ostream & os) const;
private:
	/** Only for debugging purposes.
	  * FIXME: Should we use implicit sharing? Most of the time it is "".
	  *  ""-Sharing should be done by the STL (?).
	  */
	std::string m_name;

	std::list<const UCollectable*> m_pointers;
	std::list<UObjectSlotNode*> m_objectSlots;

public: // Public Signals
	/** This signal is fired immediately before this object is destroyed
	  * Please note that all information about subclass features is already
	  * destroyed.
	  */
	USignal1<UObject*> & sigDestroyed();

private: // Private Signals
	USignal1<UObject*> m_sigDestroyed;
};

//
// inline implemenation
//

inline bool
UObject::operator==(const UObject & obj) const {
	return equals(&obj);
}

inline std::ostream & operator<<(std::ostream & os, const UObject & o) {
	return os << o.toString();
}

inline std::ostream & operator<<(std::ostream & os, const UObject * o) {
	if (o) {
		return os << o->toString();
	} else {
		return os << "NULL";
	}
}

inline const std::string &
UObject::getName() const {
	return m_name;
}

inline void
UObject::setName(const std::string & newNameA) {
	m_name = newNameA;
}

// signals
inline USignal1<UObject*> &
UObject::sigDestroyed() {
	return m_sigDestroyed;
}

} // namespace ufo

#endif // UOBJECT_HPP
